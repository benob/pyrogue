#include <math.h>
#include "rogue.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define WIDTH 80
#define HEIGHT 24
#define TILE_WIDTH 9
#define TILE_HEIGHT 16

double shift = 0;
uint32_t colors[256];
array_t* array = NULL;

void render() {
	td_clear();
	for(int y = 0; y < HEIGHT; y++)
		for(int x = 0; x < WIDTH; x++) {
			double value = sin(shift / 100 + x / 4.0);
			value += sin((shift / 100 + y / 4.0));
			value += sin(((shift / 100 + x + y) / 8.0));
			value += sin(sqrt(shift / 100 + x * x + y * y) / 16.0);
			rl_array_set(array, x, y, 128 + 128 * value / 4.0);
		}

	td_draw_array(0, array, 0, 0, TILE_WIDTH, TILE_HEIGHT, 256, NULL, colors, NULL) ;
	shift += 10;
}

int main(int argc, char** argv) {
#ifndef __EMSCRIPTEN__
	fs_open_resources("../examples/");
#endif
	td_init("plasma", WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT);
	td_load_image(0, "data/cp437.png", TILE_WIDTH, TILE_HEIGHT);

	for(int i = 0; i < 256; i++) colors[i] = td_hsv_color(i, 255, 255, 255);

	array = rl_array_new(WIDTH, HEIGHT);

	td_run(render, TD_UPDATE_LOOP);
	return 0;
}
