#include "PchApp.h"

#include "FeatureSpaceGL.h"
#include "opengl/Renderer.h"
#include "opengl/OpenGLContext.h"

#include "utils/Console.h"

#include <mmsystem.h>
#include <string>

#define FS_USE_MIPMAPS 1
#define FS_USE_EXTRA_VIEWPORTS 0

const float FeatureSpaceGL::degs_to_rad = 180.0 / M_PI;
const float FeatureSpaceGL::rads_to_deg = 1.0 / (180 / M_PI);

FeatureSpaceGL::FeatureSpaceGL(int LOD_arg, int band1_arg, int band2_arg, int band3_arg)
{
    cam_dolly = 2.5f;
    cam_xpan = 0.0f;
    cam_ypan = 0.0f;
    pitch = 0.0f;
    z_rot = 0.0f;

	band1 = band1_arg;
	band2 = band2_arg;
	band3 = band3_arg;

	smooth = false;
	num_points = 0;
	vertices = 0;
	LOD = LOD_arg;
	granularity = 1;
	while (LOD_arg) { granularity = granularity * 4; LOD_arg--; }

    // GLText 将在渲染路径中按需创建（或留空）
    gl_text = nullptr;
    // OpenGL 资源延迟初始化
    program = 0; loc_aPos = 0; loc_aColor = 1; loc_uMVP = -1;
    vaoBox = 0; vboBox = 0; boxVertexCount = 0;
    pointVaos.clear(); pointVbos.clear(); pointCounts.clear();
}

void FeatureSpaceGL::draw()
{
    // 由 Renderer 驱动绘制；此处不再直接交换缓冲
}

void FeatureSpaceGL::build_box_buffers(void)
{
    struct V { float x,y,z; float r,g,b,a; };
    std::vector<V> verts;
    verts.reserve(48);
	const float edge_opacity = 0.5f;
	const float origin_opacity = 0.8f;
	auto push = [&](glm::vec3 p, glm::vec4 c){ verts.push_back({p.x,p.y,p.z,c.r,c.g,c.b,c.a}); };
	push({0,0,0}, {1,0,0,origin_opacity}); push({1,0,0}, {1,1,1,edge_opacity});
	push({0,0,0}, {0,1,0,origin_opacity}); push({0,1,0}, {1,1,1,edge_opacity});
	push({0,0,0}, {0,0,1,origin_opacity}); push({0,0,1}, {1,1,1,edge_opacity});
	auto w = glm::vec4(1,1,1,edge_opacity);
	push({1,0,0}, w); push({1,1,0}, w);
	push({1,0,0}, w); push({1,0,1}, w);
	push({0,1,0}, w); push({1,1,0}, w);
	push({0,1,0}, w); push({0,1,1}, w);
	push({0,0,1}, w); push({1,0,1}, w);
	push({0,0,1}, w); push({0,1,1}, w);
	push({0,1,1}, w); push({1,1,1}, w);
	push({1,0,1}, w); push({1,1,1}, w);
	push({1,1,0}, w); push({1,1,1}, w);
	boxVertexCount = (GLsizei)verts.size();
	if (vaoBox == 0) glGenVertexArrays(1, &vaoBox);
	if (vboBox == 0) glGenBuffers(1, &vboBox);
	glBindVertexArray(vaoBox);
	glBindBuffer(GL_ARRAY_BUFFER, vboBox);
	glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(V), verts.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray((GLuint)loc_aPos);
	glVertexAttribPointer((GLuint)loc_aPos, 3, GL_FLOAT, GL_FALSE, sizeof(V), (const void*)0);
	glEnableVertexAttribArray((GLuint)loc_aColor);
	glVertexAttribPointer((GLuint)loc_aColor, 4, GL_FLOAT, GL_FALSE, sizeof(V), (const void*)(sizeof(float)*3));
	glBindVertexArray(0);
}

