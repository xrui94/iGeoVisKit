/*
GLText
Author: Rowan James <rowanjames@users.sourceforge.net>

Simple C++ class for nicely drawing text into an OpenGL viewport.

Known issues:
* Coordinates not accurate in case of viewports that don't occupy either
	the full viewport, or aren't flush against bottom edge.  Unsure why.
* Currently supports Windows only.  Font-loading is platform specific.

*/

#ifndef GL_TEXT_H
#define GL_TEXT_H

// GLText 仅负责在当前 OpenGL 上下文中渲染文本，不依赖具体视图组件。

#include <stb_truetype.h>
// 使用 GLM 计算投影矩阵
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GLText {
public:
    // 构造时要求外部确保 OpenGL 上下文已设为当前
    GLText(const char* font_arg, int size_arg);
    virtual ~GLText(void);

    // 设置用于投影的视口尺寸（屏幕像素）。调用方负责在视图尺寸变化时更新。
    void setViewportSize(int w, int h);

    void draw_string(const char* format, ...);
    void draw_string(int x, int y, const char* format, ...);

    void set_color(GLfloat red_arg, GLfloat green_arg, GLfloat blue_arg);
    void get_color(GLfloat* red_return, GLfloat* green_return, GLfloat* blue_return);

private:
    const char* font;
    int size;
    GLuint font_texture;
    int atlas_width;
    int atlas_height;
    float baseline;
    GLfloat red, green, blue;
    stbtt_packedchar cdata[96];
    // 现代渲染管线资源
    GLuint program;
    GLuint vao;
    GLuint vbo;
    // attribute/uniform locations
    GLint loc_aPos;
    GLint loc_aUV;
    GLint loc_uProj;
    GLint loc_uColor;
    GLint loc_uTex;
    // 默认光标位置，供无坐标版本使用
    int last_x;
    int last_y;
    // 与具体视图解耦：仅保存视口尺寸用于构建正交投影
    int viewport_width;
    int viewport_height;

    void draw_text_at(int x, int y, const char* text);
};

#endif
