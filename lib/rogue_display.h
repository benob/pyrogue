#ifndef __TD_H__
#define __TD_H__

#include <stdint.h>
#include "rogue_array.h"
#include "rogue_random.h"
#include "rogue_keys.h"
#include <SDL.h>

#ifdef USE_SDLGPU
#include <SDL_gpu.h>
#endif

#include "stbttf.h"

typedef struct {
	int width, height;
	int tile_width, tile_height;
	int tiles_per_line;
#ifdef USE_SDLGPU
	GPU_Image* texture;
#else
	SDL_Surface* surface;
	SDL_Texture* texture;
#endif
} image_t;

typedef STBTTF_Font font_t;

// constants
enum {
  // event filter
  TD_ON_REDRAW = 1 << 1,
  TD_ON_KEY = 1 << 2,
  TD_ON_MOUSE = 1 << 3,
  TD_ON_EVENT = (TD_ON_REDRAW|TD_ON_KEY|TD_ON_MOUSE),

  // events
  TD_REDRAW = 1 << 4, 
  TD_KEY = 1 << 5,
  TD_MOUSE_MOVED = 1 << 6,
  TD_MOUSE_DOWN = 1 << 7,
  TD_MOUSE_UP = 1 << 8, 
  TD_QUIT = 1 << 9,

  //  button state
  TD_MOUSE_LEFT = 1 << 10,
  TD_MOUSE_MIDDLE = 1 << 11,
  TD_MOUSE_RIGHT = 1 << 12,

  // text alignment
  TD_ALIGN_LEFT = 1 << 13,
  TD_ALIGN_RIGHT = 1 << 14,
  TD_ALIGN_CENTER = 1 << 15,
  TD_ALIGN_TOP = 1 << 16,
  TD_ALIGN_BOTTOM = 1 << 17,
  TD_ALIGN_MIDDLE = 1 << 18,
};

#define INVALID_BITS(x, bits) (x & (~bits))
#define TD_FILTER_BITS (TD_ON_REDRAW|TD_ON_KEY|TD_ON_MOUSE)
#define TD_ALIGN_BITS (TD_ALIGN_LEFT|TD_ALIGN_RIGHT|TD_ALIGN_CENTER|TD_ALIGN_TOP|TD_ALIGN_BOTTOM|TD_ALIGN_MIDDLE)

#define td_color_r(color) ((color >> 0) & 255)
#define td_color_g(color) ((color >> 8) & 255)
#define td_color_b(color) ((color >> 16) & 255)
#define td_color_a(color) ((color >> 24) & 255)
#define td_color_rgba(r, g, b, a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)
#define td_color_rgb(r, g, b) td_color_rgba(r, g, b, 255)

// TODO: set images as render targets
int td_init_display(const char* title, int width, int height);
font_t* td_load_font(const char* font_path, float font_size);
void td_free_font(font_t* font);
image_t* td_load_image(const char* filename, int tile_width, int tile_height);
void td_free_image(image_t* image);
image_t* td_array_to_image(array_t* a, int tile_width, int tile_height, int palette_size, uint32_t* palette);
array_t* td_image_to_array(image_t* image);
void td_draw_image(image_t* image, int x, int y);
void td_draw_tile(image_t* image, int x, int y, int tile);
void td_colorize_tile(image_t* image, int x, int y, int tile, uint32_t fg, uint32_t bg);
void td_draw_array(array_t* a, int x, int y, int x_shift, int y_shift, image_t* tile_image, int* tile_map, int tile_map_size, uint32_t tile_mask, uint32_t* fg_palette, int fg_palette_size, uint32_t fg_mask, uint32_t* bg_palette, int bg_palette_size, uint32_t bg_mask);
// TODO: print utf8 characters
void td_draw_text(font_t* font, int orig_x, int orig_y, const char* text, uint32_t color, int align, int line_height);
void td_draw_text_from_tiles(image_t* image, int orig_x, int orig_y, const char* text, uint32_t color, int align);
void td_size_text(font_t* font, const char* text, int* width, int* height);
void td_fill_rect(int x, int y, int w, int h, uint32_t color);
void td_draw_rect(int x, int y, int w, int h, uint32_t color);
void td_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void td_clear();
void td_quit();
void td_run(void (*update_callback)(int key), int update_filter);
uint32_t td_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
uint32_t td_hex_color(const char* color);
uint32_t td_random_color();
uint32_t td_hsv_color(unsigned int h, unsigned char s, unsigned char v, unsigned char a);
int td_mouse_x();
int td_mouse_y();
int td_mouse_button();
int td_key();
void rl_force_redraw();
int td_shift_pressed();
int td_alt_pressed();
int td_ctrl_pressed();
int td_win_pressed();

#endif
