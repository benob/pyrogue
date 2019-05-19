#ifndef __STBTTF_H__
#define __STBTTF_H__

#include <SDL.h>
#ifdef USE_SDLGPU
#include <SDL_gpu.h>
#endif

#include "stb_rect_pack.h"
#include "stb_truetype.h"

/* STBTTF: A quick and dirty SDL2 text renderer based on stb_truetype and stdb_rect_pack.
 * Benoit Favre 2019
 *
 * This header-only addon to the stb_truetype library allows to draw text with SDL2 from
 * TTF fonts with a similar API to SDL_TTF without the bloat.
 * The renderer is however limited by the integral positioning of SDL blit functions.
 * It also does not parse utf8 text and only prints ASCII characters.
 *
 * This code is public domain.
 */

typedef struct {
	stbtt_fontinfo* info;
	stbtt_packedchar* chars;
#ifdef USE_SDLGPU
	GPU_Image* atlas;
#else
	SDL_Texture* atlas;
#endif
	int texture_size;
	float size;
	float scale;
	int ascent;
	int baseline;
} STBTTF_Font;

/* Release the memory and textures associated with a font */
void STBTTF_CloseFont(STBTTF_Font* font);

/* Open a TTF font given a SDL abstract IO handler, for a given renderer and a given font size.
 * Returns NULL on failure. The font must be deallocated with STBTTF_CloseFont when not used anymore.
 * This function creates a texture atlas with prerendered ASCII characters (32-128).
 */ 
#ifdef USE_SDLGPU
STBTTF_Font* STBTTF_OpenFontRW(SDL_RWops* rw, float size);
#else
STBTTF_Font* STBTTF_OpenFontRW(SDL_Renderer* renderer, SDL_RWops* rw, float size);
#endif

/* Open a TTF font given a filename, for a given renderer and a given font size.
 * Convinience function which calls STBTTF_OpenFontRW.
 */
#ifdef USE_SDLGPU
STBTTF_Font* STBTTF_OpenFont(const char* filename, float size);
#else
STBTTF_Font* STBTTF_OpenFont(SDL_Renderer* renderer, const char* filename, float size);
#endif

/* Draw some text using the renderer draw color at location (x, y).
 * Characters are copied from the texture atlas using the renderer SDL_RenderCopy function.
 * Since that function only supports integral coordinates, the result is not great.
 * Only ASCII characters (32 <= c < 128) are supported. Anything outside this range is ignored.
 */
#ifdef USE_SDLGPU
void STBTTF_RenderText(GPU_Target* screen, STBTTF_Font* font, float x, float y, const char *text, SDL_Color fg);
#else
void STBTTF_RenderText(SDL_Renderer* renderer, STBTTF_Font* font, float x, float y, const char *text, SDL_Color fg);
#endif

/* Return the length in pixels of a text. 
 * You can get the height of a line by using font->baseline.
 */
float STBTTF_MeasureText(STBTTF_Font* font, const char *text);

#ifdef STBTTF_IMPLEMENTATION

void STBTTF_CloseFont(STBTTF_Font* font) {
#ifdef USE_SDLGPU
	if(font->atlas) GPU_FreeImage(font->atlas);
#else
	if(font->atlas) SDL_DestroyTexture(font->atlas);
#endif
	if(font->info) free(font->info);
	if(font->chars) free(font->chars);
	free(font);
}

#ifdef USE_SDLGPU
STBTTF_Font* STBTTF_OpenFontRW(SDL_RWops* rw, float size) {
#else
STBTTF_Font* STBTTF_OpenFontRW(SDL_Renderer* renderer, SDL_RWops* rw, float size) {
#endif
	Sint64 file_size = SDL_RWsize(rw);
	unsigned char* buffer = malloc(file_size);
	if(SDL_RWread(rw, buffer, file_size, 1) != 1) return NULL;
	SDL_RWclose(rw);

	STBTTF_Font* font = calloc(sizeof(STBTTF_Font), 1);
	font->info = malloc(sizeof(stbtt_fontinfo));
	font->chars = malloc(sizeof(stbtt_packedchar) * 96);

	if(stbtt_InitFont(font->info, buffer, 0) == 0) {
		free(buffer);
		STBTTF_CloseFont(font);
		return NULL;
	}

	// fill bitmap atlas with packed characters
	unsigned char* bitmap = NULL;
	font->texture_size = 32;
	while(1) {
		bitmap = malloc(font->texture_size * font->texture_size);
		stbtt_pack_context pack_context;
		stbtt_PackBegin(&pack_context, bitmap, font->texture_size, font->texture_size, 0, 1, 0);
		stbtt_PackSetOversampling(&pack_context, 1, 1);
		if(!stbtt_PackFontRange(&pack_context, buffer, 0, size, 32, 95, font->chars)) {
			// too small
			free(bitmap);
			stbtt_PackEnd(&pack_context);
			font->texture_size *= 2;
		} else {
			stbtt_PackEnd(&pack_context);
			break;
		}
	}

	// convert bitmap to texture
#ifdef USE_SDLGPU
	font->atlas = GPU_CreateImage(font->texture_size, font->texture_size, GPU_FORMAT_RGBA);
	Uint32* pixels = malloc(font->texture_size * font->texture_size * sizeof(Uint32));
	for(int i = 0; i < font->texture_size * font->texture_size; i++) {
		pixels[i] = 0xffffff | (bitmap[i] << 24);
	}
	GPU_UpdateImageBytes(font->atlas, NULL, (unsigned char*) pixels, sizeof(Uint32) * font->texture_size);
	free(pixels);
	GPU_SetImageFilter(font->atlas, GPU_FILTER_NEAREST);
	GPU_SetAnchor(font->atlas, 0, 1);
	GPU_SetBlending(font->atlas, 1);
	GPU_SetSnapMode(font->atlas, GPU_SNAP_POSITION_AND_DIMENSIONS);
#else
	font->atlas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, font->texture_size, font->texture_size);
	SDL_SetTextureBlendMode(font->atlas, SDL_BLENDMODE_BLEND);

	Uint32* pixels = malloc(font->texture_size * font->texture_size * sizeof(Uint32));
	static SDL_PixelFormat* format = NULL;
	if(format == NULL) format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	for(int i = 0; i < font->texture_size * font->texture_size; i++) {
		pixels[i] = SDL_MapRGBA(format, 0xff, 0xff, 0xff, bitmap[i]);
	}
	SDL_UpdateTexture(font->atlas, NULL, pixels, font->texture_size * sizeof(Uint32));
	free(pixels);
	free(bitmap);
#endif

	// setup additional info
  font->scale = stbtt_ScaleForPixelHeight(font->info, size);
	stbtt_GetFontVMetrics(font->info, &font->ascent, 0, 0);
  font->baseline = (int) (font->ascent * font->scale);

	free(buffer);

	return font;
}

