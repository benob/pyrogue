#include <math.h>
#include "rogue.h"

#define WIDTH 80
#define HEIGHT 24
#define TILE_WIDTH 9
#define TILE_HEIGHT 16

int main(int argc, char** argv) {
	//fs_open_resources("examples/");
	td_init("plasma", WIDTH * TILE_WIDTH, HEIGHT * TILE_HEIGHT);
	td_load_image(0, "../examples/data/cp437.png", TILE_WIDTH, TILE_HEIGHT);

	double shift = 0;
	uint32_t colors[256];
	for(int i = 0; i < 256; i++) colors[i] = td_hsv_color(i, 255, 255, 255);

	array_t* array = rl_array_new(WIDTH, HEIGHT);

	while(td_still_running()) {
		td_poll_event();
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
		td_present();
		td_delay(1000 / 60);
	}
	return 0;
}
