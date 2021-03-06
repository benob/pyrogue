#include <string.h>
#include <errno.h>
#include <SDL.h>

#ifdef USE_SDLGPU
#include <SDL_gpu.h>
#endif

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTTF_IMPLEMENTATION
#include "stbttf.h"

#ifndef USE_SDLGPU
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_PNM 
#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "rogue_util.h"
#include "rogue_display.h"
#include "rogue_filesystem.h"

#define TD_BACK_BUFFER TD_NUM_BUFFERS

typedef struct {
	int width, height;
	int window_width, window_height;
	int running, was_init;
  int key;
	int mouse_x, mouse_y, mouse_button;
	SDL_Window* window;
	SDL_Renderer* renderer;
#ifdef USE_SDLGPU
	GPU_Target* screen;
	GPU_Image* screen_image;
	GPU_Target* actual_screen;
	GPU_Rect scaled_rect;
#else
	SDL_Texture* screen;
	SDL_Rect scaled_rect;
#endif
	double device_pixel_ratio;
	int is_fullscreen, is_maximized, use_integral_scale;
	int update_filter;
	void (*update_callback)(int);
} display_t;

static display_t display;

static void __attribute__((constructor)) _td_init() {
	memset(&display, 0, sizeof(display_t));
	td_init_display("pyrogue", 320, 240);
}

static void __attribute__((destructor)) _td_fini() {
	if(display.was_init) {
#ifndef USE_SDLGPU
		SDL_DestroyTexture(display.screen);
#endif

#ifdef USE_SDLGPU
		GPU_Quit();
#else
		SDL_Quit();
#endif
	}
}

static void (*rander_callback)(void*) = NULL;
static void* render_callback_data = NULL;
void td_present();

static void null_main_loop() {
}

