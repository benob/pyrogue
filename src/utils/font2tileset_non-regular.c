#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

int main(int argc, char* argv[]) {
	if(argc != 5) {
		fprintf(stderr, "usage: %s <font> <font-size> <style> <output.png>\n", argv[0]);
		return 1;
	}
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) return 1;
	if(IMG_Init(IMG_INIT_PNG) < 0) return 1;
	if(TTF_Init() < 0) return 1;

	int size = strtol(argv[2], NULL, 10);
	char* style = argv[3];
	TTF_Font* font = TTF_OpenFont(argv[1], size);
	if(font == NULL) {
		fprintf(stderr, "could not load font '%s'\n", argv[1]);
		return 1;
	}
	if(!strcmp(style, "bold")) {
		TTF_SetFontStyle(font, TTF_STYLE_BOLD);
	} else if(!strcmp(style, "italic")) {
		TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
	} else if(strcmp(style, "normal")) {
		fprintf(stderr, "invalid style '%s', try normal, bold or italic.\n", style);
		return 1;
	}

  printf("font name=%s\n", argv[1]);
  printf("font ascent=%d\n", TTF_FontAscent(font));
  printf("font descent=%d\n", TTF_FontDescent(font));
  printf("font lineskip=%d\n", TTF_FontLineSkip(font));
  // compute offsets
  int last_width = 0;
  char text[95];
  for(int i = 0; i < 95; i++) {
    text[i] = i + 32;
    text[i + 1] = '\0';

    int width = 0, height = 0;
    TTF_SizeText(font, text, &width, &height);
    printf("char id=%d offset=%d width=%d glyph=%c\n", i + 32, last_width, width - last_width, i + 32);
    last_width = width;
  }

  // render to png
	SDL_Color fg = {255, 255, 255, 255};
  SDL_Surface* rendered = TTF_RenderUTF8_Blended(font, text, fg);

	if(IMG_SavePNG(rendered, argv[4]) < 0) {
		fprintf(stderr, "could not save result to '%s'\n", argv[4]);
		return 1;
	}

	SDL_FreeSurface(rendered);
	return 0;
}
