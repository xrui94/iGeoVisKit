#ifndef _IMAGE_GL_H
#define _IMAGE_GL_H

#include "gui/GLView.h"
#include "opengl/GLText.h"
#include "imagery/ImageFile.h"
#include "imagery/ImageTileSet.h"
#include "opengl/ImageViewport.h"
#include "imagery/ROISet.h"

#include <memory>

class Renderer;
class ShaderProgram;

class ImageGL : public ViewportListener
{
public:
	ImageGL(ImageFile* image_file, ImageViewport* image_viewport_param, ROISet* ROI_set, std::shared_ptr<Renderer> renderer);
	virtual ~ImageGL();

	//void resize_window();

	void resize(int width, int height);

	unsigned char* get_pixel_values(int image_x, int image_y); // remember to delete[]

	void notify_viewport(void);
	void notify_bands(void);

	// set mouse position in image co-ords (for drawing new ROI)	
	void set_mouse_position(int image_x, int image_y) { mouse_x = image_x; mouse_y = image_y; };

	void set_brightness_contrast(float brightnes_arg, float contrast_arg);

	// 拉伸模式：用于控制 8/16/32 位到显示范围的映射
	enum StretchMode { StretchNone, StretchPercentile2_98, StretchPercentile1_99, StretchPercentile5_95 };
	void setStretchMode(StretchMode m);

    // 渲染前确保纹理状态就绪（对 Renderer 公开的包装）
    void ensureTexturesForRender();
    // 渲染前确保着色器/VAO 等 GL 资源已创建（延迟初始化）
    void ensureGLResources();

private:
	/* Helper functions */
	void check_textures(void);
	void check_tileset(void);
	void flush_textures(void);
	void load_tile_tex(int x_index, int y_index);
	GLuint	get_tile_texture(int x_index, int y_index);
	void	free_tile_texture(int x_index, int y_index);
	void	set_tile_texture(int tile_x, int tile_y, GLuint new_id);
	void	add_new_texture(void);
	void draw_rois(void);
	void draw_existing_roi_entity(ROIEntity* entity);
	void draw_new_roi_entity(ROIEntity* entity);
public:
	void render_scene();

	void computeAutoStretchPercentile(int lowPct = 2, int highPct = 98);

public:

	/* Sub-objects */
	//GLView* gl_image;
	//GLText* gl_text;
	ImageFile* image_file;
	ImageViewport* viewport;
	ROISet* roiset;        // set of ROIs to be displayed

	/* State variables */
	int cache_size; // MB maximum decoded tiles kept in cache (static)
	int image_width, image_height; // Image dimensions (static)

	/* Viewport positon locals
		These will change on zoom/scroll */
	int viewport_width, viewport_height; // Image pixels displayed in window
	int viewport_x, viewport_y; // Top-left corner (image pixels)
	int mouse_x, mouse_y;		// Current mouse position in image co-ords (for drawing new ROI)
	int drag_last_x, drag_last_y; // Last mouse pos for left-button drag panning

	/* Image window textures */
	/* Set these up on window resize */
	ImageTileSet* tileset; // Tileset object we populate textures from
	int window_width, window_height; // Screen pixels, GL context dimensions

	// 着色器统一变量位置（图像瓦片）
	int loc_uProj;
	int loc_uZoom; // 屏幕缩放（像素/图像像素）
	int loc_uZoomOffset; // 屏幕缩放锚点偏移（窗口像素）
	// 新增：视口尺寸（屏幕像素），用于在 VS 内部计算 NDC
	int loc_uViewport = -1;
	int loc_uScale = -1;
	int loc_uOffset = -1;
	int loc_uTex = -1; // 采样器
	int loc_uTexScale = -1; // 纹理坐标有效区域缩放（边界瓦片）
	int loc_uAffine = -1; // 图像像素到地理坐标的 2x2 仿射矩阵
	int loc_uOrigin = -1; // 地理坐标中的原点偏移（GT0, GT3）
	//// 新增：像素中心补偿（缩放 + 偏移）
	//int loc_uTexelScale;
	//int loc_uTexelOffset;
	std::vector<GLuint> free_textures; // Which IDs are currently un-used
	std::vector<GLuint> textures; // Which IDs are available (used or not)
	int viewport_start_row, viewport_start_col;
	int viewport_end_row, viewport_end_col;

	/* Set these up on LOD change */
	int LOD; // Sample density factor (0 = 1:1, 1 = 1:2, 2 = 1:4, ...)
	std::vector<GLuint> tile_textures; // Array of texture IDs for correct tile
	int  tile_image_size; // Size of tile in image pixels
	int  tile_rows, tile_cols, tile_count; // Total number of rows/cols 
	int LOD_width, LOD_height; // Scaled dimensions at this LOD (*not* zoom level)

	// 边界瓦片的实际宽高（图像像素），用于非等比缩放
	int last_column_width;
	int last_row_height;

	/* General OpenGL stuff */
	unsigned int list_tile; // display list for textured tile
	bool useDisplayLists;   // whether display lists are available/safe
	//bool useModernPipeline; // whether shader-based pipeline is active
	int texture_size; // Dimension of each texture

	/* Modern pipeline resources */
	std::unique_ptr<ShaderProgram> m_imageShaderProgram = nullptr;
	unsigned int glProgram;
	unsigned int glVbo;
	unsigned int glVao;

	/* Image display uniforms/control */
	int loc_uFlipY;
	bool flipY;
	int minR8, minG8, minB8;
	int maxR8, maxG8, maxB8;
	// 当样本为 16 位时的 2–98% 阈值（以原始 16 位范围存储）
	int minR16, minG16, minB16;
	int maxR16, maxG16, maxB16;
	// Float32 百分位拉伸范围（以源浮点值存储）
	float minR32, minG32, minB32;
	float maxR32, maxG32, maxB32;
	float gammaR, gammaG, gammaB;
	StretchMode stretchMode;

	/* ROI pipeline resources (330 core) */
	std::unique_ptr<ShaderProgram> m_roiShaderProgram = nullptr;
	unsigned int glRoiProgram;
	unsigned int glRoiVbo;
	unsigned int glRoiVao;
	int locRoi_uProj;
	int locRoi_uColor;
	int locRoi_uPointSize;
	int locRoi_uAffine;
	int locRoi_uOrigin;

	/* Band information */
	int band_red;
	int band_green;
	int band_blue;

private:
	bool m_uploaded = false;

	// 顶点 + 纹理坐标数据，单位矩形（两个三角形）
	std::array<float, 24> m_quad;

	std::string m_vsSrc;
	std::string m_fsSrc;

	std::string m_vsRoiSrc;
	std::string m_fsRoiSrc;
};

#endif