int td_init_display(const char* title, int width, int height) {
	if(!display.was_init) {
		display.window_width = width;
		display.window_height = height;
		display.device_pixel_ratio = 1;
#ifdef USE_SDLGPU
		//GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
		display.actual_screen = GPU_Init(width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
#else
		if(SDL_Init(SDL_INIT_VIDEO) < 0) rl_error("could not initialize SDL");
		display.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
		if(display.window == NULL) rl_error("cannot create window");
		display.renderer = SDL_CreateRenderer(display.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);
		if(display.renderer == NULL) rl_error("cannot create renderer");
		SDL_SetRenderDrawBlendMode(display.renderer, SDL_BLENDMODE_BLEND);
#endif
		SDL_StartTextInput();
	}
#ifdef __EMSCRIPTEN__
	// let canvas handle keyboard events instead of window
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
	EM_ASM(
			// make sure canvas uses nearest neighbor smoothing
      document.getElementById('canvas').style.imageRendering = 'pixelated';
  );
#endif
#ifdef USE_SDLGPU
	//TODO: if(display.screen_image != NULL)
	display.screen_image = GPU_CreateImage(width, height, GPU_FORMAT_RGBA);
	GPU_SetImageFilter(display.screen_image, GPU_FILTER_NEAREST);
	//GPU_SetSnapMode(display.screen_image, GPU_SNAP_POSITION_AND_DIMENSIONS);
	GPU_LoadTarget(display.screen_image);
	display.screen = display.screen_image->target;
	display.window = SDL_GL_GetCurrentWindow();
#else
	if(display.screen != NULL && (display.width != width || display.height != height))
		SDL_DestroyTexture(display.screen);
	if(display.screen == NULL || display.width != width || display.height != height)
		display.screen = SDL_CreateTexture(display.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
	
	SDL_SetRenderTarget(display.renderer, display.screen);
#endif
	SDL_SetWindowTitle(display.window, title);

	display.width = width;
	display.height = height;
	display.was_init = 1;
	td_clear();
	td_present();
	return 1;
}

font_t* td_load_font(const char* font_path, float font_size) {
	uint32_t font_data_size;
	char* font_data = fs_load_asset(font_path, &font_data_size);
	if(font_data == NULL) rl_error("[Errno %d] %s: '%s'", errno, strerror(errno), font_path);
	SDL_RWops* ops = SDL_RWFromMem(font_data, font_data_size);
#ifdef USE_SDLGPU
	font_t* font = STBTTF_OpenFontRW(ops, font_size);
#else
	font_t* font = STBTTF_OpenFontRW(display.renderer, ops, font_size);
#endif
	free(font_data);
	if(font == NULL) rl_error("cannot load font '%s'", font_path);
	return font;
}

void td_free_font(font_t* font) {
	STBTTF_CloseFont(font);
}

image_t* td_load_image(const char* filename, int tile_width, int tile_height) {
	uint32_t image_data_size;
	char* image_data = fs_load_asset(filename, &image_data_size);
	if(image_data == NULL) rl_error("cannot load image '%s' from assets", filename);
	SDL_RWops* ops = SDL_RWFromMem(image_data, image_data_size);
	image_t* image = rl_malloc(sizeof(image_t));
	image->tile_width = tile_width;
	image->tile_height = tile_height;
#ifdef USE_SDLGPU
	image->texture = GPU_LoadImage_RW(ops, 1);
	if(image->texture == NULL) {
		rl_free(image, sizeof(image_t));
		free(image_data);
		rl_error("cannot decode image data '%s'", filename);
	}
	GPU_SetImageFilter(image->texture, GPU_FILTER_NEAREST);
	GPU_SetAnchor(image->texture, 0, 0);
	GPU_SetBlending(image->texture, 1);
	GPU_SetBlendMode(image->texture, GPU_BLEND_NORMAL);
	GPU_SetSnapMode(image->texture, GPU_SNAP_POSITION_AND_DIMENSIONS);
	image->width = image->texture->w;
	image->height = image->texture->h;
#else
	image->surface = STBIMG_Load_RW(ops, 1);
	if(image->surface == NULL) {
		rl_free(image, sizeof(image_t));
		free(image_data);
		rl_error("cannot decode image data '%s'", filename);
	}
	image->width = surface->w;
	image->height = surface->h;
	image->texture = SDL_CreateTextureFromSurface(display.renderer, surface);
	if(image->texture == NULL) {
		SDL_FreeSurface(image->surface);
		rl_free(image, sizeof(image_t));
		free(image_data);
		rl_error("cannot create texture for image '%s'", filename);
	}
#endif
	free(image_data);
	image->tiles_per_line = image->width / image->tile_width;
	return image;
}

void td_free_image(image_t* image) {
#ifdef USE_SDLGPU
	GPU_FreeImage(image->texture);
#else
	SDL_FreeSurface(image->surface);
	SDL_DestroyTexture(image->texture);
#endif
	rl_free(image, sizeof(image_t));
}

image_t* td_array_to_image(array_t* a, int tile_width, int tile_height, int palette_size, uint32_t* palette) {
	image_t* image = rl_malloc(sizeof(image_t));
	image->tile_width = tile_width;
	image->tile_height = tile_height;
	image->width = a->width;
	image->height = a->height;
	image->tiles_per_line = image->tile_width != 0 ? image->width / image->tile_width : 1; // prevent divide by zero
	int row_size = sizeof(uint32_t) * (a->width + a->stride);

#ifdef USE_SDLGPU
	image->texture = GPU_CreateImage(a->width, a->height, GPU_FORMAT_RGBA);
	GPU_SetImageFilter(image->texture, GPU_FILTER_NEAREST);
	GPU_SetAnchor(image->texture, 0, 0);
	GPU_SetBlending(image->texture, 1);
	GPU_SetBlendMode(image->texture, GPU_BLEND_NORMAL);
	GPU_SetSnapMode(image->texture, GPU_SNAP_POSITION_AND_DIMENSIONS);
	if(image->texture == NULL) {
		rl_free(image, sizeof(image_t));
		rl_error("cannot create texture");
	}
	if(palette_size > 0 && palette != NULL) {
		uint32_t* colors = malloc(sizeof(uint32_t) * a->width * a->height);
		for(int j = 0; j < a->height; j++) {
			for(int i = 0; i < a->width; i++) {
				VALUE value = rl_array_value(a, i, j);
				if(value >= 0 && value < palette_size) colors[j * a->width + i] = palette[value];
				else colors[j * a->width + i] = 0;
			}
		}
		GPU_UpdateImageBytes(image->texture, NULL, (const unsigned char*)colors, sizeof(uint32_t) * a->width);
		free(colors);
	} else {
		GPU_UpdateImageBytes(image->texture, NULL, (const unsigned char*)a->values, row_size);
	}
#else
	image->texture = SDL_CreateTexture(display.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, a->width, a->height);
	image->surface = SDL_CreateRGBSurfaceWithFormat(0, a->width, a->height, 32, SDL_PIXELFORMAT_RGBA32);
	}
	if(image->texture == NULL) {
		rl_free(image, sizeof(image_t));
		rl_error("cannot create texture");
	}
	SDL_UpdateTexture(image->texture, NULL, a->values, row_size);
	for(int j = 0; j < a->height; j++) memcpy(((uint32_t*) image->surface->pixels) + j * a->width, ((char*) a->values) + j * row_size, sizeof(uint32_t) * a->width);
#endif
	return image;
}

array_t* td_image_to_array(image_t* image) {
	if(image->texture == NULL) rl_error("invalid image");
#ifdef USE_SDLGPU
	SDL_Surface* surface = GPU_CopySurfaceFromImage(image->texture);
#else
	SDL_Surface* surface = image->surface;
#endif
	array_t* a = rl_array_new(image->width, image->height);
	memcpy(a->values, surface->pixels, sizeof(uint32_t) * a->width * a->height);
#ifdef USE_SDLGPU
	SDL_FreeSurface(surface);
#endif
	return a;
}

void td_draw_image(image_t* image, int x, int y) {
	if(image->texture == NULL) rl_error("invalid image");
#ifdef USE_SDLGPU
	GPU_Blit(image->texture, NULL, display.screen, x, y);
#else
	SDL_Rect rect = {x, y, image->width, image->height};
	SDL_RenderCopy(display.renderer, image->texture, NULL, &rect);
#endif
}

void td_draw_tile(image_t* image, int x, int y, int tile) {
	if(image->texture == NULL) rl_error("invalid image");
	int tile_x = (tile % image->tiles_per_line) * image->tile_width;
	int tile_y = (tile / image->tiles_per_line) * image->tile_height;
#ifdef USE_SDLGPU
	GPU_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
	GPU_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
	GPU_BlitRect(image->texture, &src_rect, display.screen, &dst_rect);
#else
	SDL_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
	SDL_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
	SDL_RenderCopy(display.renderer, image->texture, &src_rect, &dst_rect);
#endif
}

void td_colorize_tile(image_t* image, int x, int y, int tile, uint32_t fg, uint32_t bg) {
	if(image->texture == NULL) rl_error("invalid image");
	int tile_x = (tile % image->tiles_per_line) * image->tile_width;
	int tile_y = (tile / image->tiles_per_line) * image->tile_height;
#ifdef USE_SDLGPU
	SDL_Color fg_color = {td_color_r(fg), td_color_g(fg), td_color_b(fg), td_color_a(fg)};
	SDL_Color bg_color = {td_color_r(bg), td_color_g(bg), td_color_b(bg), td_color_a(bg)};
	GPU_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
	GPU_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
	if(bg != 0) GPU_RectangleFilled(display.screen, x, y, x + image->tile_width, y + image->tile_height, bg_color);
	if(fg != 0) GPU_SetColor(image->texture, fg_color);
	GPU_BlitRect(image->texture, &src_rect, display.screen, &dst_rect);
	GPU_UnsetColor(image->texture);
#else
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
#endif
}

/* slow scan for first bit set to 1 */
static int shift_from_mask(uint32_t number) {
  int i = 0;
  while(i < 32 && ((number & 1) == 0)) {
    number >>= 1;
    i++;
  }
  return i;
}

void td_draw_array(array_t* a, int x, int y, int x_shift, int y_shift, image_t* tile_image, int* tile_map, int tile_map_size, uint32_t tile_mask, uint32_t* fg_palette, int fg_palette_size, uint32_t fg_mask, uint32_t* bg_palette, int bg_palette_size, uint32_t bg_mask) {

  int tile_width, tile_height;
	if(tile_image) {
    if(tile_image->texture == NULL) rl_error("invalid image");
    if(x_shift == 0) x_shift = tile_image->tile_width;
    if(y_shift == 0) y_shift = tile_image->tile_height;
    tile_width = tile_image->tile_width;
    tile_height = tile_image->tile_height;
  } else {
    if(x_shift == 0) x_shift = 1;
    if(y_shift == 0) y_shift = 1;
    tile_width = x_shift;
    tile_height = y_shift;
  }

  int tile_shift = shift_from_mask(tile_mask);
  int fg_shift = shift_from_mask(fg_mask);
  int bg_shift = shift_from_mask(bg_mask);

	/* split background and foreground drawing to prevent slow switch of gl primitive type */
  if(bg_palette != NULL) {
    for(int j = 0; j < a->height; j++) {
      for(int i = 0; i < a->width; i++) {
        int num = rl_array_get(a, i, j);
        int bg_num = (num & bg_mask) >> bg_shift;
        if(bg_num >= 0 && bg_num < bg_palette_size) {
          uint32_t bg = bg_palette[bg_num];
#ifdef USE_SDLGPU
          SDL_Color bg_color = {td_color_r(bg), td_color_g(bg), td_color_b(bg), td_color_a(bg)};
          GPU_Rect dst_rect = {x + x_shift * i, y + y_shift * j, tile_width, tile_height};
          GPU_RectangleFilled2(display.screen, dst_rect, bg_color);
#else
          SDL_Rect dst_rect = {x + x_shift * i, y + y_shift * j, tile_width, tile_height};
          SDL_SetRenderDrawColor(display.renderer, td_color_r(bg), td_color_g(bg), td_color_b(bg), td_color_a(bg));
          SDL_RenderFillRect(display.renderer, &dst_rect);
#endif
        }
      }
      printf("\n");
    }
  }

  if(tile_image != NULL) {
    for(int j = 0; j < a->height; j++) {
      for(int i = 0; i < a->width; i++) {
        int num = rl_array_get(a, i, j);
        int fg_num = (num & fg_mask) >> fg_shift;
        int tile_num = (num & tile_mask) >> tile_shift;

        if(fg_palette != NULL) {
          uint32_t fg = 0xffffffff; // default to no colormod
          if(fg_num >= 0 && fg_num < fg_palette_size) fg = fg_palette[fg_num];
#ifdef USE_SDLGPU
          SDL_Color fg_color = {td_color_r(fg), td_color_g(fg), td_color_b(fg), td_color_a(fg)};
          GPU_SetColor(tile_image->texture, fg_color);
#else
          SDL_SetTextureColorMod(tile_image->texture, td_color_r(fg), td_color_g(fg), td_color_b(fg));
          SDL_SetTextureAlphaMod(tile_image->texture, td_color_a(fg));
#endif
        }

        int tile = tile_num; 
        if(tile_map != NULL) {
          if(tile_num >= 0 && tile_num < tile_map_size) tile = tile_map[tile_num];
          else continue;
        }
        int tile_x = (tile % tile_image->tiles_per_line) * tile_width;
        int tile_y = (tile / tile_image->tiles_per_line) * tile_height;
#ifdef USE_SDLGPU
        GPU_Rect src_rect = {tile_x, tile_y, tile_width, tile_height};
        GPU_Rect dst_rect = {x + x_shift * i, y + y_shift * j, tile_width, tile_height};
        GPU_BlitRect(tile_image->texture, &src_rect, display.screen, &dst_rect);
#else
        SDL_Rect src_rect = {tile_x, tile_y, tile_width, tile_height};
        SDL_Rect dst_rect = {x + x_shift * i, y + y_shift * j, tile_width, tile_height};
        SDL_RenderCopy(display.renderer, tile_image->texture, &src_rect, &dst_rect);
#endif
      }
    }
  }

  // restore state
  if(tile_image) {
#ifdef USE_SDLGPU
    GPU_UnsetColor(tile_image->texture);
#else
    SDL_SetTextureColorMod(tile_image->texture, 255, 255, 255);
    SDL_SetTextureAlphaMod(tile_image->texture, 255);
#endif
  }
}


// TODO: align is not implemented
void td_draw_text_from_tiles(image_t* image, int orig_x, int orig_y, const char* text, uint32_t color, int align) {
	if(image->texture == NULL) rl_error("invalid image");
	int width = strlen(text) * image->tile_width;
	int height = image->tile_height;
	if(align == 0) align = TD_ALIGN_LEFT | TD_ALIGN_TOP;
	if(align & TD_ALIGN_CENTER) orig_x -= width / 2;
	else if(align & TD_ALIGN_RIGHT) orig_x -= width;
	if(align & TD_ALIGN_MIDDLE) orig_y -= height / 2;
	else if(align & TD_ALIGN_BOTTOM) orig_y -= height;
	int x = orig_x, y = orig_y;
#ifdef USE_SDLGPU
	SDL_Color fg = {td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color)};
	GPU_SetColor(image->texture, fg);
#else
	SDL_SetTextureColorMod(image->texture, td_color_r(color), td_color_g(color), td_color_b(color));
	SDL_SetTextureAlphaMod(image->texture, td_color_a(color));
#endif
	while(text && *text != '\0') {
		if(*text == '\n') {
			x = orig_x;
			y += image->tile_height;
		} else {
			int tile_x = (*text % image->tiles_per_line) * image->tile_width;
			int tile_y = (*text / image->tiles_per_line) * image->tile_height;
#ifdef USE_SDLGPU
			GPU_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
			GPU_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
			GPU_BlitRect(image->texture, &src_rect, display.screen, &dst_rect);
#else
			SDL_Rect src_rect = {tile_x, tile_y, image->tile_width, image->tile_height};
			SDL_Rect dst_rect = {x, y, image->tile_width, image->tile_height};
			SDL_RenderCopy(display.renderer, image->texture, &src_rect, &dst_rect);
#endif
			x += image->tile_width;
		}
		text++;
	}
#ifdef USE_SDLGPU
	GPU_UnsetColor(image->texture);
#else
	SDL_SetTextureColorMod(image->texture, 255, 255, 255);
	SDL_SetTextureAlphaMod(image->texture, 255);
#endif
}

