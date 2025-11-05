#include "PchApp.h"
#include "OverviewGL.h"
#include "utils/Console.h"
#include "opengl/Renderer.h"
#include "opengl/OpenGLContext.h"

#include <shellapi.h>
#include <memory>
// GLM for matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define DEBUG_GL 0

OverviewGL::OverviewGL(ImageFile* image_file, ImageViewport* image_viewport_param)
{
    ImageProperties* image_properties;
    
    assert(image_file != NULL);
    assert(image_viewport_param != NULL);
    
    viewport = image_viewport_param;
    viewport->register_listener(this);
    viewport->get_display_bands(&band_red, &band_green, &band_blue);
    
    image_properties = image_file->getImageProperties();
    image_height = image_properties->getHeight();
    image_width = image_properties->getWidth();
    
    tex_overview_id = 0;
    glInitialized = false;

    // Avoid OpenGL calls here; resources are created lazily on first render
    texture_size = 512; // default until context is available
    tileset = new ImageTileSet(-1, image_file, texture_size, 0);
    LOD_height = tileset->get_LOD_height();
    LOD_width = tileset->get_LOD_width();
}

void OverviewGL::ensureGLResources()
{
    if (glInitialized) return;
    glDisable(GL_DEPTH_TEST);

    // Build shader programs and VBOs for modern pipeline
    auto compile_shader = [](GLenum type, const char* src) -> GLuint {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, NULL);
        glCompileShader(sh);
        GLint ok = GL_FALSE; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) { GLint len = 0; glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len); std::string log(len, '\0'); glGetShaderInfoLog(sh, len, &len, &log[0]); OutputDebugStringA(("OverviewGL shader compile error: " + log + "\n").c_str()); glDeleteShader(sh); return 0; }
        return sh;
    };
    auto link_program = [](GLuint vs, GLuint fs) -> GLuint {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs); glAttachShader(prog, fs); glLinkProgram(prog);
        GLint ok = GL_FALSE; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) { GLint len = 0; glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len); std::string log(len, '\0'); glGetProgramInfoLog(prog, len, &len, &log[0]); OutputDebugStringA(("OverviewGL program link error: " + log + "\n").c_str()); glDeleteProgram(prog); return 0; }
        return prog;
    };

    const char* vsTex =
        "#version 330 core\n"
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aUV;\n"
        "uniform mat4 uProj;\n"
        "out vec2 vUV;\n"
        "void main(){ vUV=aUV; gl_Position=uProj*vec4(aPos,0.0,1.0);}";
    const char* fsTex =
        "#version 330 core\n"
        "in vec2 vUV;\n"
        "uniform sampler2D uTex;\n"
        "uniform bool uFlipY;\n"
        "out vec4 FragColor;\n"
        "void main(){ vec2 uv = vec2(vUV.x, uFlipY ? 1.0 - vUV.y : vUV.y); FragColor = texture(uTex, uv); }";
    GLuint vs1 = compile_shader(GL_VERTEX_SHADER, vsTex);
    GLuint fs1 = compile_shader(GL_FRAGMENT_SHADER, fsTex);
    progTex = (vs1 && fs1) ? link_program(vs1, fs1) : 0;
    if (vs1) glDeleteShader(vs1); if (fs1) glDeleteShader(fs1);
    locTex_aPos = glGetAttribLocation(progTex, "aPos");
    locTex_aUV  = glGetAttribLocation(progTex, "aUV");
    locTex_uProj = glGetUniformLocation(progTex, "uProj");
    locTex_uTex = glGetUniformLocation(progTex, "uTex");
    GLint locTex_uFlipY = glGetUniformLocation(progTex, "uFlipY");
    glUseProgram(progTex);
    // 不在采样阶段翻转 Y，保持与主图一致（投影控制方向）
    if (locTex_uFlipY >= 0) glUniform1i(locTex_uFlipY, 0);

    const char* vsColor =
        "#version 330 core\n"
        "layout(location=0) in vec2 aPos;\n"
        "uniform mat4 uProj;\n"
        "void main(){ gl_Position=uProj*vec4(aPos,0.0,1.0);}";
    const char* fsColor =
        "#version 330 core\n"
        "uniform vec4 uColor;\n"
        "out vec4 FragColor;\n"
        "void main(){ FragColor = uColor; }";
    GLuint vs2 = compile_shader(GL_VERTEX_SHADER, vsColor);
    GLuint fs2 = compile_shader(GL_FRAGMENT_SHADER, fsColor);
    progColor = (vs2 && fs2) ? link_program(vs2, fs2) : 0;
    if (vs2) glDeleteShader(vs2); if (fs2) glDeleteShader(fs2);
    locColor_aPos = glGetAttribLocation(progColor, "aPos");
    locColor_uProj = glGetUniformLocation(progColor, "uProj");
    locColor_uColor = glGetUniformLocation(progColor, "uColor");

    glGenBuffers(1, &vboTile);
    glGenBuffers(1, &vboLines);
    glGenVertexArrays(1, &vaoTile);
    glGenVertexArrays(1, &vaoLines);

    // Tile quad geometry (two triangles) in normalized unit square
    struct V2UV { float x,y,u,v; };
    V2UV quad[6] = {
        {0.f,0.f, 0.f,0.f}, {1.f,0.f, 1.f,0.f}, {1.f,1.f, 1.f,1.f},
        {0.f,0.f, 0.f,0.f}, {1.f,1.f, 1.f,1.f}, {0.f,1.f, 0.f,1.f}
    };
    glBindBuffer(GL_ARRAY_BUFFER, vboTile);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    // Configure tile VAO
    glBindVertexArray(vaoTile);
    glEnableVertexAttribArray((GLuint)locTex_aPos);
    glVertexAttribPointer((GLuint)locTex_aPos, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void*)0);
    glEnableVertexAttribArray((GLuint)locTex_aUV);
    glVertexAttribPointer((GLuint)locTex_aUV, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (const void*)(sizeof(float)*2));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glInitialized = true;
}

