#include "PchApp.h"
#include "ImageGL.h"
#include "GLUtils.h"
#include "opengl/Renderer.h"
#include "opengl/OpenGLContext.h"
#include "config.h"
#include "utils/Settings.h"
#include "utils/Console.h"
#include "ShaderProgram.h"

#define DEBUG_GL_TEXTURES 0
#define DEBUG_GL 0

#include <memory>
#include <cmath>

ImageGL::ImageGL(ImageFile* image_file_ptr, ImageViewport* image_viewport_param, ROISet *ROI_set, std::shared_ptr<Renderer> renderer)
{
	m_quad = {
		// 第一个三角形
		0.0f, 0.0f, 0.0f, 0.0f,  // 顶点1: (0,0), 纹理坐标: (0,0)
		1.0f, 0.0f, 1.0f, 0.0f,  // 顶点2: (1,0), 纹理坐标: (1,0)
		1.0f, 1.0f, 1.0f, 1.0f,  // 顶点3: (1,1), 纹理坐标: (1,1)
		// 第二个三角形
		0.0f, 0.0f, 0.0f, 0.0f,  // 顶点1: (0,0), 纹理坐标: (0,0)
		1.0f, 1.0f, 1.0f, 1.0f,  // 顶点3: (1,1), 纹理坐标: (1,1)
		0.0f, 1.0f, 0.0f, 1.0f   // 顶点4: (0,1), 纹理坐标: (0,1)
	};

	//
	m_vsSrc =
		"#version 330 core\n"
		"layout(location=0) in vec2 aPos;\n"
		"layout(location=1) in vec2 aTex;\n"
		"uniform mat4 uProj;\n"
		"uniform vec2 uScale;\n"
		"uniform vec2 uOffset;\n"
		"uniform vec2 uTexScale;\n"	 // 纹理坐标有效区域缩放（边界瓦片）
		"out vec2 vTex;\n"
		"void main(){\n"
		"	vec2 transformedPos = aPos * uScale + uOffset;\n"
		"    // 直接在图像像素坐标系下投影，保持与固定管线 glOrtho(viewport) 一致\n"
		"	vTex = aTex * uTexScale;\n"
		"	gl_Position = uProj * vec4(transformedPos, 0.0, 1.0);\n"
		"}";

	m_fsSrc =
		"#version 330 core\n"
		"in vec2 vTex;\n"
		"uniform sampler2D uTex;\n"
		"uniform bool uFlipY;\n"
		"out vec4 FragColor;\n"
		"void main(){\n"
		"	vec2 uv = vec2(vTex.x, uFlipY ? 1.0 - vTex.y : vTex.y);\n"
		"	FragColor = texture(uTex, uv);\n"
		"}";

	//
	m_vsRoiSrc =
		"#version 330 core\n"
		"layout(location=0) in vec2 aPos;\n"
		"uniform mat4 uProj;\n"
		"uniform float uPointSize;\n"
		"void main(){ gl_Position = uProj * vec4(aPos,0.0,1.0); gl_PointSize = uPointSize; }";

	m_fsRoiSrc =
		"#version 330 core\n"
		"uniform vec3 uColor;\n"
		"out vec4 FragColor;\n"
		"void main(){ FragColor = vec4(uColor,1.0); }";

	//
	ImageProperties* image_properties;

	/* Check for bad arguments */
	//assert (window_widget != NULL);
	assert (image_file_ptr != NULL);
	assert (image_viewport_param != NULL);
	assert (renderer != nullptr);

	/* Copy parameters to locals */
	roiset=ROI_set;
	viewport = image_viewport_param;
	image_file = image_file_ptr;
	/* Grab image properties from image_file */
	image_properties = image_file->getImageProperties();
	image_height = image_properties->getHeight();
	image_width = image_properties->getWidth();
	/* Load cache size from preferences */
	cache_size = settingsFile->getSettingi("preferences","cachesize",128);
	
	/* Create our OpenGL context */
	// 在新的架构中，GLView由ImageWindow管理，不再在ImageGL中直接创建
	
	/* Initialize local variables
	Most of the texture and tile stuff is initialized in flush_textures() */
	LOD = -1; // Invalid value to force creation on first run
	tileset = NULL; // No reference to a tileset untill LOD is decided
	tile_count = 0;
	viewport_x = 0;
	viewport_y = 0;
	viewport_width = 0;
	viewport_height = 0;
	viewport_start_row = 0;
	viewport_start_col = 0;
	viewport_end_row = 0;
	viewport_end_col = 0;
	flush_textures();

	
	/* Initialize OpenGL machine */
    //gl_image->make_current();

	// Write OpenGL extensions to console
	//Console::write((char*) glGetString(GL_EXTENSIONS));

    //// 使用不透明黑色，避免合成时出现白底
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glShadeModel(GL_FLAT);
    //glDisable(GL_DEPTH_TEST);
    

    /* Select texture size */
    {
	    GLint max_texture_size;
		int user_texture_size = settingsFile->getSettingi("preferences","texsize",512);
		if (user_texture_size == 0) user_texture_size = 512;
		{
			int order = -1;
			/* Prevent use of absurdly low values */
			user_texture_size = std::max(user_texture_size, 32);
			
			/* Round to next lowest power of two if not already */
			while (user_texture_size) {
				user_texture_size = user_texture_size>>1;
				order++;
			}
			user_texture_size = int(round(pow(2.0f, order)));
			Console::write("(II) User texture size (rounded) = %d\n", user_texture_size);
		}
	    // glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*) &max_texture_size);
		texture_size = std::min(user_texture_size, max_texture_size);
	}

	//gl_image->resize();

	/* Disable legacy display lists entirely in favor of modern pipeline */
	useDisplayLists = false;
	list_tile = 0;

	/* Set inital mouse coords */
	mouse_x=0;
	mouse_y=0;

    // 初始化拉伸参数与模式
    minR8 = minG8 = minB8 = 0; maxR8 = maxG8 = maxB8 = 255;
    minR16 = minG16 = minB16 = 0; maxR16 = maxG16 = maxB16 = 65535;
    gammaR = gammaG = gammaB = 1.0f;
    // 默认启用 2–98% 百分位拉伸，避免 8/16/32 位影像偏暗或纯白
    stretchMode = StretchPercentile2_98;

	/* Test that we haven't already messed up... */
	// _GL_CHECK_("ImageGL::ImageGL 初始化结束");
	assert(glGetError() == GL_NO_ERROR);

	/* Initial viewport size */
	//resize_window();
	
	/* Start listening for events */
	viewport->register_listener(this);
}

