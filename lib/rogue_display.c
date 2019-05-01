#include <SDL.h>
#include <string.h>
#include <errno.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTTF_IMPLEMENTATION
#include "stbttf.h"

#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "rogue_util.h"
#include "rogue_display.h"
#include "rogue_filesystem.h"

#define TD_BACK_BUFFER TD_NUM_BUFFERS

typedef struct {
	int width, height;
	int tile_width, tile_height;
	int tiles_per_line;
	uint32_t* pixels;
	SDL_Texture* texture;
} image_t;

typedef struct {
	int width, height;
	int running, was_init;
	int mouse_x, mouse_y, mouse_button;
	SDL_Window* window;
	SDL_Renderer* renderer;
	STBTTF_Font* font;
	int line_height;
	image_t images[TD_NUM_IMAGES];
	SDL_Texture* buffers[TD_NUM_BUFFERS + 1];
	int use_backbuffer, is_fullscreen, is_maximized, use_integral_scale;
} display_t;

static display_t display;

static void __attribute__((constructor)) _td_init() {
	memset(&display, 0, sizeof(display_t));
}

// TODO: merge buffers and images
static void __attribute__((destructor)) _td_fini() {
	if(display.was_init) {
		for(int i = 0; i < TD_NUM_IMAGES; i++) {
			if(display.images[i].texture != NULL) SDL_DestroyTexture(display.images[i].texture);
			if(display.images[i].pixels != NULL) free(display.images[i].pixels);
		}
		for(int i = 0; i < TD_NUM_BUFFERS + 1; i++)
			if(display.buffers[i] != NULL) SDL_DestroyTexture(display.buffers[i]);

		if(display.font != NULL) {
			STBTTF_CloseFont(display.font);
		}
		SDL_Quit();
	}
}

static void (*rander_callback)(void*) = NULL;
static void* render_callback_data = NULL;