void td_draw_text(font_t* font, int orig_x, int orig_y, const char* text, uint32_t color, int align, int line_height) {
	int width, height;
	td_size_text(font, text, &width, &height);
	if(align == 0) align = TD_ALIGN_LEFT | TD_ALIGN_TOP;
	if(align & TD_ALIGN_CENTER) orig_x -= width / 2;
	else if(align & TD_ALIGN_RIGHT) orig_x -= width;
	if(align & TD_ALIGN_MIDDLE) orig_y -= height / 2;
	else if(align & TD_ALIGN_BOTTOM) orig_y -= height;
	int x = orig_x, y = orig_y + (int) (font->ascent * font->scale);
	if(line_height == 0) line_height = font->line_height;
	SDL_Color fg = {td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color)};
	while(text && *text != '\0') {
		if(*text == '\n') {
			x = orig_x;
			y += line_height;
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
#ifdef USE_SDLGPU
				STBTTF_RenderText(display.screen, font, x, y, line, fg);
#else
				SDL_SetRenderDrawColor(display.renderer, fg.r, fg.g, fg.b, fg.a);
				STBTTF_RenderText(display.renderer, font, x, y, line, fg);
#endif
			}
			text += length;
		}
	}
}

void td_size_text(font_t* font, const char* text, int* width, int* height) {
	*width = STBTTF_MeasureText(font, text);
	*height = font->line_height;
};