void ImageGL::ensureTexturesForRender()
{
    // 对外部渲染器公开的安全包装，触发必要的纹理检查与更新
    check_textures();
}

void ImageGL::ensureGLResources() {
	// 已初始化则直接返回
	if (m_uploaded) return;

	// 钳制纹理尺寸（避免在构造期访问 GL）
	GLint max_texture_size = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	if (texture_size <= 0) {
		texture_size = 512; // 兜底默认值（2 的幂，足够安全）
	}
	if (max_texture_size > 0) {
		texture_size = std::min(texture_size, (int)max_texture_size);
	}

	//
	m_imageShaderProgram = std::make_unique<ShaderProgram>(ImageGL::m_vsSrc, ImageGL::m_fsSrc);
	m_imageShaderProgram->use();

	// uniform 位置
	glProgram = m_imageShaderProgram->getProgram();
	loc_uTex = m_imageShaderProgram->getUniformLocation("uTex");
	if (loc_uTex >= 0) {
		glUniform1i(loc_uTex, 0);
	}

	loc_uProj = m_imageShaderProgram->getUniformLocation("uProj");
	loc_uTexScale = m_imageShaderProgram->getUniformLocation("uTexScale");
	loc_uScale = m_imageShaderProgram->getUniformLocation("uScale");
	loc_uOffset = m_imageShaderProgram->getUniformLocation("uOffset");
	loc_uFlipY = m_imageShaderProgram->getUniformLocation("uFlipY");
	flipY = false;
	if (loc_uFlipY >= 0) glUniform1i(loc_uFlipY, (GLint)(flipY ? 1 : 0));

	// 顶点 + 纹理坐标数据，单位矩形（两个三角形）
	glGenBuffers(1, &glVbo);
	glBindBuffer(GL_ARRAY_BUFFER, glVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ImageGL::m_quad), ImageGL::m_quad.data(), GL_STATIC_DRAW);
	glGenVertexArrays(1, &glVao);
	glBindVertexArray(glVao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));

	glBindVertexArray(0); // （非必须）防止后续误操作修改此 VAO
	
	//
	m_roiShaderProgram = std::make_unique<ShaderProgram>(ImageGL::m_vsRoiSrc, ImageGL::m_fsRoiSrc);
	m_roiShaderProgram->use();

	glRoiProgram = m_roiShaderProgram->getProgram();
	locRoi_uProj = m_roiShaderProgram->getUniformLocation("uProj");
	locRoi_uColor = m_roiShaderProgram->getUniformLocation("uColor");
	locRoi_uPointSize = m_roiShaderProgram->getUniformLocation("uPointSize");

	glGenBuffers(1, &glRoiVbo);
	glBindBuffer(GL_ARRAY_BUFFER, glRoiVbo);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &glRoiVao);
	glBindVertexArray(glRoiVao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
	glEnable(GL_PROGRAM_POINT_SIZE);

	glBindVertexArray(0); // （非必须）防止后续误操作修改此 VAO

	// 标记当前瓦片的数据已上传至 GPU
	m_uploaded = true;
}

ImageGL::~ImageGL()
{
	if (glVao) {
		glDeleteVertexArrays(1, &glVao);
		glVao = 0;
	}
	if (glRoiVao) {
		glDeleteVertexArrays(1, &glRoiVao);
		glRoiVao = 0;
	}
	if (glRoiVbo) {
		glDeleteBuffers(1, &glRoiVbo);
		glRoiVbo = 0;
	}
	if (glRoiProgram) {
		glDeleteProgram(glRoiProgram);
		glRoiProgram = 0;
	}
	flush_textures();
	tile_textures.clear();
	//delete gl_text;
	//delete gl_image;
	delete tileset;
}


/* Re-draw our window */
void ImageGL::notify_viewport(void)
{
	#if DEBUG_GL
	Console::write("(II) ImageGL::notify_viewport()\n");
	#endif

	/* Update viewport locals */
	viewport_x = viewport->get_image_x(); 
	viewport_y = viewport->get_image_y();
	viewport_width = viewport->get_viewport_width();
	viewport_height = viewport->get_viewport_height();
	// 不在此处进行任何 OpenGL 操作；纹理检查改由渲染回调中执行，
	// 以确保当前 GL 上下文有效并避免阻塞 UI。
}

// 实际绘制逻辑：在 GLView 的 paintGL 中被调用
void ImageGL::render_scene()
{
	// 由 Renderer + OpenGLContext 集中绘制，不再直接调用 OpenGL
	// 仅同步视口数据，供 Renderer 计算投影与图块偏移
	viewport_x = viewport->get_image_x();
	viewport_y = viewport->get_image_y();
	viewport_width = viewport->get_viewport_width();
	viewport_height = viewport->get_viewport_height();
}

/* Draw ROI outlines using 330 core */
void ImageGL::draw_rois(void)
{
	// 已迁移到 Renderer + OpenGLContext 命令集，不再直接调用 OpenGL
}

// draw an ROI entity in the current colour
void ImageGL::draw_existing_roi_entity(ROIEntity *entity)
{
	// 已迁移到 Renderer + OpenGLContext 命令集，不再直接调用 OpenGL
}


// draw an ROI entity in the current colour
void ImageGL::draw_new_roi_entity(ROIEntity *entity)
{
	// 已迁移到 Renderer + OpenGLContext 命令集，不再直接调用 OpenGL
}


void ImageGL::notify_bands(void)
{
	#if DEBUG_GL
	Console::write("(II) ImageGL::notify_bands()\n");
	#endif
	int r, g, b;
	viewport->get_display_bands(&band_red, &band_green, &band_blue);
	// 根据当前拉伸模式计算自动拉伸阈值，并重置可选 gamma
	switch (stretchMode) {
		case StretchNone:
			// 不进行拉伸：保持默认范围
			minR8 = minG8 = minB8 = 0; maxR8 = maxG8 = maxB8 = 255;
			minR16 = minG16 = minB16 = 0; maxR16 = maxG16 = maxB16 = 65535;
			minR32 = minG32 = minB32 = 0.0f; maxR32 = maxG32 = maxB32 = 1.0f;
			break;
		case StretchPercentile2_98:
			computeAutoStretchPercentile(2, 98);
			break;
		case StretchPercentile1_99:
			computeAutoStretchPercentile(1, 99);
			break;
		case StretchPercentile5_95:
			computeAutoStretchPercentile(5, 95);
			break;
	}
	gammaR = 1.0f; gammaG = 1.0f; gammaB = 1.0f;

	tile_textures.assign(tile_count, 0);
	free_textures = textures;
	notify_viewport();
}