int td_init(const char* title, int width, int height) {
	if(!display.was_init) {
		if(SDL_Init(SDL_INIT_VIDEO) < 0) rl_error("count not initialize SDL");
		SDL_StartTextInput();
	}
	if(display.renderer != NULL) SDL_DestroyRenderer(display.renderer);
	if(display.window != NULL) SDL_DestroyWindow(display.window);
	display.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	if(display.window == NULL) rl_error("cannot create window");
	display.renderer = SDL_CreateRenderer(display.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if(display.renderer == NULL) rl_error("cannot create renderer");
	SDL_RenderSetLogicalSize(display.renderer, width, height);
	SDL_SetRenderDrawBlendMode(display.renderer, SDL_BLENDMODE_BLEND);

	display.width = width;
	display.height = height;
	display.running = 1;
	display.was_init = 1;
	td_use_backbuffer(1);
	//td_set_integral_scale(1);
	return 1;
}

void td_load_font(const char* font_path, int font_size, int line_height) {
	uint32_t font_data_size;
	char* font_data = fs_load_asset(font_path, &font_data_size);
	if(font_data == NULL) rl_error("[Errno %d] %s: '%s'", errno, strerror(errno), font_path);
	SDL_RWops* ops = SDL_RWFromMem(font_data, font_data_size);
	if(display.font != NULL) STBTTF_CloseFont(display.font);
	display.font = STBTTF_OpenFontRW(display.renderer, ops, font_size);
	if(display.font) display.line_height = display.font->baseline;
	free(font_data);
	if(display.font == NULL) rl_error("cannot load font '%s'", font_path);
	if(line_height != 0) display.line_height = line_height;
}

void td_set_integral_scale(int value) {
	SDL_RenderSetIntegerScale(display.renderer, value ? SDL_TRUE : SDL_FALSE);
}

int td_load_image(int index, const char* filename, int tile_width, int tile_height) {
	uint32_t image_data_size;
	char* image_data = fs_load_asset(filename, &image_data_size);
	if(image_data == NULL) rl_error("cannot load image '%s' from assets", filename);
	SDL_RWops* ops = SDL_RWFromMem(image_data, image_data_size);
	SDL_Surface* surface = STBIMG_Load_RW(ops, 1);
	free(image_data);
	if(surface == NULL) rl_error("cannot decode image data '%s'", filename);
	image_t image;
	image.tile_width = tile_width;
	image.tile_height = tile_height;
	image.width = surface->w;
	image.height = surface->h;
	image.tiles_per_line = image.width / image.tile_width;
	image.texture = SDL_CreateTextureFromSurface(display.renderer, surface);
	image.pixels = malloc(sizeof(uint32_t) * surface->w * surface->h);
	memcpy(image.pixels, surface->pixels, sizeof(uint32_t) * surface->w * surface->h);
	if(image.texture == NULL) rl_error("cannot create texture for image '%s'", filename);
	SDL_FreeSurface(surface);
	if(display.images[index].texture != NULL) {
		SDL_DestroyTexture(display.images[index].texture);
		free(display.images[index].pixels);
	}
	display.images[index] = image;
	return 1;
}

void td_array_to_image(int index, array_t* a, int tile_width, int tile_height) {
	if(index < 0 || index >= TD_NUM_IMAGES) rl_error("invalid image '%d'", index);
	image_t* image = &display.images[index];
	image->tile_width = tile_width;
	image->tile_height = tile_height;
	image->width = a->width;
	image->height = a->height;
	image->tiles_per_line = image->width / image->tile_width;

	if(image->texture == NULL || a->width != image->width || a->height != image->height) {
		if(image->texture != NULL) SDL_DestroyTexture(image->texture);
		image->texture = SDL_CreateTexture(display.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, a->width, a->height);
	}
	if(image->texture == NULL) rl_error("cannot create texture");
	SDL_UpdateTexture(image->texture, NULL, a->values, sizeof(uint32_t) * (a->width + a->stride));

	if(image->pixels != NULL) free(image->pixels);
	image->pixels = malloc(sizeof(uint32_t) * a->width * a->height);
	if(a->is_view) {
		for(int i = 0; i < a->height; i++)
			memcpy(image->pixels + i * a->width, a->values + i * (a->width + a->stride), sizeof(uint32_t) * a->width);
	} else {
		memcpy(image->pixels, a->values, sizeof(uint32_t) * a->width * a->height);
	}
}

array_t* td_image_to_array(int index) {
	if(index < 0 || index >= TD_NUM_IMAGES) rl_error("invalid image '%d'", index);
	image_t* image = &display.images[index];
	if(image->pixels == NULL) rl_error("invalid image '%d'", index);
	array_t* a = rl_array_new(image->width, image->height);
	memcpy(a->values, image->pixels, sizeof(uint32_t) * a->width * a->height);
	return a;
}

void td_draw_image(int index, int x, int y) {
	if(index < 0 || index >= TD_NUM_IMAGES) rl_error("invalid image '%d'", index);
	image_t* image = &display.images[index];
	if(image->texture == NULL) rl_error("invalid image '%d'", index);
	SDL_Rect rect = {x, y, image->width, image->height};
	SDL_RenderCopy(display.renderer, image->texture, NULL, &rect);
}

void td_draw_tile(int index, int x, int y, int tile) {
	if(index < 0 || index >= TD_NUM_IMAGES) rl_error("invalid image '%d'", index);
	image_t* image = &display.images[index];
	if(image->texture == NULL) rl_error("invalid image '%d'", index);
	int tile_x = (tile % image->tiles_per_line) * image->tile_width;
	int tile_y = (tile / image->tiles_per_line) * image->tile_height;
	SDL_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
	SDL_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
	SDL_RenderCopy(display.renderer, image->texture, &src_rect, &dst_rect);
}

void td_colorize_tile(int index, int x, int y, int tile, uint32_t fg, uint32_t bg) {
	if(index < 0 || index >= TD_NUM_IMAGES) rl_error("invalid image '%d'", index);
	image_t* image = &display.images[index];
	if(image->texture == NULL) rl_error("invalid image '%d'", index);
	int tile_x = (tile % image->tiles_per_line) * image->tile_width;
	int tile_y = (tile / image->tiles_per_line) * image->tile_height;
	SDL_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
	SDL_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
	if(bg != 0) {
		SDL_SetRenderDrawColor(display.renderer, td_color_r(bg), td_color_g(bg), td_color_b(bg), td_color_a(bg));
		SDL_RenderFillRect(display.renderer, &dst_rect);
	}
	if(fg != 0) {
		SDL_SetTextureColorMod(image->texture, td_color_r(fg), td_color_g(fg), td_color_b(fg));
		SDL_SetTextureAlphaMod(image->texture, td_color_a(fg));
	}
	SDL_RenderCopy(display.renderer, image->texture, &src_rect, &dst_rect);
	SDL_SetTextureColorMod(image->texture, 255, 255, 255);
	SDL_SetTextureAlphaMod(image->texture, 255);
}

void td_draw_array(int index, array_t* a, int x, int y, int x_shift, int y_shift, int info_size, int* info_mapping, uint32_t* info_fg, uint32_t* info_bg) {
	if(index < 0 || index >= TD_NUM_IMAGES) rl_error("invalid image '%d'", index);
	image_t* image = &display.images[index];
	if(image->texture == NULL) rl_error("invalid image '%d'", index);
	if(x_shift == 0) x_shift = image->tile_width;
	if(y_shift == 0) y_shift = image->tile_height;
	/* split background and foreground drawing to prevent slow switch of gl primitive type */
	for(int j = 0; j < a->height; j++) {
		for(int i = 0; i < a->width; i++) {
			int num = rl_array_get(a, i, j);
			if(num < 0) continue;
			if(info_bg != NULL && num >= info_size) continue;
			uint32_t bg = info_bg ? info_bg[num] : 0;
			if(bg != 0) {
				SDL_Rect dst_rect = {x + x_shift * i, y + y_shift * j, image->tile_width, image->tile_height};
				SDL_SetRenderDrawColor(display.renderer, td_color_r(bg), td_color_g(bg), td_color_b(bg), td_color_a(bg));
				SDL_RenderFillRect(display.renderer, &dst_rect);
			}
		}
	}
	for(int j = 0; j < a->height; j++) {
		for(int i = 0; i < a->width; i++) {
			int num = rl_array_get(a, i, j);
			if(num < 0) continue;
			if((info_mapping != NULL || info_fg != NULL) && num >= info_size) continue;
			int tile = info_mapping != NULL ? info_mapping[num] : num;
			int tile_x = (tile % image->tiles_per_line) * image->tile_width;
			int tile_y = (tile / image->tiles_per_line) * image->tile_height;
			uint32_t fg = info_fg ? info_fg[num] : 0;
			SDL_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
			SDL_Rect dst_rect = {x + x_shift * i, y + y_shift * j, image->tile_width, image->tile_height};
			if(fg != 0) {
				SDL_SetTextureColorMod(image->texture, td_color_r(fg), td_color_g(fg), td_color_b(fg));
				SDL_SetTextureAlphaMod(image->texture, td_color_a(fg));
			}
			SDL_RenderCopy(display.renderer, image->texture, &src_rect, &dst_rect);
		}
	}
	SDL_SetTextureColorMod(image->texture, 255, 255, 255);
	SDL_SetTextureAlphaMod(image->texture, 255);
}


void td_use_backbuffer(int value) {
	display.use_backbuffer = value;
	if(display.buffers[TD_BACK_BUFFER] != NULL) SDL_DestroyTexture(display.buffers[TD_BACK_BUFFER]);
	if(value) {
		display.buffers[TD_BACK_BUFFER] = SDL_CreateTexture(display.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, display.width, display.height);
	} else {
		display.buffers[TD_BACK_BUFFER] = NULL;
	}
	SDL_SetRenderTarget(display.renderer, display.buffers[TD_BACK_BUFFER]);
}

void td_set_buffer(int index) {
	if(index < 0 || index >= TD_NUM_BUFFERS) {
		SDL_SetRenderTarget(display.renderer, display.buffers[TD_BACK_BUFFER]);
	} else {
		if(display.buffers[index] == NULL)
      display.buffers[index] = SDL_CreateTexture(display.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, display.width, display.height);

		SDL_SetRenderTarget(display.renderer, display.buffers[index]);
	}
}

void td_blit_buffer(int index) {
	SDL_RenderCopy(display.renderer, display.buffers[index], NULL, NULL);
}

void td_print_text(int orig_x, int orig_y, const char* text, uint32_t color, int align) {
	if(!display.font) rl_error("no font loaded");
	int x = orig_x, y = orig_y + display.font->baseline;
	SDL_Color fg = {td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color)};
	while(text && *text != '\0') {
		if(*text == '\n') {
			x = orig_x;
			y += display.line_height;
			text++;
		} else {
			char* eol = strchr(text, '\n');
			int length = 0;
			if(eol == NULL) length = strlen(text);
			else length = eol - text;
			if(length > 0) { 
				char line[length + 1];
				strncpy(line, text, length);
				line[length] = '\0';
				SDL_SetRenderDrawColor(display.renderer, fg.r, fg.g, fg.b, fg.a);
				int width = STBTTF_MeasureText(display.font, line);
				if(align == TD_ALIGN_CENTER) x -= width / 2;
				else if(align == TD_ALIGN_RIGHT) x -= width;
				STBTTF_RenderText(display.renderer, display.font, x, y, line);
			}
			text += length;
		}
	}
}