void td_fill_rect(int x, int y, int w, int h, uint32_t color) {
#ifdef USE_SDLGPU
	SDL_Color fg = {td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color)};
	GPU_RectangleFilled(display.screen, x, y, x + w, y + h, fg);
#else
	SDL_Rect rect = {x, y, w, h};
	SDL_SetRenderDrawColor(display.renderer, td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color));
	SDL_RenderFillRect(display.renderer, &rect);
#endif
}

void td_draw_rect(int x, int y, int w, int h, uint32_t color) {
#ifdef USE_SDLGPU
	SDL_Color fg = {td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color)};
	GPU_Rectangle(display.screen, x, y, x + w, y + h, fg);
#else
	SDL_Rect rect = {x, y, w, h};
	SDL_SetRenderDrawColor(display.renderer, td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color));
	SDL_RenderDrawRect(display.renderer, &rect);
#endif
}

void td_draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
#ifdef USE_SDLGPU
	SDL_Color fg = {td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color)};
	GPU_Line(display.screen, x1, y1, x2, y2, fg);
#else
	SDL_SetRenderDrawColor(display.renderer, td_color_r(color), td_color_g(color), td_color_b(color), td_color_a(color));
	SDL_RenderDrawLine(display.renderer, x1, y1, x2, y2);
