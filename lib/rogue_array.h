#ifndef __ROGUE_ARRAY_H__
#define __ROGUE_ARRAY_H__

#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stddef.h>

#define VALUE int32_t
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

#define rl_array_value(a, x, y) ((a)->values[((a)->stride + (a)->width) * (y) + (x)])


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
void rl_array_draw_line(array_t* a, int x1, int y1, int x2, int y2, VALUE value);
void rl_array_draw_rect(array_t *a, int x, int y, uint32_t width, uint32_t height, VALUE value);
void rl_array_fill_rect(array_t *a, int x, int y, uint32_t width, uint32_t height, VALUE value);
int rl_array_can_see(array_t *a, int x1, int y1, int x2, int y2, VALUE blocking);
array_t* rl_array_field_of_view(array_t* a, int xc, int yc, int radius, VALUE blocking, int light_walls);
path_t* rl_array_shortest_path(array_t* a, int x1, int y1, int x2, int y2, VALUE blocking);
void rl_array_dijkstra(array_t* a);
void rl_array_cell_automaton(array_t* a, const char* definition, int wrap);
VALUE rl_array_min(array_t *a, VALUE blocking);
VALUE rl_array_max(array_t* a, VALUE blocking);
point_t rl_array_argmin(array_t* a, VALUE blocking);
point_t rl_array_argmax(array_t* a, VALUE blocking);
int rl_array_find_random(array_t* a, VALUE needle, int tries, int* rx, int* ry);
array_t* rl_array_copy(array_t* a, array_t* mask);
void rl_array_copy_to(array_t* src, array_t* dest, array_t* mask);

#define rl_array_op(name) \
	array_t* rl_array_left_##name##_value(array_t* a, VALUE value); \
	array_t* rl_array_left_##name##_other(array_t* a, array_t* other); \
	array_t* rl_array_right_##name##_value(array_t* a, VALUE value); \
	array_t* rl_array_right_##name##_other(array_t* a, array_t* other); \
	void rl_array_inplace_##name##_value(array_t* a, VALUE value); \
	void rl_array_inplace_##name##_other(array_t* a, array_t* other); 

rl_array_op(equal);
rl_array_op(greater_than);
rl_array_op(less_than);
rl_array_op(greater_or_equal_than);
rl_array_op(less_or_equal_than);
rl_array_op(not_equal);
rl_array_op(and);
rl_array_op(or);
rl_array_op(xor);
rl_array_op(lshift);
rl_array_op(rshift);
rl_array_op(add);
rl_array_op(sub);
rl_array_op(mul);
rl_array_op(div);
rl_array_op(mod);

array_t* rl_array_unary_minus(array_t* a);
array_t* rl_array_unary_not(array_t* a);
array_t* rl_array_unary_invert(array_t* a);
int rl_array_any_equals(array_t* a, VALUE value);
int rl_array_all_equal(array_t* a, VALUE value);
int rl_array_count(array_t* a, VALUE value);
uint64_t rl_array_sum(array_t* a);
array_t* rl_array_abs(array_t* a);
array_t* rl_array_matmul(array_t* a, array_t* b);
array_t* rl_array_apply_kernel(array_t* a, array_t* kernel);

#endif
