#ifndef __ROGUE_ARRAY_H__
#define __ROGUE_ARRAY_H__

#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stddef.h>

/*
#define VALUE float
#define VALUE_MIN FLT_MIN
#define VALUE_MAX FLT_MAX
#define VALUE_FORMAT "%2g"
*/
#define VALUE int
#define VALUE_MIN INT_MIN
#define VALUE_MAX INT_MAX
#define VALUE_FORMAT "%d"

/* array type */
typedef struct {
  uint32_t width, height;
	uint32_t stride;
	int is_view;
  VALUE* values;
} array_t;

typedef struct {
	int x, y;
} point_t;

typedef struct {
	int size;
	point_t points[1]; /* variable part */
} path_t;

array_t* rl_array_new(uint32_t width, uint32_t height);
array_t* rl_array_from_string(const char* string);
char* rl_array_to_string(array_t* a);
array_t* rl_array_view(array_t* b, int x, int y, uint32_t width, uint32_t height);
void rl_array_free(array_t* a);
uint32_t rl_array_width(array_t* a);
uint32_t rl_array_height(array_t* a);
VALUE rl_array_get(array_t* a, int x, int y);
void rl_array_set(array_t* a, int x, int y, VALUE value);
void rl_array_fill(array_t *a, VALUE value);
void rl_array_replace(array_t *a, VALUE value1, VALUE value2);
void rl_array_random_int(array_t *a, int lower, int upper);
void rl_array_random_2d(array_t *a, int lower, int upper);
void rl_array_random(array_t *a);
void rl_array_print(array_t *a);
void rl_walk_line_start(int x1, int y1, int x2, int y2);
int rl_walk_line_next(int *rx, int* ry);
void rl_array_line(array_t* a, int x1, int y1, int x2, int y2, VALUE value);
void rl_array_rect(array_t *a, int x, int y, uint32_t width, uint32_t height, VALUE value);
int rl_array_can_see(array_t *a, int x1, int y1, int x2, int y2, VALUE blocking);
array_t* rl_array_field_of_view(array_t* a, int xc, int yc, int radius, VALUE blocking, int light_walls);
path_t* rl_array_shortest_path(array_t* a, int x1, int y1, int x2, int y2, VALUE blocking);
void rl_array_dijkstra(array_t* a);
//point_t rl_array_best_move(array_t *a, int x, int y);
void rl_array_add(array_t* a, VALUE value, VALUE except);
void rl_array_mul(array_t *a, VALUE value, VALUE except);
VALUE rl_array_min(array_t *a, VALUE blocking);
VALUE rl_array_max(array_t* a, VALUE blocking);
point_t rl_array_argmin(array_t* a, VALUE blocking);
point_t rl_array_argmax(array_t* a, VALUE blocking);
int rl_array_find_random(array_t* a, VALUE needle, int tries, int* rx, int* ry);
int rl_array_place_random(array_t* a, VALUE needle, VALUE value, int tries, int* rx, int* ry);
array_t* rl_array_get_sub(array_t* a, int x, int y, uint32_t width, uint32_t height, VALUE default_value);
void rl_array_set_sub(array_t *a, int x, int y, array_t* b);
array_t* rl_array_copy(array_t* a);
int rl_array_copy_masked(array_t* src, array_t* dest, array_t* mask, VALUE keep);
array_t* rl_array_equals(array_t* a, VALUE value);

#endif