OverviewGL::~OverviewGL()
{
    delete tileset;
}

/* Re-draw our overview window */
void OverviewGL::notify_viewport(void)
{
    // 视口变化仅记录，渲染阶段读取最新参数；不进行 OpenGL 调用
}

void OverviewGL::render_scene()
{
    // 由 Renderer + OpenGLContext 集中绘制，不再直接调用 OpenGL
    // 此处仅保证资源已初始化，剩余绘制统一在 GLView::paintGL 中通过回调完成
    ensureGLResources();
}

void OverviewGL::notify_bands(void)
{
    // 仅更新当前波段设置，纹理更新在渲染阶段进行
    viewport->get_display_bands(&band_red, &band_green, &band_blue);
}

void OverviewGL::set_brightness_contrast(float brightness_arg, float contrast_arg)
{
	#if DEBUG_GL
	Console::write("OverviewGL::set_brightness_contrast(%1.4f,%1.4f)\n", brightness_arg, contrast_arg);
	#endif
    // 在现代或不支持 Imaging 子集的上下文中，避免使用 GL_COLOR 矩阵操作。
    // 亮度/对比度效果改为在纹理生成阶段处理；此处仅触发带更新。
    notify_bands();
}

// 在当前上下文中生成或更新概览纹理（由 Renderer 渲染阶段调用）
void OverviewGL::make_texture(void)
{
    if (glIsTexture(tex_overview_id) != GL_TRUE) {
        glGenTextures(1, &tex_overview_id);
        glBindTexture(GL_TEXTURE_2D, (GLuint)tex_overview_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)tex_overview_id);
    }

    viewport->get_display_bands(&band_red, &band_green, &band_blue);
    char* tex_overview = tileset->get_tile_RGB(0, 0, band_red, band_green, band_blue);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_overview);
    delete[] tex_overview;
}
