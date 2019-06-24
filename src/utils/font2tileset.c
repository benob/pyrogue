#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

static char* utf8_ascii_table[] = {
    " ", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "◙", "♂", "♀", "♪", "♫", "☼",
    "►", "◄", "↕", "‼", "¶", "§", "▬", "↨", "↑", "↓", "→", "←", "∟", "↔", "▲", "▼",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", 
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", 
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", 
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "⌂",
    "Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å",
    "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "¢", "£", "¥", "₧", "ƒ",
    "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»",
    "░", "▒", "▓", "│", "┤", "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝", "╜", "╛", "┐",
    "└", "┴", "┬", "├", "─", "┼", "╞", "╟", "╚", "╔", "╩", "╦", "╠", "═", "╬", "╧",
    "╨", "╤", "╥", "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█", "▄", "▌", "▐", "▀",
    "α", "ß", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ", "Θ", "Ω", "δ", "∞", "φ", "ε", "∩",
    "≡", "±", "≥", "≤", "⌠", "⌡", "÷", "≈", "°", "∙", "·", "√", "ⁿ", "²", "■", "□",
};

int main(int argc, char* argv[]) {
	if(argc != 7) {
		fprintf(stderr, "usage: %s <font> <font-size> <style> <tile-width> <tile-height> <output.png>\n", argv[0]);
		return 1;
	}
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) return 1;
	if(IMG_Init(IMG_INIT_PNG) < 0) return 1;
	if(TTF_Init() < 0) return 1;

	int size = strtol(argv[2], NULL, 10);
	int width = strtol(argv[4], NULL, 10);
	int height = strtol(argv[5], NULL, 10);
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

	//TTF_SetFontHinting(font, TTF_HINTING_LIGHT);

	SDL_Surface* tileset = SDL_CreateRGBSurfaceWithFormat(0, 16 * width, 16 * height, 32, SDL_PIXELFORMAT_RGBA32);
	if(tileset == NULL) {
		fprintf(stderr, "could not create tileset surface of size %dx%d\n", 16 * width, 16 * height);
		return 1;
	}

	SDL_Color fg = {255, 255, 255, 255};
	for(int i = 0; i < 256; i++) {
		SDL_Surface* rendered = TTF_RenderUTF8_Blended(font, utf8_ascii_table[i], fg);
		int x = i % 16;
		int y = i / 16;
		SDL_Rect dst_rect = {x * width + (width - rendered->w) / 2, y * height + height - rendered->h, rendered->w, rendered->h};
		SDL_BlitSurface(rendered, NULL, tileset, &dst_rect);
		SDL_FreeSurface(rendered);
	}

	if(IMG_SavePNG(tileset, argv[6]) < 0) {
		fprintf(stderr, "could not save result to '%s'\n", argv[5]);
		return 1;
	}

	SDL_FreeSurface(tileset);
	return 0;
}
