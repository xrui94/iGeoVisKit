#include "PchApp.h"

#include "GLText.h"
#include "GLUtils.h"

#include <stdarg.h>
#include <vector>
#include <filesystem>
#include <string>
#include <fstream>

#include <stb_truetype.h>

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);
    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(sh, len, &len, &log[0]);
        OutputDebugStringA(("GLText shader compile error: " + log + "\n").c_str());
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static GLuint link_program(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(prog, len, &len, &log[0]);
        OutputDebugStringA(("GLText program link error: " + log + "\n").c_str());
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

GLText::GLText(const char* font_arg, int size_arg)
{
    font = font_arg;
    size = size_arg;
    red = 1.0;
    green = 1.0;
    blue = 1.0;
    program = 0;
    vbo = 0;
    loc_aPos = -1;
    loc_aUV = -1;
    loc_uProj = -1;
    loc_uColor = -1;
    loc_uTex = -1;
    last_x = 0;
    last_y = 0;
    viewport_width = 0;
    viewport_height = 0;

    // 构造时要求外部确保当前有有效 OpenGL 上下文

	std::string font_path = font ? std::string(font) : std::string();
	bool has_sep = false;
	for (char c : font_path) { if (c == '/' || c == '\\' || c == ':') { has_sep = true; break; } }
	if (!has_sep) {
		std::string lower = font_path; for (char &c : lower) c = (char)tolower(c);
		std::vector<std::string> candidates;
		candidates.push_back("C:/Windows/Fonts/" + lower + ".ttf");
		if (lower == "ariel" || lower == "arial") {
			candidates.push_back("C:/Windows/Fonts/arial.ttf");
			candidates.push_back("C:/Windows/Fonts/ARIAL.TTF");
		}
		for (const auto &p : candidates) {
			if (std::filesystem::exists(p)) { font_path = p; break; }
		}
	}

	std::vector<unsigned char> ttf;
	if (!font_path.empty() && std::filesystem::exists(font_path)) {
		std::ifstream f(font_path, std::ios::binary);
		f.seekg(0, std::ios::end);
		size_t len = (size_t)f.tellg();
		f.seekg(0, std::ios::beg);
		ttf.resize(len);
		f.read((char*)ttf.data(), len);
	}

	stbtt_fontinfo info;
	float scale = 1.0f;
	if (!ttf.empty() && stbtt_InitFont(&info, ttf.data(), 0)) {
		int ascent, descent, lineGap;
		scale = stbtt_ScaleForPixelHeight(&info, (float)size);
		stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
		baseline = (float)ascent * scale;
	} else {
		baseline = (float)size; // 退化处理
	}

	atlas_width = 512;
	atlas_height = 512;
	std::vector<unsigned char> atlas((size_t)atlas_width * (size_t)atlas_height);
	stbtt_pack_context spc;
	if (!ttf.empty() && stbtt_PackBegin(&spc, atlas.data(), atlas_width, atlas_height, 0, 1, NULL)) {
		stbtt_PackSetOversampling(&spc, 2, 2);
		stbtt_PackFontRange(&spc, ttf.data(), 0, (float)size, 32, 96, cdata);
		stbtt_PackEnd(&spc);
		glGenTextures(1, &font_texture);
		glBindTexture(GL_TEXTURE_2D, font_texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // 使用 Core Profile 兼容的单通道纹理格式
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlas_width, atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		assert(glGetError() == GL_NO_ERROR);

		// 字符信息已打包到成员 cdata
	} else {
		font_texture = 0;
	}

    // 构建着色器程序（GLSL 3.30，Core Profile）
    const char* vs_src =
        "#version 330 core\n"
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aUV;\n"
        "uniform mat4 uProj;\n"
        "out vec2 vUV;\n"
        "void main(){\n"
        "    vUV = aUV;\n"
        "    gl_Position = uProj * vec4(aPos, 0.0, 1.0);\n"
        "}";

    const char* fs_src =
        "#version 330 core\n"
        "in vec2 vUV;\n"
        "uniform sampler2D uTex;\n"
        "uniform vec3 uColor;\n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "    float a = texture(uTex, vUV).r;\n"
        "    FragColor = vec4(uColor, a);\n"
        "}";

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    program = (vs && fs) ? link_program(vs, fs) : 0;
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);
    assert(program != 0);
    // 查询属性位置；若驱动返回 -1（例如采用 layout 固定位置或优化裁剪），回退到显式 layout 的 0/1 索引
    loc_aPos = glGetAttribLocation(program, "aPos");
    if (loc_aPos < 0) loc_aPos = 0;
    loc_aUV  = glGetAttribLocation(program, "aUV");
    if (loc_aUV < 0) loc_aUV = 1;
    loc_uProj = glGetUniformLocation(program, "uProj");
    loc_uColor = glGetUniformLocation(program, "uColor");
    loc_uTex = glGetUniformLocation(program, "uTex");
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
}

GLText::~GLText(void)
{
    // 要求调用方保证当前上下文有效
    if (glIsTexture(font_texture) == GL_TRUE) {
        glDeleteTextures(1, &font_texture);
    }
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
}

void GLText::draw_string(const char* format, ...)
{
    char* text = new char[512];
    va_list	ap;
    
    assert(format != NULL);
    
    va_start(ap, format);
    vsprintf(text, format, ap);
    va_end(ap);

	// 使用最后一次明确坐标或默认 (0,0)
	draw_text_at(last_x, last_y, text);
	delete[] text;

}

void GLText::draw_string(int x, int y, const char* format, ...)
{
    char* text = new char[512];
    va_list	ap;
    
    assert(format != NULL);
    
    va_start(ap, format);
    vsprintf(text, format, ap);
    va_end(ap);

	draw_text_at(x, y, text);
	last_x = x;
	last_y = y;
	delete[] text;
}

void GLText::set_color(GLfloat red_arg, GLfloat green_arg, GLfloat blue_arg)
{
	red = red_arg;
	green = green_arg;
	blue = blue_arg;
	
	if (red < 0.0) red = 0.0;
	if (green < 0.0) green = 0.0;
	if (blue < 0.0) blue = 0.0;
	if (red > 1.0) red = 1.0;
	if (green > 1.0) green = 1.0;
	if (blue > 1.0) blue = 1.0;
}

void GLText::get_color(GLfloat* red_return, GLfloat* green_return, GLfloat* blue_return)
{
    if (red_return != NULL) *red_return = red;
    if (green_return != NULL) *green_return = green;
    if (blue_return != NULL) *blue_return = blue;
}

void GLText::draw_text_at(int x, int y, const char* text)
{
    // 要求调用方保证当前上下文有效
    if (font_texture == 0 || text == nullptr || *text == '\0') {
        return; // 没有可用字体纹理或空文本时直接返回，避免产生 GL 错误
    }
    if (program == 0) {
        return; // 着色器程序未就绪时不绘制，避免 GL 错误
    }
    if (viewport_width <= 0 || viewport_height <= 0) {
        return; // 视口尺寸未知时无法正确构造投影
    }
	GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
	if (depth_test == GL_TRUE) {
		glDisable(GL_DEPTH_TEST);
	}

	// 启用混合以正确显示字形透明度
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 使用 GLM 构造屏幕空间正交投影矩阵
    glm::mat4 proj = glm::ortho(0.0f, (float)viewport_width, (float)viewport_height, 0.0f);

	glUseProgram(program);
	glUniformMatrix4fv(loc_uProj, 1, GL_FALSE, &proj[0][0]);
	glUniform3f(loc_uColor, red, green, blue);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_texture);
	glUniform1i(loc_uTex, 0);

    struct Vertex { float x, y, u, v; };
    std::vector<Vertex> verts;
    verts.reserve(strlen(text) * 6);

	float sx = (float)x;
	float sy = (float)y + baseline;
	int w = atlas_width;
	int h = atlas_height;
	for (size_t i = 0; i < strlen(text); ++i) {
		unsigned char ch = (unsigned char)text[i];
		if (ch < 32 || ch >= 128) continue;
		stbtt_aligned_quad q;
		stbtt_GetPackedQuad(cdata, w, h, ch - 32, &sx, &sy, &q, 1);
		// 两个三角形
		verts.push_back({ q.x0, q.y0, q.s0, q.t0 });
		verts.push_back({ q.x1, q.y0, q.s1, q.t0 });
		verts.push_back({ q.x1, q.y1, q.s1, q.t1 });
		verts.push_back({ q.x0, q.y0, q.s0, q.t0 });
		verts.push_back({ q.x1, q.y1, q.s1, q.t1 });
		verts.push_back({ q.x0, q.y1, q.s0, q.t1 });
	}

    // 若无有效字形（例如包含非 ASCII 字符），直接跳过绘制，避免启用非法属性索引
    if (!verts.empty()) {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(Vertex)), verts.data(), GL_DYNAMIC_DRAW);
        if (loc_aPos >= 0) {
            glEnableVertexAttribArray((GLuint)loc_aPos);
            glVertexAttribPointer((GLuint)loc_aPos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, x));
        }
        if (loc_aUV >= 0) {
            glEnableVertexAttribArray((GLuint)loc_aUV);
            glVertexAttribPointer((GLuint)loc_aUV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, u));
        }

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);

	if (depth_test == GL_TRUE) {
		glEnable(GL_DEPTH_TEST);
	}
	// 在断言前捕获并输出GL错误信息
	_GL_CHECK_("GLText::draw() 完成");
	assert(glGetError() == GL_NO_ERROR);
}

void GLText::setViewportSize(int w, int h)
{
    viewport_width = w;
    viewport_height = h;
}

