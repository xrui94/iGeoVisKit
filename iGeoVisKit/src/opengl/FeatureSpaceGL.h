#ifndef FEATURE_SPACE_GL_H
#define FEATURE_SPACE_GL_H

#include "GLText.h"
#include "PointsHash.h"
#include <memory>

class Renderer;

// 移除 Qt 依赖，实现与 Qt 解耦

// 固定管线特性在 Core Profile 下不可用，禁用点精灵路径
#define USE_POINT_SPRITES 0
// 移除Windows的OpenGL头文件包含，使用Qt的OpenGL支持
// #ifdef _WIN32
// #include <GL/gl.h>
// #include <GL/glu.h>
// // 对于Windows平台，不需要glext.h，使用Windows OpenGL扩展机制
// #else
// // 在非Windows平台上使用glext.h
// #include <GL/glext.h>
// #endif

// 使用 GLM 进行矩阵计算
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class FeatureSpaceGL
{
public:
	// FeatureSpaceGL(HWND hwnd_arg, int LOD_arg, int band1, int band2, int band3);
	FeatureSpaceGL(int LOD_arg, int band1, int band2, int band3);
	// 准备数据后通过 Renderer 渲染
	void draw(void);
	void resize(void);
	
	void translate_cam(float x, float y);
	void rot_cam(float x_diff, float y_diff);
	void zoom_cam(float diff);
    
    int granularity;
        
    static const float degs_to_rad;
    static const float rads_to_deg;

	void add_points(points_hash_t points_hash,
			unsigned char red, unsigned char green, unsigned char blue);
			
    void toggle_smooth(void);

private:
	friend class Renderer;
	// 数据准备：公开给 Renderer 使用
	GLText* gl_text;

	// 在有效的 GL 上下文中初始化着色器与缓冲区
	void ensureGLResources();
		
        void build_box_buffers(void);

	int LOD;
    unsigned int vertices;
    unsigned int num_points;
    // 现代管线资源
	GLuint program;           // 通用颜色着色器程序
	GLint  loc_aPos;
	GLint  loc_aColor;
	GLint  loc_uMVP;

    // 盒子线段 VAO/VBO
    GLuint vaoBox;
    GLuint vboBox;
	GLsizei boxVertexCount;

    // 点云 VAO/VBO（分块存储以避免超大缓冲）
	std::vector<GLuint> pointVaos;
	std::vector<GLuint> pointVbos;
	std::vector<GLsizei> pointCounts;
    

	GLfloat cam_dolly;
	GLfloat z_rot;
	GLfloat pitch;
	GLfloat cam_xpan;
	GLfloat cam_ypan;
    
    bool smooth;
    
    int band1, band2, band3;
    
    // QWidget *widget;  // 保存Qt widget引用（已移除）
};

#endif