void td_size_text(const char* text, int* width, int* height) {
	if(!display.font) rl_error("no font loaded");
	*width = STBTTF_MeasureText(display.font, text);
	*height = display.line_height;
};

void td_fill_rect(int x, int y, int w, int h, uint32_t color) {
	SDL_Rect rect = {x, y, w, h};
	SDL_SetRenderDrawColor(display.renderer, td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color));
	SDL_RenderFillRect(display.renderer, &rect);
}

void td_draw_rect(int x, int y, int w, int h, uint32_t color) {
	SDL_Rect rect = {x, y, w, h};
	SDL_SetRenderDrawColor(display.renderer, td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color));
	SDL_RenderDrawRect(display.renderer, &rect);
}

int td_still_running() {
	return display.running;
}

int td_wait_key() {
	int key = td_wait_event(0);
	while(display.running && key == TD_REDRAW) {
		key = td_wait_event(0);
	}
	return key;
}
 
int td_wait_event(int include_mouse) {
	int key;
	SDL_Event event;
	while(display.running) {
		while(SDL_WaitEvent(&event) != 0) {
			switch(event.type) {
				case SDL_QUIT:
					display.running = 0;
					return TD_QUIT;
					break;
				case SDL_WINDOWEVENT:
					td_present();
					break;
        case SDL_TEXTINPUT:
					return event.text.text[0];
					break;
				case SDL_KEYDOWN:
					key = event.key.keysym.sym;
					if(SDL_GetModState() & KMOD_ALT) {
						if(key == SDLK_RETURN) {
							if(display.is_fullscreen) {
								SDL_SetWindowFullscreen(display.window, 0);
								display.is_fullscreen = 0;
							}
							else {
								SDL_SetWindowFullscreen(display.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
								display.is_fullscreen = 1;
							}
						} else if(key == SDLK_q) {
							// TODO: use customizable exit handler
							exit(1); // force quit
						} else if(key == SDLK_i) {
							// TODO: investgate why it doesn't update immediately.
							display.use_integral_scale = 1 - display.use_integral_scale;
							SDL_RenderSetIntegerScale(display.renderer, display.use_integral_scale ? SDL_TRUE : SDL_FALSE);
						}
						return TD_REDRAW;
					}
					else if(key >= 32 && key < 127) continue;
					else if(key != SDLK_LALT && key != SDLK_LSHIFT && key != SDLK_LCTRL && key != SDLK_RALT && key != SDLK_RSHIFT && key != SDLK_RCTRL && key != SDLK_LGUI && key != SDLK_RGUI) return key;
					break;
				case SDL_MOUSEMOTION:
					display.mouse_x = event.motion.x;
					display.mouse_y = event.motion.y;
					if(include_mouse)
						return TD_MOUSE;
					break;
				case SDL_MOUSEBUTTONUP:
					display.mouse_button = 0;
					if(include_mouse)
						return TD_MOUSE;
					break;
				case SDL_MOUSEBUTTONDOWN:
					display.mouse_button = event.button.button;
					if(include_mouse)
						return TD_MOUSE;
					break;
			}
		}
	}
	return TD_QUIT;
}

int td_poll_event() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			display.running = 0;
			return 0;
		}
	}
	return 1;
}