#endif
}

// emscripten hack for redrawing when canvas was resized
void rl_force_redraw() {
	SDL_Event event = {.type = SDL_WINDOWEVENT};
	event.window.event = SDL_WINDOWEVENT_EXPOSED;
	SDL_PushEvent(&event);
}

void td_present() {
#ifndef USE_SDLGPU
	SDL_SetRenderTarget(display.renderer, NULL);
	td_clear();
#endif
#ifdef __EMSCRIPTEN__
	// canvas size is based on canvas container
	int canvas_fit_parent = EM_ASM_INT(return Module.canvas_fit_parent);
	if(canvas_fit_parent) {
	display.window_width = EM_ASM_INT({ 
			let container = Module.canvas.parentNode;
			return container.clientWidth;
	});
	display.window_height = EM_ASM_INT({ 
			let container = Module.canvas.parentNode;
			return container.clientHeight;
	});
	} else {
		display.window_width = display.width;
		display.window_height = display.height;
	}
	GPU_SetWindowResolution(display.window_width, display.window_height);
	display.device_pixel_ratio = emscripten_get_device_pixel_ratio();
	// fix handling of HiDPI by emscripten
	display.window_width = (int) (display.window_width * display.device_pixel_ratio);
	display.window_height = (int) (display.window_height * display.device_pixel_ratio);
#endif
	int width = display.window_width, height = display.window_height, x = 0, y = 0;
	if(width == 0 || height == 0) return;
	if(width * display.height / height < display.width) {
		int new_height = (int) display.height * width / display.width;
		y = (height - new_height) / 2;
		height = new_height;
	} else {
		int new_width = (int) display.width * height / display.height;
		x = (width - new_width) / 2;
		width = new_width;
	}
	display.scaled_rect.x = x;
	display.scaled_rect.y = y;
	display.scaled_rect.w = width;
	display.scaled_rect.h = height;
#ifdef USE_SDLGPU
	GPU_Clear(display.actual_screen);
	GPU_BlitRect(display.screen_image, NULL, display.actual_screen, &display.scaled_rect);
	GPU_Flip(display.actual_screen);
#else
	SDL_RenderCopy(display.renderer, display.screen, NULL, &display.scaled_rect);
	SDL_RenderPresent(display.renderer);
#endif
	// TODO: wait only the time needed to achieve fps
	SDL_Delay(1000 / 60);
#ifndef USE_SDLGPU
	SDL_SetRenderTarget(display.renderer, display.screen);
#endif
}