void FeatureSpaceGL::add_points(points_hash_t points_hash, unsigned char red, unsigned char green, unsigned char blue)
{
	Console::write("FeatureSpaceGL::add_points(size = %d, r = %d, g = %d, b = %d\n",
			points_hash.size(), red, green, blue);
	assert(points_hash.size() > 0);
	unsigned int total_count = 0;
	for (auto &kv : points_hash) total_count += kv.second;
	vertices += (unsigned int)points_hash.size();
	unsigned int average_count = total_count / (unsigned int)points_hash.size();
	Console::write("total = %d, average_count = %d\n", total_count, average_count);

    struct PV { float x,y,z; float r,g,b,a; };
    const unsigned int points_per_vbo = 5000;
    std::vector<PV> chunk; chunk.reserve(points_per_vbo);
	float rf = (float)red/255.0f, gf = (float)green/255.0f, bf = (float)blue/255.0f;
	for (auto it = points_hash.begin(); it != points_hash.end(); ++it) {
		unsigned int point = it->first; unsigned int count = it->second; if (count == 0) continue;
		unsigned char* pointx = (unsigned char*)&point; unsigned char* pointy = (unsigned char*)&point + 1; unsigned char* pointz = (unsigned char*)&point + 2;
		float a = (((float)count/(float)average_count) * 0.45f) + 0.1f;
		chunk.push_back({ (*pointx)/255.0f, (*pointy)/255.0f, (*pointz)/255.0f, rf, gf, bf, a });
		num_points += count;
		if (chunk.size() >= points_per_vbo) {
			GLuint vao=0, vbo=0; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
			glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, chunk.size()*sizeof(PV), chunk.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray((GLuint)loc_aPos);
			glVertexAttribPointer((GLuint)loc_aPos, 3, GL_FLOAT, GL_FALSE, sizeof(PV), (const void*)0);
			glEnableVertexAttribArray((GLuint)loc_aColor);
			glVertexAttribPointer((GLuint)loc_aColor, 4, GL_FLOAT, GL_FALSE, sizeof(PV), (const void*)(sizeof(float)*3));
			glBindVertexArray(0);
			pointVaos.push_back(vao); pointVbos.push_back(vbo); pointCounts.push_back((GLsizei)chunk.size()); chunk.clear();
		}
	}
	if (!chunk.empty()) {
		GLuint vao=0, vbo=0; glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
		glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, chunk.size()*sizeof(PV), chunk.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray((GLuint)loc_aPos);
		glVertexAttribPointer((GLuint)loc_aPos, 3, GL_FLOAT, GL_FALSE, sizeof(PV), (const void*)0);
		glEnableVertexAttribArray((GLuint)loc_aColor);
		glVertexAttribPointer((GLuint)loc_aColor, 4, GL_FLOAT, GL_FALSE, sizeof(PV), (const void*)(sizeof(float)*3));
		glBindVertexArray(0);
		pointVaos.push_back(vao); pointVbos.push_back(vbo); pointCounts.push_back((GLsizei)chunk.size());
	}
}

void FeatureSpaceGL::translate_cam(float x, float y)
{
	cam_xpan+=x;
	cam_ypan+=y;
	draw();
}

void FeatureSpaceGL::zoom_cam(float diff)
{
	cam_dolly += diff;
	draw();
}	

void FeatureSpaceGL::rot_cam(float x_diff, float y_diff)
{
	pitch += y_diff;
	z_rot += x_diff;
	draw();
}		

void FeatureSpaceGL::resize(void)
{
    // 由外部 GLView 管理尺寸；如需文本，调用方设置 gl_text 视口尺寸
}

void FeatureSpaceGL::toggle_smooth(void)
{
    Console::write("Toggling FS Smoothing\n");
    smooth = !smooth;
    draw();
}

void FeatureSpaceGL::ensureGLResources()
{
    if (program != 0) return;
    // 构建 3.3 core 着色器程序
    const char* vs =
        "#version 330 core\n"
        "layout(location=0) in vec3 aPos;\n"
        "layout(location=1) in vec4 aColor;\n"
        "uniform mat4 uMVP;\n"
        "out vec4 vColor;\n"
        "void main(){ vColor=aColor; gl_Position=uMVP*vec4(aPos,1.0); }";
    const char* fs =
        "#version 330 core\n"
        "in vec4 vColor;\n"
        "out vec4 FragColor;\n"
        "void main(){ FragColor = vColor; }";
    GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsh,1,&vs,NULL);
    glCompileShader(vsh);

    GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsh,1,&fs,NULL);
    glCompileShader(fsh);

    program = glCreateProgram();
    glAttachShader(program,vsh);
    glAttachShader(program,fsh);
    glLinkProgram(program);
    glDeleteShader(vsh);
    glDeleteShader(fsh);

    loc_aPos = 0;
    loc_aColor = 1;
    loc_uMVP = glGetUniformLocation(program, "uMVP");

    if (vaoBox == 0 || vboBox == 0 || boxVertexCount == 0) {
        build_box_buffers();
    }
}