void td_present() {
	if(display.use_backbuffer) {
		SDL_SetRenderTarget(display.renderer, NULL);
		td_clear();
		SDL_RenderCopy(display.renderer, display.buffers[TD_BACK_BUFFER], NULL, NULL);
		SDL_RenderPresent(display.renderer);
		SDL_Delay(1000 / 60);
		SDL_SetRenderTarget(display.renderer, display.buffers[TD_BACK_BUFFER]);
	} else {
		SDL_RenderPresent(display.renderer);
		SDL_Delay(1000 / 60);
	}
}

void td_clear() {
  SDL_SetRenderDrawColor(display.renderer, 0, 0, 0, 255);
  SDL_RenderClear(display.renderer);
}

void td_quit() {
#ifdef __EMSCRIPTEN__
	emscripten_cancel_main_loop();
#else
	display.running = 0;
#endif
}

static void process_events(void* arg) {
	void (*callback)(int key) = (void (*)(int)) arg;
	int key = TD_PASS;
	SDL_Event event;
	while(SDL_PollEvent(&event) != 0) {
		switch(event.type) {
			case SDL_QUIT:
				td_quit();
				key = TD_QUIT;
				break;
			case SDL_WINDOWEVENT:
				key = TD_REDRAW;
				break;
			case SDL_TEXTINPUT:
				key = event.text.text[0];
				break;
			case SDL_KEYDOWN:
				key = event.key.keysym.sym;
				if(SDL_GetModState() & KMOD_ALT) {
					if(key == SDLK_RETURN) {
						if(display.is_fullscreen) {
							SDL_SetWindowFullscreen(display.window, 0);
							display.is_fullscreen = 0;
						} else {
							SDL_SetWindowFullscreen(display.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
							display.is_fullscreen = 1;
						}
					} else if(key == SDLK_q) {
						// TODO: use customizable exit handler
						exit(1); // force quit
					}
					key = TD_REDRAW;
				}
				else if(key >= 32 && key < 127) continue;
				else if(key != SDLK_LALT && key != SDLK_LSHIFT && key != SDLK_LCTRL && key != SDLK_RALT && key != SDLK_RSHIFT && key != SDLK_RCTRL && key != SDLK_LGUI && key != SDLK_RGUI) { }
				break;
		}
	}
	printf("%d\n", key);
	if(key != TD_PASS) callback(key);
}

void td_run(void (*update_callback)(int key)) {
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(process_events, update_callback, 0, 1);
#else
	int key = TD_REDRAW;
	while(display.running) {
		process_events(update_callback);
		td_present();
	}
#endif
}

void td_delay(uint32_t ms) {
	SDL_Delay(ms);
}

char* td_get_dir(const char* organization, const char* application) {
	return SDL_GetPrefPath(organization, application);
}

uint32_t td_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return td_color_rgba(r, g, b, a);
}

uint32_t td_hsv_color(unsigned int h, unsigned char s, unsigned char v, unsigned char a) {
	unsigned char r, g, b;
	unsigned char region, remainder, p, q, t;

	if (s == 0) return td_color_rgba(v, v, v, a);

	region = h / 43;
	remainder = (h - (region * 43)) * 6; 

	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * remainder) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		default: r = v; g = p; b = q; break;
	}
	return td_color_rgba(r, g, b, a);
}

int td_mouse_x() {
	return display.mouse_x;
}

int td_mouse_y() {
	return display.mouse_y;
}

int td_mouse_button() {
	return display.mouse_button;
}

void td_draw_points(td_point_t* points, int num, uint32_t color) {
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, display.width, display.height, 32, SDL_PIXELFORMAT_RGBA32);
	for(int i = 0; i < num; i++)
		((uint32_t*)surface->pixels)[points[i].x + points[i].y * surface->w] = color;
	SDL_Texture* texture = SDL_CreateTextureFromSurface(display.renderer, surface);
	SDL_RenderCopy(display.renderer, texture, NULL, NULL);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}
