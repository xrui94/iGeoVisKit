#ifndef _IMAGE_OVERVIEW_H
#define _IMAGE_OVERVIEW_H

#include "imagery/ImageFile.h"
#include "imagery/ImageTileSet.h"
#include "opengl/ImageViewport.h"

class OverviewGL : public ViewportListener
{
  public:
    OverviewGL(ImageFile* image_file, ImageViewport* image_viewport_param);
    virtual ~OverviewGL(void);

    void notify_viewport(void);
    void notify_bands(void);

	void set_brightness_contrast(float brightnes_arg, float contrast_arg);

  public:
    void make_texture(void);
    void ensureGLResources();
    void render_scene();

    ImageViewport* viewport;

    /* General OpenGL stuff */
    GLfloat scalefactor_tile, scalefactor_lines;
    unsigned int list_tile; // display list for textured tile

    int image_width, image_height;
    int band_red, band_green, band_blue;

    	/* Overview window texture */
    ImageTileSet* tileset;
    GLint texture_size;
    int LOD_height, LOD_width;
    GLuint tex_overview_id;
    bool glInitialized;

    /* Programmable pipeline resources */
    GLuint progTex;
    GLuint progColor;
    GLuint vboTile;
    GLuint vboLines;
    GLuint vaoTile;
    GLuint vaoLines;
    // attributes and uniforms for texture program
    GLint locTex_aPos;
    GLint locTex_aUV;
    GLint locTex_uProj;
    GLint locTex_uTex;
    // attributes and uniforms for color program
    GLint locColor_aPos;
    GLint locColor_uProj;
    GLint locColor_uColor;
};

#endif