#ifndef _IMAGE_HANDLER_H
#define _IMAGE_HANDLER_H

// #include <QWidget>
#include <memory>
#include "ImageFile.h"
#include "ImageProperties.h"
#include "ROISet.h"
#include "opengl/OverviewGL.h"
#include "opengl/ImageGL.h"
#include "opengl/ImageViewport.h"

class Renderer;

class ImageHandler
{
public:
    /* Class Operators */
    ImageHandler(const char* filename, ROISet *ROI_set, std::shared_ptr<Renderer> renderer);
	virtual ~ImageHandler(void);
	
	/* Data Operators */
	ImageProperties* get_image_properties(void);
	BandInfo* get_band_info(int band_number);
	string get_info_string(void);
	
	/* Window Operators */
	void redraw(void);

	//
	ImageGL* getImageGL() const { return image_gl; }
	OverviewGL* getOverviewGL() const { return overview_gl; }

	//void resize_image_window(void);
	ImageViewport* get_image_viewport(void) {return image_viewport;}
	ImageFile* get_image_file(void) {return image_file;}
	
	// get pixel values from window co-ordinates
	unsigned char* get_window_pixel_values(int x, int y); // remember to delete[]
	// get pixel values from absolute coordinates at this zoom level
	unsigned char* get_zoom_pixel_values(int x, int y); // remember to delete[]
    // get pixel values from absolute image coordinates (from displayed LOD)
    unsigned char* get_image_pixel_values(int x, int y); // remember to delete[]

	/* Contrast/brigtness */
	// Valid parameters are between 1 and 500; default (normal) = 250
	void set_brightness_contrast(int new_brightness, int new_contrast);
	void get_brightness_contrast(int* brightness_return, int* contrast_return);
	void reset_brightness_contrast(void);

	// 拉伸模式设置：转发到 ImageGL
	void set_stretch_mode(ImageGL::StretchMode m);
	
	/* Set Current Mouse Position In Image Co-Ords (For ROI Drawing) */
	void set_mouse_position(int ix,int iy) {image_gl->set_mouse_position(ix,iy);};

private:
	/* Sub-objects */
	ImageFile* image_file;
	ImageProperties* image_properties;
	ImageViewport* image_viewport;
	ImageGL* image_gl;
	OverviewGL* overview_gl;
	
	// Current state of brightness/contrast values
	int contrast_value;
	int brightness_value;
};

#endif