void ImageGL::setStretchMode(StretchMode m)
{
    // 更新模式并按需重新计算百分位阈值
    stretchMode = m;
    switch (stretchMode) {
        case StretchNone:
            minR8 = minG8 = minB8 = 0; maxR8 = maxG8 = maxB8 = 255;
            minR16 = minG16 = minB16 = 0; maxR16 = maxG16 = maxB16 = 65535;
            minR32 = minG32 = minB32 = 0.0f; maxR32 = maxG32 = maxB32 = 1.0f;
            break;
        case StretchPercentile2_98:
            computeAutoStretchPercentile(2, 98);
            break;
        case StretchPercentile1_99:
            computeAutoStretchPercentile(1, 99);
            break;
        case StretchPercentile5_95:
            computeAutoStretchPercentile(5, 95);
            break;
    }
    // 刷新波段并强制瓦片纹理重新生成，确保拉伸立即生效
    notify_bands();
}

void ImageGL::check_textures(void)
{
	check_tileset();
	
	// 调试输出：检查视口参数
	printf("DEBUG: check_textures - viewport: x=%d, y=%d, w=%d, h=%d\n", 
		   viewport_x, viewport_y, viewport_width, viewport_height);
	printf("DEBUG: check_textures - tile_image_size=%d, tile_rows=%d, tile_cols=%d\n", 
		   tile_image_size, tile_rows, tile_cols);
			
	/* Find new display rect */
	int new_start_row, new_start_col, new_end_row, new_end_col;
	new_start_col = viewport_x / tile_image_size;
	new_start_row = viewport_y / tile_image_size;

    // 使用向上取整确保部分可见的瓦片也被包含，并留一列/一行裕量避免滚动时出现间隙
    //new_end_col = new_start_col + (int)ceil((double)viewport_width / (double)tile_image_size) + 1;
    //new_end_row = new_start_row + (int)ceil((double)viewport_height / (double)tile_image_size) + 1;
	new_end_col = new_start_col + (viewport_width / tile_image_size) + 1;
	new_end_row = new_start_row + (viewport_height / tile_image_size) + 1;

	new_end_row = std::min(new_end_row, tile_rows - 1);
	new_end_col = std::min(new_end_col, tile_cols - 1);
	
	// 调试输出：检查计算的瓦片范围
	printf("DEBUG: check_textures - tile range: col[%d-%d], row[%d-%d]\n", 
		   new_start_col, new_end_col, new_start_row, new_end_row);


	/* free un-used texture IDs */
	//gl_image->make_current();

	assert(tileset != NULL);

	/* Compare new exposed tiles to old */
	/* Delete old & ! new */
	// For each column within old or new set bounds
	for (int x = std::min(viewport_start_col, new_start_col); x <= std::max(viewport_end_col, new_end_col); x++) {
		// if this column is within the new set, load it
		if ((x >= new_start_col) && (x <= new_end_col)) {
			for (int y = new_start_row; y <= new_end_row; y++) {
				load_tile_tex(x, y);
			}
		}
		else { // unload it
			for (int y = viewport_start_row; y <= viewport_end_row; y++) {
				free_tile_texture(x, y);
			}
		}
	}

	// For each row within old or new set bounds
	for (int y = std::min(viewport_start_row, new_start_row); y <= std::max(viewport_end_row, new_end_row); y++) {
		// If this row is within the new set, load it
		if ((y >= new_start_row) && (y <= new_end_row)) {
			for (int x = new_start_col; x <= new_end_col; x++) {
				load_tile_tex(x, y);
			}
		}
		else { // unload it
			for (int x = viewport_start_col; x <= viewport_end_col; x++) {
				free_tile_texture(x, y);
			}
		}
	}

	viewport_start_row = new_start_row;
	viewport_start_col = new_start_col;
	viewport_end_row = new_end_row;
	viewport_end_col = new_end_col;
	

	/* Have we messed up? */
	//_GL_CHECK_("ImageGL::check_textures 更新视口范围后");
	assert(glGetError() == GL_NO_ERROR);
}