void td_clear() {
#ifdef USE_SDLGPU
	GPU_Clear(display.screen);
#else
  SDL_SetRenderDrawColor(display.renderer, 0, 0, 0, 255);
  SDL_RenderClear(display.renderer);
#endif
}

void td_quit() {
#ifdef __EMSCRIPTEN__
	emscripten_cancel_main_loop();
#else
	display.running = 0;
#endif
}

static void process_events() {
  int need_redraw = 0;
	SDL_Event event;
	while(SDL_PollEvent(&event) != 0) {
		switch(event.type) {
			case SDL_QUIT:
				td_quit();
				return;
				break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
#ifdef USE_SDLGPU
					GPU_SetWindowResolution(event.window.data1, event.window.data2);
#endif
					display.window_width = event.window.data1;
					display.window_height = event.window.data2;
          need_redraw = 1;
				} else if(event.window.event == SDL_WINDOWEVENT_EXPOSED) {
          need_redraw = 1;
				}
				break;
			case SDL_TEXTINPUT:
				display.key = event.text.text[0];
        if(display.update_filter & TD_ON_KEY) {
          display.update_callback(TD_KEY);
          need_redraw = 1;
        }
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN: {
        int mouse_x, mouse_y, mouse_state;
        mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);

				// convert mouse location
				display.mouse_x = (mouse_x - display.scaled_rect.x) 
					* display.width * display.device_pixel_ratio / display.scaled_rect.w;
				display.mouse_y = (mouse_y - display.scaled_rect.y) 
					* display.height * display.device_pixel_ratio / display.scaled_rect.h;

				// handle buttons
        display.mouse_button = 0;
        if(mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) display.mouse_button |= TD_MOUSE_LEFT;
        if(mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) display.mouse_button |= TD_MOUSE_MIDDLE;
        if(mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) display.mouse_button |= TD_MOUSE_RIGHT;

        if(display.update_filter & TD_ON_MOUSE) {
          if(event.type == SDL_MOUSEBUTTONUP) display.update_callback(TD_MOUSE_UP);
          else if(event.type == SDL_MOUSEBUTTONDOWN) display.update_callback(TD_MOUSE_DOWN);
          else if(display.mouse_x >= 0 && display.mouse_x < display.width && display.mouse_y >= 0 && display.mouse_y < display.height) 
            display.update_callback(TD_MOUSE_MOVED);
          need_redraw = 1;
        }
				break;
      }
			case SDL_KEYDOWN: {
				int key = event.key.keysym.sym;
				if(SDL_GetModState() & KMOD_ALT) {
					if(key == SDLK_RETURN) {
						if(display.is_fullscreen) {
							SDL_SetWindowFullscreen(display.window, 0);
							display.is_fullscreen = 0;
						} else {
							SDL_SetWindowFullscreen(display.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
							display.is_fullscreen = 1;
						}
            need_redraw = 1;
					} else if(key == SDLK_q) {
						// TODO: use customizable exit handler
						exit(1); // force quit
					}
        } else if((key < 32 || key > 127)
            && key != SDLK_LALT && key != SDLK_LSHIFT && key != SDLK_LCTRL 
            && key != SDLK_RALT && key != SDLK_RSHIFT && key != SDLK_RCTRL 
            && key != SDLK_LGUI && key != SDLK_RGUI) { 
          display.key = key;
          if(display.update_filter & TD_ON_KEY) {
            display.update_callback(TD_KEY);
            need_redraw = 1;
          }
        }
        break;

      }
		}
	}
  if(display.update_filter & TD_ON_REDRAW) {
    display.update_callback(TD_REDRAW);
    need_redraw = 1;
  }
  if(need_redraw) td_present();
}

