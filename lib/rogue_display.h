#ifndef __TD_H__
#define __TD_H__

#include <stdint.h>
#include "rogue_array.h"
#include "rogue_keys.h"

// text alignment
enum {TD_ALIGN_LEFT, TD_ALIGN_RIGHT, TD_ALIGN_CENTER};
// events
enum {TD_QUIT = -3, TD_MOUSE = -2, TD_REDRAW = -1, TD_PASS = 0};
// event types
enum {TD_UPDATE_KEY=1, TD_UPDATE_MOUSE=2, TD_UPDATE_LOOP=4};

typedef struct {
	int x, y;
} td_point_t;

#define TD_NUM_IMAGES 32
#define TD_NUM_BUFFERS 4

#define td_color_r(color) ((color >> 24) & 255)
#define td_color_g(color) ((color >> 16) & 255)
#define td_color_b(color) ((color >> 8) & 255)
#define td_color_a(color) ((color >> 0) & 255)
#define td_color_rgba(r, g, b, a) ((r) << 24 | (g) << 16 | (b) << 8 | (a))
#define td_color_rgb(r, g, b) td_color_rgba(r, g, b, 255)

// TODO: set images as render targets
int td_init(const char* title, int width, int height);
void td_load_font(const char* font_path, int font_size, int line_height);
// TODO: associate to keypress and internalize
void td_set_integral_scale(int value);
void td_use_backbuffer(int value);
int td_load_image(int image, const char* filename, int tile_width, int tile_height);
void td_array_to_image(int index, array_t* a, int tile_width, int tile_height);
array_t* td_image_to_array(int index);
void td_draw_image(int image, int x, int y);
void td_draw_tile(int image, int x, int y, int tile);
void td_colorize_tile(int image, int x, int y, int tile, uint32_t fg, uint32_t bg);
void td_draw_array(int index, array_t* a, int x, int y, int x_shift, int y_shift, int info_size, int* info_mapping, uint32_t* info_fg, uint32_t* info_bg);
void td_set_buffer(int buffer);
// print utf8 characters
void td_print_text(int orig_x, int orig_y, const char* text, uint32_t color, int align);
void td_print_text_from_tiles(int index, int orig_x, int orig_y, const char* text, uint32_t color, int align);
void td_size_text(const char* text, int* width, int* height);
void td_fill_rect(int x, int y, int w, int h, uint32_t color);
void td_draw_rect(int x, int y, int w, int h, uint32_t color);
void td_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void td_draw_points(td_point_t* points, int num, uint32_t color);
int td_still_running();
int td_wait_key();
int td_wait_event(int include_mouse);
int td_poll_event();
void td_present();
void td_delay(uint32_t ms);
void td_clear();
void td_quit();
void td_run(void (*update_callback)(int key), int update_filter);
uint32_t td_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
uint32_t td_hsv_color(unsigned int h, unsigned char s, unsigned char v, unsigned char a);
int td_mouse_x();
int td_mouse_y();
int td_mouse_button();

#endif