void ImageGL::load_tile_tex(int x_index, int y_index)
{
#if DEBUG_GL_TEXTURES
	Console::write("(II) load_tile_tex(%d, %d)\n", x_index, y_index);
#endif
	char* tex_data;
	// 使用 tileset 的实际 tile 尺寸，避免 LOD 下坐标偏移
	int tile_x = x_index * tile_image_size;
	int tile_y = y_index * tile_image_size;
	GLuint tex_id;
	
	// Only load if not already
	if (get_tile_texture(x_index, y_index) != 0) return;
	
	// 防御：确保存在可用纹理 ID，避免对空 vector 调用 back()
	if (free_textures.empty()) {
		add_new_texture();
		if (free_textures.empty()) {
			Console::write("(EE) load_tile_tex: no free textures available after add_new_texture.\n");
			return; // 无法继续安全上传纹理
		}
	}
	// Load the tile data
	tex_data = tileset->get_tile_RGB(tile_x,tile_y,band_red,band_green,band_blue);
	
	// 调试输出：检查纹理数据
	printf("DEBUG: Loading tile (%d,%d) at image pos (%d,%d), tex_data=%p\n", 
		   x_index, y_index, tile_x, tile_y, tex_data);
	if (tex_data) {
		printf("DEBUG: First few pixels: R=%d G=%d B=%d\n", 
			   (unsigned char)tex_data[0], (unsigned char)tex_data[1], (unsigned char)tex_data[2]);
	}
	
	// Grab a free texture ID
	// 由于可能在极端情况下仍为空（如 GL 初始化失败），再次防御检查
	if (free_textures.empty()) {
		Console::write("(EE) load_tile_tex: free_textures unexpectedly empty before pop_back.\n");
		return;
	}
	tex_id = free_textures.back();
	free_textures.pop_back();
	
    // 在上传前进行每通道自动拉伸与可选 gamma 校正
    //gl_image->make_current();

    assert(glIsTexture(tex_id) == GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (tex_data) {
        int count = texture_size * texture_size;
        int sampleBytes = tileset ? tileset->get_sample_size() : 1;
        float gR = gammaR <= 0.0f ? 1.0f : gammaR;
        float gG = gammaG <= 0.0f ? 1.0f : gammaG;
        float gB = gammaB <= 0.0f ? 1.0f : gammaB;
        if (sampleBytes == 1) {
            // 若为单波段且存在调色板，则先将索引展开为 RGB
            int bands = image_file && image_file->getImageProperties() ? image_file->getImageProperties()->getNumBands() : 0;
            bool expandedPalette = false;
            if (bands == 1) {
                BandInfo* bi = image_file->getBandInfo(1);
                if (bi && bi->getBand()) {
                    GDALRasterBand* gb = bi->getBand();
                    GDALColorTableH ct = GDALGetRasterColorTable(gb);
                    if (ct) {
                        unsigned char* idx = (unsigned char*)tex_data;
                        unsigned char* palRGB = new unsigned char[count * 3];
                        int entryCount = GDALGetColorEntryCount(ct);
                        for (int i = 0; i < count; ++i) {
                            int o = i * 3;
                            int id = (int)idx[o]; // 使用第一个通道作为索引
                            if (id < 0 || id >= entryCount) {
                                palRGB[o]   = (unsigned char)id;
                                palRGB[o+1] = (unsigned char)id;
                                palRGB[o+2] = (unsigned char)id;
                            } else {
                                const GDALColorEntry* e = GDALGetColorEntry(ct, id);
                                palRGB[o]   = (unsigned char)e->c1;
                                palRGB[o+1] = (unsigned char)e->c2;
                                palRGB[o+2] = (unsigned char)e->c3;
                            }
                        }
                        delete[] tex_data;
                        tex_data = (char*)palRGB;
                        expandedPalette = true;
                    }
                }
            }
            // 若关闭拉伸，或刚刚展开了调色板（颜色已映射），则直接上传
            if (stretchMode == StretchNone || expandedPalette) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
                delete[] tex_data;
            } else {
                int rmin = minR8, rmax = maxR8; if (rmax <= rmin) { rmin = 0; rmax = 255; }
                int gmin = minG8, gmax = maxG8; if (gmax <= gmin) { gmin = 0; gmax = 255; }
                int bmin = minB8, bmax = maxB8; if (bmax <= bmin) { bmin = 0; bmax = 255; }
                float invR = (rmax > rmin) ? 1.0f / float(rmax - rmin) : 1.0f;
                float invG = (gmax > gmin) ? 1.0f / float(gmax - gmin) : 1.0f;
                float invB = (bmax > bmin) ? 1.0f / float(bmax - bmin) : 1.0f;
                for (int i = 0; i < count; ++i) {
                    int o = i * 3;
                    float r = (float)(unsigned char)tex_data[o];
                    float g = (float)(unsigned char)tex_data[o+1];
                    float b = (float)(unsigned char)tex_data[o+2];
                    float nr = (r - float(rmin)) * invR; if (nr < 0.f) nr = 0.f; if (nr > 1.f) nr = 1.f; nr = powf(nr, 1.0f / gR);
                    float ng = (g - float(gmin)) * invG; if (ng < 0.f) ng = 0.f; if (ng > 1.f) ng = 1.f; ng = powf(ng, 1.0f / gG);
                    float nb = (b - float(bmin)) * invB; if (nb < 0.f) nb = 0.f; if (nb > 1.f) nb = 1.f; nb = powf(nb, 1.0f / gB);
                    tex_data[o]   = (unsigned char)(nr * 255.0f + 0.5f);
                    tex_data[o+1] = (unsigned char)(ng * 255.0f + 0.5f);
                    tex_data[o+2] = (unsigned char)(nb * 255.0f + 0.5f);
                }
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
                delete[] tex_data;
            }
        } else if (sampleBytes == 2) {
            // 根据源数据类型选择有符号/无符号指针，并获取 NoData
            int probeBand = band_red > 0 ? band_red : 1;
            bool isInt16 = false;
            if (image_file && image_file->getBandInfo(probeBand)) {
                std::string dt = image_file->getBandInfo(probeBand)->getRasterDataType();
                // GDAL 返回的名称通常为 "Int16" 或 "UInt16"
                isInt16 = (dt.find("Int16") != std::string::npos) && (dt.find("UInt16") == std::string::npos);
            }

            // 获取各通道 NoData（若存在则用于屏蔽）
            bool hasNoDataR = false, hasNoDataG = false, hasNoDataB = false;
            double ndR = (band_red > 0)   ? image_file->getNoDataValue(band_red, &hasNoDataR)   : 0.0;
            double ndG = (band_green > 0) ? image_file->getNoDataValue(band_green, &hasNoDataG) : 0.0;
            double ndB = (band_blue > 0)  ? image_file->getNoDataValue(band_blue, &hasNoDataB)  : 0.0;

            unsigned char* out8 = new unsigned char[count * 3];
            int rmin = minR16, rmax = maxR16; if (stretchMode == StretchNone) { rmin = 0; rmax = 65535; }
            int gmin = minG16, gmax = maxG16; if (stretchMode == StretchNone) { gmin = 0; gmax = 65535; }
            int bmin = minB16, bmax = maxB16; if (stretchMode == StretchNone) { bmin = 0; bmax = 65535; }
            float invR = (rmax > rmin) ? 1.0f / float(rmax - rmin) : 1.0f;
            float invG = (gmax > gmin) ? 1.0f / float(gmax - gmin) : 1.0f;
            float invB = (bmax > bmin) ? 1.0f / float(bmax - bmin) : 1.0f;

            if (isInt16) {
                short* s16 = (short*)tex_data;
                short ndRs = (short)ndR, ndGs = (short)ndG, ndBs = (short)ndB;
                for (int i = 0; i < count; ++i) {
                    int o = i * 3;
                    short r16 = s16[o];
                    short g16 = s16[o+1];
                    short b16 = s16[o+2];
                    bool isNoData = (hasNoDataR && r16 == ndRs) || (hasNoDataG && g16 == ndGs) || (hasNoDataB && b16 == ndBs);
                    if (isNoData) {
                        out8[o] = out8[o+1] = out8[o+2] = 0;
                        continue;
                    }
                    float r = (float)r16, g = (float)g16, b = (float)b16;
                    float nr = (r - float(rmin)) * invR; if (nr < 0.f) nr = 0.f; if (nr > 1.f) nr = 1.f; nr = powf(nr, 1.0f / gR);
                    float ng = (g - float(gmin)) * invG; if (ng < 0.f) ng = 0.f; if (ng > 1.f) ng = 1.f; ng = powf(ng, 1.0f / gG);
                    float nb = (b - float(bmin)) * invB; if (nb < 0.f) nb = 0.f; if (nb > 1.f) nb = 1.f; nb = powf(nb, 1.0f / gB);
                    out8[o]   = (unsigned char)(nr * 255.0f + 0.5f);
                    out8[o+1] = (unsigned char)(ng * 255.0f + 0.5f);
                    out8[o+2] = (unsigned char)(nb * 255.0f + 0.5f);
                }
            } else {
                unsigned short* u16 = (unsigned short*)tex_data;
                unsigned short ndRu = (unsigned short)ndR, ndGu = (unsigned short)ndG, ndBu = (unsigned short)ndB;
                for (int i = 0; i < count; ++i) {
                    int o = i * 3;
                    unsigned short r16 = u16[o];
                    unsigned short g16 = u16[o+1];
                    unsigned short b16 = u16[o+2];
                    bool isNoData = (hasNoDataR && r16 == ndRu) || (hasNoDataG && g16 == ndGu) || (hasNoDataB && b16 == ndBu);
                    if (isNoData) {
                        out8[o] = out8[o+1] = out8[o+2] = 0;
                        continue;
                    }
                    float r = (float)r16, g = (float)g16, b = (float)b16;
                    float nr = (r - float(rmin)) * invR; if (nr < 0.f) nr = 0.f; if (nr > 1.f) nr = 1.f; nr = powf(nr, 1.0f / gR);
                    float ng = (g - float(gmin)) * invG; if (ng < 0.f) ng = 0.f; if (ng > 1.f) ng = 1.f; ng = powf(ng, 1.0f / gG);
                    float nb = (b - float(bmin)) * invB; if (nb < 0.f) nb = 0.f; if (nb > 1.f) nb = 1.f; nb = powf(nb, 1.0f / gB);
                    out8[o]   = (unsigned char)(nr * 255.0f + 0.5f);
                    out8[o+1] = (unsigned char)(ng * 255.0f + 0.5f);
                    out8[o+2] = (unsigned char)(nb * 255.0f + 0.5f);
                }
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, out8);
            delete[] tex_data;
            delete[] out8;
        } else if (sampleBytes == 4) {
            // Float32：优先使用基于全图 2–98% 的百分位范围，其次 BandInfo 的 Min/Max，最后回退 0–1
            bool hasNoDataR = false, hasNoDataG = false, hasNoDataB = false;
            double ndR = (band_red > 0)   ? image_file->getNoDataValue(band_red, &hasNoDataR)   : 0.0;
            double ndG = (band_green > 0) ? image_file->getNoDataValue(band_green, &hasNoDataG) : 0.0;
            double ndB = (band_blue > 0)  ? image_file->getNoDataValue(band_blue, &hasNoDataB)  : 0.0;

            // 先尝试使用百分位范围
            bool havePctR = (maxR32 > minR32);
            bool havePctG = (maxG32 > minG32);
            bool havePctB = (maxB32 > minB32);
            float minR = havePctR ? minR32 : 0.0f;
            float maxR = havePctR ? maxR32 : 1.0f;
            float minG = havePctG ? minG32 : 0.0f;
            float maxG = havePctG ? maxG32 : 1.0f;
            float minB = havePctB ? minB32 : 0.0f;
            float maxB = havePctB ? maxB32 : 1.0f;

            // 若某一通道没有百分位结果，则尝试 BandInfo 的全局范围
            auto tryBandInfo = [&](int band, float& outMin, float& outMax) {
                if (outMax > outMin) return; // 已有有效范围
                if (band > 0 && image_file && image_file->getBandInfo(band)) {
                    BandInfo* bi = image_file->getBandInfo(band);
                    double mn = bi->getDataMin(); double mx = bi->getDataMax();
                    if (mx > mn) { outMin = (float)mn; outMax = (float)mx; }
                }
            };
            tryBandInfo(band_red,   minR, maxR);
            tryBandInfo(band_green, minG, maxG);
            tryBandInfo(band_blue,  minB, maxB);

            float invR = (maxR > minR) ? 1.0f / (maxR - minR) : 1.0f;
            float invG = (maxG > minG) ? 1.0f / (maxG - minG) : 1.0f;
            float invB = (maxB > minB) ? 1.0f / (maxB - minB) : 1.0f;

            const float* f32 = (const float*)tex_data;
            unsigned char* out8 = new unsigned char[count * 3];
            for (int i = 0; i < count; ++i) {
                int o = i * 3;
                float r = f32[o], g = f32[o+1], b = f32[o+2];
                bool isNoData = !std::isfinite(r) || !std::isfinite(g) || !std::isfinite(b) ||
                                (hasNoDataR && r == (float)ndR) || (hasNoDataG && g == (float)ndG) || (hasNoDataB && b == (float)ndB);
                if (isNoData) { out8[o] = out8[o+1] = out8[o+2] = 0; continue; }
                float nr = (r - minR) * invR; if (nr < 0.f) nr = 0.f; if (nr > 1.f) nr = 1.f; nr = powf(nr, 1.0f / gR);
                float ng = (g - minG) * invG; if (ng < 0.f) ng = 0.f; if (ng > 1.f) ng = 1.f; ng = powf(ng, 1.0f / gG);
                float nb = (b - minB) * invB; if (nb < 0.f) nb = 0.f; if (nb > 1.f) nb = 1.f; nb = powf(nb, 1.0f / gB);
                out8[o]   = (unsigned char)(nr * 255.0f + 0.5f);
                out8[o+1] = (unsigned char)(ng * 255.0f + 0.5f);
                out8[o+2] = (unsigned char)(nb * 255.0f + 0.5f);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, out8);
            delete[] tex_data;
            delete[] out8;
        } else {
            // Fallback: treat as 8-bit
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
            delete[] tex_data;
        }
        printf("DEBUG: Uploaded texture ID %u, size %dx%d\n", tex_id, texture_size, texture_size);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
		_GL_CHECK_("ImageGL::load_tile_tex 纹理上传后");
		assert(glGetError() == GL_NO_ERROR);
	assert(glIsTexture(tex_id) == GL_TRUE);

    // Update the tile involved to show the new texture ID
    set_tile_texture(x_index, y_index, tex_id);
}

void ImageGL::check_tileset(void)
{
	/* Find needed LOD */
	float tmp_zoom = 0.5f;
	int needed_LOD = 0;
	float zl = viewport->get_zoom_level();
	// 防御：当缩放未初始化（0 或更小）时，采用 LOD=0，避免无限循环
	if (zl <= 0.0f) {
		needed_LOD = 0;
	} else {
		// 设定一个合理上限，避免在极端情况下长时间迭代
		const int LOD_LIMIT = 16;
		while (tmp_zoom >= zl && needed_LOD < LOD_LIMIT) {
			tmp_zoom = tmp_zoom / 2.0f;
			needed_LOD++;
		}
	}

	/* Compare with current.
		If not the same, we need to do a few things... */
	if (needed_LOD != LOD) {
#if DEBUG_GL
		Console::write("Changing LOD to ");
		Console::write(needed_LOD);
		Console::write("\n");
#endif
		// Free old tileset and load new
		if (tileset != NULL) delete tileset;
		LOD = needed_LOD;

		if (texture_size <= 0) {
			Console::write("(EE) texture_size invalid (%d). Clamping to 512.\n", texture_size);
			texture_size = 512;
		}
		
		tileset = new ImageTileSet(LOD, image_file, texture_size, cache_size);

		// [re-]initialize the texture ID array
		tile_rows = tileset->get_rows();
		tile_cols = tileset->get_columns();
		tile_count = tile_rows * tile_cols;
		#if DEBUG_GL
		Console::write("Allocating  ");
		Console::write(tile_count);
		Console::write(" tiles.\n");
		#endif
		tile_textures.assign(tile_count, 0);
			
        // 初始化视口起始行/列为 0，避免越界（此前调试硬编码 400 会导致崩溃）
        viewport_start_col = 0;
        viewport_start_row = 0;
		viewport_end_col = -1;
		viewport_end_row = -1;
			
		/* All textures can be re-used */
		free_textures = textures;
		
		/* find size of tile in image space */
		tile_image_size = texture_size;
		for (int x = 0; x < LOD; x++) {
			tile_image_size = tile_image_size * 2;
		}

		// 缓存边界瓦片的实际尺寸，供渲染阶段做非等比缩放
		last_column_width = tileset->get_last_column_width();
		last_row_height = tileset->get_last_row_height();
	}
	#if DEBUG_GL
	else {
		Console::write("Leaving LOD at ");
		Console::write(LOD);
		Console::write("\n");
	}
	#endif
}

void ImageGL::flush_textures(void)
{
	/* De-allocate all our textures */
	//gl_image->make_current();
	while (!textures.empty()) {
		GLuint my_tex;
		my_tex = textures.back();
		textures.pop_back();
		glDeleteTextures(1, &my_tex);
	}
	/* Clean out free texture vector */
	free_textures.clear();
}

//void ImageGL::resize_window()
//{
//#if DEBUG_GL
//	Console::write("(II) Resize window triggered.\n");
//#endif
//
//	/* Re-size the OpenGL context */
//	gl_image->resize();	// 改为在 ImageWindow 中设置渲染回调时，进行 resize
//
//	// We use this as an invalid test case later, so we don't want it to work...
//	assert(glIsTexture(0) == GL_FALSE);
//
//	// 防御性：仅在尺寸有效时更新视口，避免首次为 0 导致黑屏
//	int w = gl_image->width();
//	int h = gl_image->height();
//	//if (w <= 0 || h <= 0) {
//	//	// 尝试使用控件当前可用的大小作为回退
//	//	QSize sz = gl_image->size();
//	//	w = sz.width();
//	//	h = sz.height();
//	//}
//	//if (w > 0 && h > 0) {
//	//	viewport->set_window_size(w, h);
//	//}
//	viewport->set_window_size(w, h);
//}

void ImageGL::resize(int width, int height)
{
	// We use this as an invalid test case later, so we don't want it to work...
	assert(glIsTexture(0) == GL_FALSE);

	viewport->set_window_size(width, height);


	/* Update viewport locals */
	viewport_x = viewport->get_image_x();
	viewport_y = viewport->get_image_y();
	viewport_width = viewport->get_viewport_width();
	viewport_height = viewport->get_viewport_height();
}

void ImageGL::add_new_texture(void)
{
	//gl_image->make_current();

	/* Get another texture ID and set it up */
	GLuint new_tex;
	GLint proxy_width; // used for checking available video memory
	glGenTextures(1, &new_tex);
	glBindTexture(GL_TEXTURE_2D, new_tex);
	/* Set up wrapping and filtering parameters for this texture */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	_GL_CHECK_("ImageGL::add_new_texture 设置参数后");

	// 若生成失败或未被驱动识别为有效纹理，避免向容器插入无效 ID
	if (!glIsTexture(new_tex)) {
		Console::write("(EE) add_new_texture: glGenTextures returned invalid texture id.\n");
		return;
	}
		
	/* Load proxy texture to check for enough space */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &proxy_width);
	/* And abort if not */
	assert(proxy_width == texture_size);
	_GL_CHECK_("ImageGL::add_new_texture 代理纹理后");
	
	/* Now, we know the new texture is valid, and we can start using it */
	textures.push_back(new_tex);
	free_textures.push_back(new_tex);
}