void td_run(void (*update_callback)(int key), int update_filter) {
  if(INVALID_BITS(update_filter, TD_FILTER_BITS)) rl_error("invalid event filter");
	display.update_callback = update_callback;
	display.update_filter = update_filter;
	display.running = 1;
	update_callback(TD_REDRAW);
	td_present();
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(process_events, 0, 1);
#else
	while(display.running) {
		process_events();
	}
#endif
	update_callback(TD_QUIT);
}

uint32_t td_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return td_color_rgba(r, g, b, a);
}

#define hex2int(ch) \
	((ch >= '0' && ch <= '9') ? (ch - '0') : \
	( (ch >= 'A' && ch <= 'F') ? (ch - 'A' + 10) : \
	( (ch >= 'a' && ch <= 'f') ? (ch - 'a' + 10) : \
		(-1) )))

uint32_t td_hex_color(const char* color) {
	if(*color != '#') rl_error("invalid hex color");
	int length = strlen(color);
	int r, g, b, a = 255;
	if(length == 4) {
		r = hex2int(color[1]) << 4 | hex2int(color[1]);
		g = hex2int(color[2]) << 4 | hex2int(color[2]);
		b = hex2int(color[3]) << 4 | hex2int(color[3]);
	} else if(length == 7) {
		r = hex2int(color[1]) << 4 | hex2int(color[2]);
		g = hex2int(color[3]) << 4 | hex2int(color[4]);
		b = hex2int(color[5]) << 4 | hex2int(color[6]);
	} else if(length == 9) {
		a = hex2int(color[1]) << 4 | hex2int(color[2]);
		r = hex2int(color[3]) << 4 | hex2int(color[4]);
		g = hex2int(color[5]) << 4 | hex2int(color[6]);
		b = hex2int(color[7]) << 4 | hex2int(color[8]);
	} else rl_error("invalid hex color");
	if(r < 0 || g < 0 || b < 0 || a < 0) rl_error("invalid hex color");
	uint32_t result = td_color_rgba(r, g, b, a);
	return result;
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

int td_key() {
  return display.key;
}

uint32_t td_random_color() {
	return td_color_rgb(rl_random_int(0, 255), rl_random_int(0, 255), rl_random_int(0, 255));
}

int td_shift_pressed() {
	return SDL_GetModState() & KMOD_SHIFT;
}

int td_alt_pressed() {
	return SDL_GetModState() & KMOD_ALT;
}

int td_ctrl_pressed() {
	return SDL_GetModState() & KMOD_CTRL;
}

int td_win_pressed() {
	return SDL_GetModState() & KMOD_GUI;
}


