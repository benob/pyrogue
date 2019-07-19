#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "rogue_util.h"

static void default_error_handler(const char* message) {
	fprintf(stderr, "ERROR: %s\n", message);
	exit(1);
}

error_handler_t _rl_current_error_handler = default_error_handler;
char _error_buffer[1024]; // needed for message to survive long jump

void rl_set_error_handler(error_handler_t error_handler) {
	if(error_handler == NULL) _rl_current_error_handler = default_error_handler;
	else _rl_current_error_handler = error_handler;
}

static void* _realloc_wrapper(void* mem, size_t old_size, size_t new_size) {
	return realloc(mem, new_size);
}

static void _free_wrapper(void* mem, size_t size) {
	free(mem);
}

void rl_set_allocator(void* (*malloc_in)(size_t size), void* (*realloc_in)(void* mem, size_t old_size, size_t new_size), void (*free_in)(void* mem, size_t size)) {
	if(malloc_in != NULL) {
		rl_malloc = malloc_in;
		rl_realloc = realloc_in;
		rl_free = free_in;
	} else {
		printf("unset allocator\n");
		rl_malloc = malloc;
		rl_realloc = _realloc_wrapper;
		rl_free = _free_wrapper;
	}
}

void* (*rl_malloc)(size_t size) = malloc;
void* (*rl_realloc)(void* mem, size_t old_size, size_t new_size) = _realloc_wrapper;
void (*rl_free)(void* mem, size_t size) = _free_wrapper;

float rl_distance(float x1, float y1, float x2, float y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	return sqrtf(dx * dx + dy * dy);
}

// line walking based on libtcod/bresenham_c.c 
typedef struct {
	int x1, y1, x2, y2;
	int dx, dy, sx, sy, e;
} line_t;
static line_t line;

void rl_walk_line_start(int x1, int y1, int x2, int y2) {
	line.x1=x1; line.y1=y1;
	line.x2=x2; line.y2=y2;
	line.dx=x2 - x1;
	line.dy=y2 - y1;
	if ( line.dx > 0 ) {
		line.sx=1;
	} else if ( line.dx < 0 ){
		line.sx=-1;
	} else line.sx=0;
	if ( line.dy > 0 ) {
		line.sy=1;
	} else if ( line.dy < 0 ){
		line.sy=-1;
	} else line.sy = 0;
	if ( line.sx*line.dx > line.sy*line.dy ) {
		line.e = line.sx*line.dx;
		line.dx *= 2;
		line.dy *= 2;
	} else {
		line.e = line.sy*line.dy;
		line.dx *= 2;
		line.dy *= 2;
	}
}
int rl_walk_line_next(int *x, int *y) {
	if ( line.sx*line.dx > line.sy*line.dy ) {
		if ( line.x1 == line.x2 ) return 0;
		line.x1+=line.sx;
		line.e -= line.sy*line.dy;
		if ( line.e < 0) {
			line.y1+=line.sy;
			line.e+=line.sx*line.dx;
		}
	} else {
		if ( line.y1 == line.y2 ) return 0;
		line.y1+=line.sy;
		line.e -= line.sx*line.dx;
		if ( line.e < 0) {
			line.x1+=line.sx;
			line.e+=line.sy*line.dy;
		}
	}
	*x=line.x1;
	*y=line.y1;
	return 1;
}