unsigned char* ImageGL::get_pixel_values(int image_x, int image_y)
{
	if (tileset != NULL)
		return tileset->get_pixel_values(image_x, image_y);
		
	/* If we don't currently have a tileset, the values will be wrong anyway,
		so we should just avoid delete[]'ing memory we haven't new[]'ed.
		This might occur for a few instants between the window being shown and
		the tileset being loaded... or something */
	return new unsigned char[image_file->getImageProperties()->getNumBands()];
}

// Return the texture id of a tile at column/row index:
GLuint ImageGL::get_tile_texture(int x_index, int y_index) {
	int tile_index = y_index*tile_cols + x_index;
	assert(tile_textures.size() > tile_index);
	#if DEBUG_GL_TEXTURES
	Console::write("(II) get_tile_texture(%d, %d) - tile_textures[%d]-> %d\n",
			x_index, y_index, tile_index, tile_textures.at(tile_index) );
	#endif
	return tile_textures.at(tile_index);
}
// Set the texture id of a tile at column/row index:
void ImageGL::set_tile_texture(int x_index, int y_index, GLuint new_id) {
	int tile_index = y_index*tile_cols + x_index;
	assert(tile_textures.size() > tile_index);
	#if DEBUG_GL_TEXTURES
	Console::write("(II) set_tile_texture(%d, %d) - tile_textures[%d] = %d\n",
			x_index, y_index, tile_index, new_id);
	#endif
	tile_textures[tile_index] = new_id;
}

