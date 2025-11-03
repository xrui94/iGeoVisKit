#ifndef _IMAGE_TILE_SET_H
#define _IMAGE_TILE_SET_H

#include "ImageFile.h"

#include <deque>

typedef struct tile_t {
	int tile_index;
	char* data;
	unsigned int age;
} tile, *tile_ptr;

class ImageTileSet
{
public:
	/* Class control */
	ImageTileSet(int level_of_detail, ImageFile* file, int tex_size_param, int cache_size);
	virtual ~ImageTileSet();
	
	/* Tile Data */
	char* get_tile_RGB(int x, int y, int band_R, int band_G, int band_B);
	char* get_tile_RGB_LOD(int LOD_x, int LOD_y, int band_R, int band_G, int band_B);
	unsigned char* get_pixel_values(int x, int y);
	unsigned char* get_pixel_values_LOD(int x, int y);
	
	int get_LOD_factor() const {return LOD_factor;}
	int get_tile_image_size() const {return tile_size;}
	
	int get_last_column_width() const {return last_column_width;}
	int get_last_row_height() const {return last_row_height;}
	
	int get_texture_size() const { return tex_size; }
	int get_columns() const { return columns; }
	int get_rows() const { return rows; }
	int get_tile_size() const { return tile_size; }
	int get_LOD() const { return LOD; }
	int get_LOD_width() const { return LOD_width; }
	int get_LOD_height() const { return LOD_height; }
	int get_sample_size() const { return sample_size; }
	
	void get_cache_stats(unsigned int* size_p,
						 unsigned int* fill_p,
						 unsigned int* hits_p,
						 unsigned int* misses_p
	) const;
	
private:
	/* Helper functions */
	int load_tile(int image_x, int image_y);
	void align_tile(char** tile, int tile_dimension,
					int data_width, int data_height);
	
	/* Tileset properties */
	int LOD;
	int LOD_factor;
	ImageFile* image_file;
	
	/* Cache */
	int cache_size;
	int cache_fill;
	int cache_hits;
	int cache_misses;
	
	/* Data properties */
	int image_width, image_height, num_bands, sample_size;
	int LOD_width, LOD_height;
	int columns, rows, tile_size;
	int last_column_width, last_row_height;
	int tex_size, tile_memory_size;

	/* This stores the actual tile information */
	std::deque<tile_t*> tiles;
};

#endif
