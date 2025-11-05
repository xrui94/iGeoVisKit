#include "PchApp.h"

#include "ImageHandler.h"

// Enable this to include debugging
#define DEBUG_IMAGE_HANDLER 1

#if DEBUG_IMAGE_HANDLER
#include "utils/Console.h"
#endif

ImageHandler::ImageHandler(const char* filename, ROISet *ROI_set, std::shared_ptr<Renderer> renderer)
{
#if DEBUG_IMAGE_HANDLER
	Console::write("(II) Initializing image handler(%p, %p, %s)\n", /*(void*)overview_widget, (void*)image_widget, */filename);
#endif
	
	// Initialize class variables
	image_properties = NULL;			
	image_viewport = NULL;
	overview_gl = NULL;
	image_gl = NULL;
	brightness_value = 250;
	contrast_value = 250;
	
	// Check for lazily unspecified (NULL argument) parameters
	//assert(overview_widget != NULL);
	//assert(image_widget != NULL);
	assert(filename != NULL);

	// Initialize image file (could be threaded)
	image_file = new ImageFile(filename);
	assert(image_file != NULL);
	assert(image_file->getImageProperties() != NULL);
	// !! If image_file worked...
	if (image_file->getImageProperties() != NULL)
	{
		image_properties = image_file->getImageProperties();
		assert(image_properties != NULL);
		
		image_viewport = new ImageViewport(image_properties);
		assert(image_viewport != NULL);

		/* Initialize overview GL (decoupled from Qt; GLView set up in OverviewWindow) */
		overview_gl = new OverviewGL(image_file, image_viewport);
		assert(overview_gl != NULL);

		image_gl = new ImageGL(image_file, image_viewport, ROI_set, renderer);
		assert(image_gl != NULL);
		
		image_viewport->set_display_bands(1,2,3);

		// 设置合理的初始缩放级别，避免0.0缩放导致的黑屏
		// 计算适合窗口的缩放级别，默认设置为1.0（100%）
		float initial_zoom = 1.0f;
		image_viewport->set_zoom_level(initial_zoom);
		
		// 调试输出：确认缩放级别设置
		printf("DEBUG: Initial zoom level set to: %f\n", image_viewport->get_zoom_level());

		// 初始时主动设置窗口尺寸并触发一次视口通知，确保纹理及时加载
		//image_gl->resize_window();
		//image_viewport->notify_viewport_listeners();
	} else {
		delete image_file;
		image_file = NULL;
	}
}

ImageHandler::~ImageHandler(void)
{
	#if DEBUG_IMAGE_HANDLER
	Console::write("(II) ImageHandler shutting  down...\n");
	#endif
	delete image_viewport;
	delete overview_gl;
	delete image_gl;
	delete image_file;
	#if DEBUG_IMAGE_HANDLER
	Console::write("(II) ImageHandler shutdown complete.\n");
	#endif
}

void ImageHandler::redraw(void)
{
	image_viewport->notify_viewport_listeners();
}

// Return the current ImageProperties object
// Should be NULL for invalid image
ImageProperties* ImageHandler::get_image_properties(void)
{
	return image_properties;
}

BandInfo* ImageHandler::get_band_info(int band_number)
{
	return image_file->getBandInfo(band_number);
}

//void ImageHandler::resize_image_window(void)
//{
//    image_gl->resize_window();
//}

/* This function gets pixel values in absolute image coordinates */
unsigned char* ImageHandler::get_image_pixel_values(int x, int y)
{
	return image_gl->get_pixel_values(x,y);
}
/* This function gets pixel values from absolute coordinates at zoom scaling*/
unsigned char* ImageHandler::get_zoom_pixel_values(int x, int y)
{
	float zoom_factor = image_viewport->get_zoom_level() / 100.0;
	/* Translate zoom coordinates to image coordinates */
	x = int(round(zoom_factor * float(x)));
	y = int(round(zoom_factor * float(y)));
		
	/* Resume query */
	return get_image_pixel_values(x,y);
}


/* This function gets pixel values from the current viewport */
unsigned char* ImageHandler::get_window_pixel_values(int x, int y)
{
	int ix, iy;
	image_viewport->translate_window_to_image(x, y, &ix, &iy);
	return image_gl->get_pixel_values(ix,iy);
}

string ImageHandler::get_info_string(void)
{
    return image_file->getInfoString();
}    

// Should be given parameters between 1 and 500 (inclusive)
void ImageHandler::set_brightness_contrast(int new_brightness, int new_contrast)
{
	Console::write("ImageHandler::set_contrast_brightness(%d, %d)\n", new_brightness, new_contrast);

	float brightness_param, contrast_param;
	brightness_value = new_brightness;
	contrast_value = new_contrast;
	
	brightness_param = (float(brightness_value) / 250.0) - 1.0;
	contrast_param = float(contrast_value) / 250.0;
	
	image_gl->set_brightness_contrast(brightness_param, contrast_param);
	overview_gl->set_brightness_contrast(brightness_param, contrast_param);
}
void ImageHandler::get_brightness_contrast(int* brightness_return, int* contrast_return)
{
	if (brightness_return != NULL) *brightness_return = brightness_value;
	if (contrast_return != NULL) *contrast_return = contrast_value;
}

void ImageHandler::reset_brightness_contrast(void)
{
	set_brightness_contrast(250,250);
}

// 设置影像拉伸模式并请求刷新
void ImageHandler::set_stretch_mode(ImageGL::StretchMode m)
{
	if (image_gl) {
		image_gl->setStretchMode(m);
	}
}