void ImageGL::free_tile_texture(int x_index, int y_index)
{
	#if DEBUG_GL_TEXTURES
	Console::write("(II) free_tile_texture(%d, %d)\n", x_index, y_index);
	#endif
	GLuint tex_id;
	int tile_index = y_index*tile_cols + x_index;
	assert(tile_textures.size() > tile_index);
	tex_id = tile_textures[tile_index];
	if (tex_id != 0) {
		free_textures.push_back(tex_id);
		tile_textures[tile_index] = 0;
	}
}

void ImageGL::set_brightness_contrast(float brightness_arg, float contrast_arg)
{
	#if DEBUG_GL
	Console::write("ImageGL::set_brightness_contrast(%1.4f,%1.4f)\n", brightness_arg, contrast_arg);
	#endif
	
	//gl_image->make_current();
	glPushAttrib(GL_MATRIX_MODE);
	glMatrixMode(GL_COLOR);
	glLoadIdentity();

	// Set brightness
	glTranslatef(brightness_arg,brightness_arg,brightness_arg);
	// Set contrast
	glScalef(contrast_arg,contrast_arg,contrast_arg);

	glPopAttrib();
	notify_bands();
}

/* Compute per-channel auto-stretch percentiles (e.g., 2–98%) for current bands */
void ImageGL::computeAutoStretchPercentile(int lowPct, int highPct)
{
	if (!image_file) return;
	int w = image_file->getImageProperties() ? image_file->getImageProperties()->getWidth() : 0;
	int h = image_file->getImageProperties() ? image_file->getImageProperties()->getHeight() : 0;
	if (w <= 0 || h <= 0) return;

	// 钳制波段索引，避免 1 波段影像时越界访问
	int bandsCount = image_file->getImageProperties() ? image_file->getImageProperties()->getNumBands() : 0;
	int br = band_red; int bg = band_green; int bb = band_blue;
	if (bandsCount > 0) {
		if (br > bandsCount) br = bandsCount;
		if (bg > bandsCount) bg = bandsCount;
		if (bb > bandsCount) bb = bandsCount;
	}

	// 获取各波段的 NoData 值
	bool hasNoDataR = false, hasNoDataG = false, hasNoDataB = false;
	double noDataR = 0, noDataG = 0, noDataB = 0;
	if (br > 0) noDataR = image_file->getNoDataValue(br, &hasNoDataR);
	if (bg > 0) noDataG = image_file->getNoDataValue(bg, &hasNoDataG);
	if (bb > 0) noDataB = image_file->getNoDataValue(bb, &hasNoDataB);
	
	int sampleBytes = tileset ? tileset->get_sample_size() : image_file->getSampleSizeBytes();
	if (sampleBytes <= 1) {
		// 8 位：也改为 GDAL 分块采样，避免因瓦片缓存或 LOD 影响统计
		const int bins = 256;
		std::vector<int> histR(bins, 0), histG(bins, 0), histB(bins, 0);
		int blockW = std::min(256, w);
		int blockH = std::min(256, h);
		int stepX = std::max(blockW, w / 16);
		int stepY = std::max(blockH, h / 16);
		int bands = bandsCount;
		for (int y0 = 0; y0 < h; y0 += stepY) {
			for (int x0 = 0; x0 < w; x0 += stepX) {
				int rw = std::min(blockW, w - x0);
				int rh = std::min(blockH, h - y0);
				int bufBytes = rw * rh * bands * 1;
				char* buf = new char[bufBytes];
				image_file->getRasterData(rw, rh, x0, y0, buf, rw, rh);
				unsigned char* p8 = (unsigned char*)buf;
				for (int j = 0; j < rh; ++j) {
					for (int i = 0; i < rw; ++i) {
						int base = (i * bands) + (j * rw * bands);
						unsigned char r = (br   ? p8[base + (br   - 1)] : 0);
						unsigned char g = (bg ? p8[base + (bg - 1)] : 0);
						unsigned char b = (bb  ? p8[base + (bb  - 1)] : 0);
						
						// 排除 NoData 值
						bool isValidR = !hasNoDataR || (r != (unsigned char)noDataR);
						bool isValidG = !hasNoDataG || (g != (unsigned char)noDataG);
						bool isValidB = !hasNoDataB || (b != (unsigned char)noDataB);
						
						if (isValidR) histR[(int)r]++;
						if (isValidG) histG[(int)g]++;
						if (isValidB) histB[(int)b]++;
					}
				}
				delete[] buf;
			}
		}
		auto find_range8 = [&](const std::vector<int>& hist, int& outMin, int& outMax) {
			long long total = 0; for (int v : hist) total += v; if (total == 0) { outMin = 0; outMax = 255; return; }
			long long lowT = (long long)(total * (lowPct / 100.0));
			long long highT = (long long)(total * (highPct / 100.0));
			long long acc = 0; int lo = 0; for (int i = 0; i < bins; ++i) { acc += hist[i]; if (acc >= lowT) { lo = i; break; } }
			acc = 0; int hi = 255; for (int i = 0; i < bins; ++i) { acc += hist[i]; if (acc >= highT) { hi = i; break; } }
			outMin = lo; outMax = hi;
		};
		find_range8(histR, minR8, maxR8);
		find_range8(histG, minG8, maxG8);
		find_range8(histB, minB8, maxB8);
	} else if (sampleBytes == 2) {
		// 16 位：基于采样块构建 0..65535 直方图
		const int bins = 65536;
		std::vector<int> histR(bins, 0), histG(bins, 0), histB(bins, 0);
		int blockW = std::min(128, w);
		int blockH = std::min(128, h);
		int stepX = std::max(blockW, w / 16);
		int stepY = std::max(blockH, h / 16);
		int bands = bandsCount;
		for (int y0 = 0; y0 < h; y0 += stepY) {
			for (int x0 = 0; x0 < w; x0 += stepX) {
				int rw = std::min(blockW, w - x0);
				int rh = std::min(blockH, h - y0);
				int bufBytes = rw * rh * bands * sampleBytes;
				char* buf = new char[bufBytes];
				image_file->getRasterData(rw, rh, x0, y0, buf, rw, rh);
				unsigned short* p16 = (unsigned short*)buf;
				for (int j = 0; j < rh; ++j) {
					for (int i = 0; i < rw; ++i) {
						int base = (i * bands) + (j * rw * bands);
						unsigned short r = (br   ? p16[base + (br   - 1)] : 0);
						unsigned short g = (bg ? p16[base + (bg - 1)] : 0);
						unsigned short b = (bb  ? p16[base + (bb  - 1)] : 0);
						
						// 排除 NoData 值
						bool isValidR = !hasNoDataR || (r != (unsigned short)noDataR);
						bool isValidG = !hasNoDataG || (g != (unsigned short)noDataG);
						bool isValidB = !hasNoDataB || (b != (unsigned short)noDataB);
						
						if (isValidR) histR[(int)r]++;
						if (isValidG) histG[(int)g]++;
						if (isValidB) histB[(int)b]++;
					}
				}
				delete[] buf;
			}
		}
		auto find_range16 = [&](const std::vector<int>& hist, int& outMin, int& outMax) {
			long long total = 0; for (int v : hist) total += v; if (total == 0) { outMin = 0; outMax = 65535; return; }
			long long lowT = (long long)(total * (lowPct / 100.0));
			long long highT = (long long)(total * (highPct / 100.0));
			long long acc = 0; int lo = 0; for (int i = 0; i < bins; ++i) { acc += hist[i]; if (acc >= lowT) { lo = i; break; } }
			acc = 0; int hi = 65535; for (int i = 0; i < bins; ++i) { acc += hist[i]; if (acc >= highT) { hi = i; break; } }
			outMin = lo; outMax = hi;
		};
		find_range16(histR, minR16, maxR16);
		find_range16(histG, minG16, maxG16);
		find_range16(histB, minB16, maxB16);
	} else if (sampleBytes == 4) {
		// Float32：采样像素集合，计算 2–98% 百分位，过滤 NaN/Inf，避免依赖 GDAL Min/Max
		std::vector<float> sampR; std::vector<float> sampG; std::vector<float> sampB;
		sampR.reserve(200000); sampG.reserve(200000); sampB.reserve(200000);
		int blockW = std::min(128, w);
		int blockH = std::min(128, h);
		int stepX = std::max(blockW, w / 16);
		int stepY = std::max(blockH, h / 16);
		int bands = bandsCount;
		for (int y0 = 0; y0 < h; y0 += stepY) {
			for (int x0 = 0; x0 < w; x0 += stepX) {
				int rw = std::min(blockW, w - x0);
				int rh = std::min(blockH, h - y0);
				int bufBytes = rw * rh * bands * sampleBytes;
				char* buf = new char[bufBytes];
				image_file->getRasterData(rw, rh, x0, y0, buf, rw, rh);
				float* p32 = (float*)buf;
				for (int j = 0; j < rh; ++j) {
					for (int i = 0; i < rw; ++i) {
						int base = (i * bands) + (j * rw * bands);
						float r = (br ? p32[base + (br - 1)] : 0.0f);
						float g = (bg ? p32[base + (bg - 1)] : 0.0f);
						float b = (bb ? p32[base + (bb - 1)] : 0.0f);
						bool isValidR = std::isfinite(r) && (!hasNoDataR || r != (float)noDataR);
						bool isValidG = std::isfinite(g) && (!hasNoDataG || g != (float)noDataG);
						bool isValidB = std::isfinite(b) && (!hasNoDataB || b != (float)noDataB);
						if (isValidR) sampR.push_back(r);
						if (isValidG) sampG.push_back(g);
						if (isValidB) sampB.push_back(b);
					}
				}
				delete[] buf;
				// 控制样本规模
				if (sampR.size() > 200000) sampR.resize(200000);
				if (sampG.size() > 200000) sampG.resize(200000);
				if (sampB.size() > 200000) sampB.resize(200000);
			}
		}
		auto pctRange = [&](std::vector<float>& v, float lp, float hp, float& outMin, float& outMax){
			if (v.empty()) { outMin = 0.0f; outMax = 0.0f; return; }
			std::sort(v.begin(), v.end());
			size_t n = v.size();
			size_t loIdx = (size_t)std::floor((lp/100.0f) * (float)(n-1));
			size_t hiIdx = (size_t)std::floor((hp/100.0f) * (float)(n-1));
			outMin = v[loIdx]; outMax = v[hiIdx];
			// 如果范围过窄或无效，回退到样本整体范围
			if (!(outMax > outMin) || (outMax - outMin) < 1e-12f) {
				outMin = v.front(); outMax = v.back();
			}
		};
		pctRange(sampR, (float)lowPct, (float)highPct, minR32, maxR32);
		pctRange(sampG, (float)lowPct, (float)highPct, minG32, maxG32);
		pctRange(sampB, (float)lowPct, (float)highPct, minB32, maxB32);
	}

	// 调试输出：打印当前阈值，便于确认拉伸范围是否异常
    Console::write("(II) AutoStretch 8-bit: R[%d,%d] G[%d,%d] B[%d,%d]\n", minR8, maxR8, minG8, maxG8, minB8, maxB8);
    Console::write("(II) AutoStretch 16-bit: R[%d,%d] G[%d,%d] B[%d,%d]\n", minR16, maxR16, minG16, maxG16, minB16, maxB16);
    Console::write("(II) AutoStretch 32-bit: R[%f,%f] G[%f,%f] B[%f,%f]\n", minR32, maxR32, minG32, maxG32, minB32, maxB32);
}