#ifdef USE_SDLGPU
STBTTF_Font* STBTTF_OpenFont(const char* filename, float size) {
#else
STBTTF_Font* STBTTF_OpenFont(SDL_Renderer* renderer, const char* filename, float size) {
#endif
	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
	if(rw == NULL) return NULL;
#ifdef USE_SDLGPU
	return STBTTF_OpenFontRW(rw, size);
#else
	return STBTTF_OpenFontRW(renderer, rw, size);
#endif
}

#ifdef USE_SDLGPU
void STBTTF_RenderText(GPU_Target* screen, STBTTF_Font* font, float x, float y, const char *text, SDL_Color fg) {
	GPU_SetColor(font->atlas, fg);
#else
void STBTTF_RenderText(SDL_Renderer* renderer, STBTTF_Font* font, float x, float y, const char *text, SDL_Color fg) {
	SDL_SetTextureColorMod(font->atlas, fg.r, fg.g, fg.b);
	SDL_SetTextureAlphaMod(font->atlas, fg.a);
#endif
	for(int i = 0; text[i]; i++) {
		if (text[i] >= 32 && text[i] < 127) {
			//if(i > 0) x += stbtt_GetCodepointKernAdvance(font->info, text[i - 1], text[i]) * font->scale;

			stbtt_packedchar* info = &font->chars[text[i] - 32];

#ifdef USE_SDLGPU
			GPU_Rect src_rect = {info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0};
			GPU_Rect dst_rect = {x + info->xoff, y + info->yoff, info->x1 - info->x0, info->y1 - info->y0};
			GPU_BlitRect(font->atlas, &src_rect, screen, &dst_rect);
#else
			SDL_Rect src_rect = {info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0};
			SDL_Rect dst_rect = {x + info->xoff, y + info->yoff, info->x1 - info->x0, info->y1 - info->y0};
			SDL_RenderCopy(renderer, font->atlas, &src_rect, &dst_rect);
#endif
			x += info->xadvance;
		}
	}
#ifdef USE_SDLGPU
	//GPU_UnsetColor(font->atlas);
#endif
}

float STBTTF_MeasureText(STBTTF_Font* font, const char *text) {
	float width = 0;
	for(int i = 0; text[i]; i++) {
		if (text[i] >= 32 && text[i] <= 127) {
			//if(i > 0) width += stbtt_GetCodepointKernAdvance(font->info, text[i - 1], text[i]) * font->scale;

			stbtt_packedchar* info = &font->chars[text[i] - 32];
			width += info->xadvance;
		}
	}
	return width;
}

/*******************
 * Example program *
 *******************

#include <stdio.h>

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTTF_IMPLEMENTATION

#include "stbttf.h"

int main(int argc, char** argv) {
	if(argc != 2) {
		fprintf(stderr, "usage: %s <font>\n", argv[0]);
		exit(1);
	}
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("stbttf", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(renderer, 640, 480);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	STBTTF_Font* font = STBTTF_OpenFont(renderer, argv[1], 32);

	while(1) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) exit(0);
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		// set color and render some text
		SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
		STBTTF_RenderText(renderer, font, 50, 50, "This is a test");
		// render the atlas to check its content
		//SDL_Rect dest = {0, 0, font->texturesize, font->texturesize};
		//SDL_RenderCopy(renderer, font->atlas, &dest, &dest);
		SDL_RenderPresent(renderer);
		SDL_Delay(1000 / 60);
	}
	STBTTF_CloseFont(font);
	SDL_Quit();
}

*/

#endif

#endif
