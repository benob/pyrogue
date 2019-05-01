#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rogue_array.h"
#include "rogue_util.h"
#include "rogue_random.h"

static int hex2int(char ch) {
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	return 7;
}

#define array_value(a, x, y) ((a)->values[((a)->stride + (a)->width) * (y) + (x)])

array_t* rl_array_new(uint32_t width, uint32_t height) {
  array_t *a = malloc(sizeof(array_t));
	a->values = calloc(sizeof(VALUE), width * height);
	a->stride = 0;
	a->is_view = 0;
  a->width = width;
  a->height = height;
  return a;
}

array_t* rl_array_from_string(const char* string) {
	const char* start = string, *end=NULL;
	int width = strtol(start, (char**) &end, 10); start = end + 1;
	int height = strtol(start, (char**) &end, 10); start = end + 1;
	array_t* a = rl_array_new(width, height);
	for(int i = 0; i < width * height; i++) {
		a->values[i] = strtol(start, (char**) &end, 10); 
		start = end + 1;
	}
	return a;
}

char* rl_array_to_string(array_t* a) {
	int allocated = 128, size = 0;
	char* result = malloc(allocated);
	char buffer[16];
	snprintf(result, 128, "%d %d", a->width, a->height);
	size = strlen(result);
	for(int i = 0; i < a->width * a->height; i++) {
		snprintf(buffer, 16, " %d", a->values[i]);
		if(size + strlen(buffer) + 1 > allocated) {
			allocated += allocated / 2;
			result = realloc(result, allocated);
		}
		strcat(result + size, buffer);
		size += strlen(buffer);
	}
	return result;
}

array_t* rl_array_view(array_t* b, int x, int y, uint32_t width, uint32_t height) {
	if(x >= (int) b->width || y >= (int) b->height) return NULL; // won't allow an empty view
	if(x < 0) { width -= -x; x = 0; }
	if(y < 0) { height -= -y; y = 0; }
	if(x + width > b->width) width = b->width - x;
	if(y + height > b->height) height = b->height - y;
  array_t *a = malloc(sizeof(array_t));
	a->values = &array_value(b, x, y);
	a->stride = b->width - width;
	a->is_view = 1;
  a->width = width;
  a->height = height;
  return a;
}

void rl_array_free(array_t* a) {
	if(!a->is_view) free(a->values); // if stride is not 0, then it's a view
	free(a);
}
uint32_t rl_array_width(array_t* a) {
	return a->width;
}

uint32_t rl_array_height(array_t* a) {
	return a->height;
}

VALUE rl_array_get(array_t* a, int x, int y) {
	if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
		return array_value(a, x, y);
	}
	fprintf(stderr, "invalid array get at (%d, %d)\n", x, y);
	return VALUE_MIN;
}

void rl_array_set(array_t* a, int x, int y, VALUE value) {
	if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
		array_value(a, x, y) = value;
	} else {
		fprintf(stderr, "invalid array set at (%d, %d)\n", x, y);
	}
}

void rl_array_fill(array_t *a, VALUE value) {
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			array_value(a, i, j) = value;
}

void rl_array_replace(array_t *a, VALUE value1, VALUE value2) {
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			if(array_value(a, i, j) == value1) array_value(a, i, j) = value2;
}

void rl_array_random_int(array_t *a, int lower, int upper) {
	if(lower > upper) lower = upper;
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			array_value(a, i, j) = rl_random_next() % (1 + upper - lower) + lower;
}

void rl_array_random(array_t *a) {
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			array_value(a, i, j) = (VALUE) rl_random_next(); // / UINT_MAX;
}

void rl_array_random_2d(array_t *a, int32_t x, int32_t y) {
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			array_value(a, i, j) = (VALUE) rl_random_2d(x + i, y + j); // / UINT_MAX;
}

void rl_array_print(array_t *a) {
  for(int y = 0; y < a->height; y++) {
    for(int x = 0; x < a->width; x++) {
			VALUE value = array_value(a, x, y);
			if(value == VALUE_MAX) printf("∞ ");
			else if(value == VALUE_MIN) printf("-∞ ");
			else printf(VALUE_FORMAT " ", value);
    }
    printf("\n");
  }
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

void rl_array_line(array_t* a, int x1, int y1, int x2, int y2, VALUE value) {
	int x = x1, y = y1, has_next = 1;
	for(rl_walk_line_start(x1, y1, x2, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
    if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
			array_value(a, x, y) = value;
    }
	}
}

// TODO: hollow rect (can be simulated with lines)
void rl_array_rect(array_t *a, int x, int y, uint32_t width, uint32_t height, VALUE value) {
  for(int j = y; j < y + height; j++) {
    for(int i = x; i < x + width; i++) {
      if(i >= 0 && i < a->width && j > 0 && j < a->height) {
				array_value(a, i, j) = value;
      }
    }
  }
}

// TODO: symetric line of sight
int rl_array_can_see(array_t *a, int x1, int y1, int x2, int y2, VALUE blocking) {
	int x = x1, y = y1, has_next = 1;
	for(rl_walk_line_start(x1, y1, x2, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
    if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
      if(array_value(a, x, y) == blocking) return 0;
    } else return 0;
	}
  //if(x1 == x2 && y1 == y2 && array_value(a, x1, y1) != blocking) return 1;
  //if(x1 == x2 && y1 == y2) return 1;
	return 1;

  /*int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = x1 < x2 ? 1 : -1;
  int sy = y1 < y2 ? 1 : -1;
  int err = dx - dy;

  while(1) {
    if(x1 >= 0 && x1 < a->width && y1 >= 0 && y1 < a->height) {
      if(array_value(a, x1, y1) == blocking) break;
    } else break;
    if(x1 == x2 && y1 == y2) break;
    int e2 = 2 * err;
    if(e2 > -dx) {
      err -= dy;
      x1 += sx;
    }
    if(e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }
  if(x1 == x2 && y1 == y2 && array_value(a, x1, y1) != blocking) return 1;
  //if(x1 == x2 && y1 == y2) return 1;
  return 0;*/
}

#define fov_test(x, y) \
	rl_array_set(result, (x), (y), rl_array_can_see(a, (xc), (yc), (x), (y), blocking));
#define fov_test_all_octants(x, y) \
	fov_test(xc+x, yc+y); \
	fov_test(xc-x, yc+y); \
	fov_test(xc+x, yc-y); \
	fov_test(xc-x, yc-y); \
	fov_test(xc+y, yc+x); \
	fov_test(xc-y, yc+x); \
	fov_test(xc+y, yc-x); \
	fov_test(xc-y, yc-x);

array_t* rl_array_field_of_view(array_t* a, int xc, int yc, int radius, VALUE blocking, int light_walls) {
	array_t* result = rl_array_new(a->width, a->height);
	int x, y;
	/*for(y = yc - radius; y <= yc + radius; y++) {
		for(x = xc - radius; x <= xc + radius; x++) {
			if(x >= 0 && x < result->width && y >= 0 && y < result->height) {
				int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
				if(distance < radius * radius) {
					array_value(result, x, y) = rl_array_can_see(a, xc, yc, x, y, blocking);
				}
			}
		}
	}*/
	int x2, y2;
	int radius_sqr = radius * radius;
	for(x2 = xc - radius; x2 <= xc + radius; x2++) {
		int x = xc, y = yc, has_next = 1;
		for(rl_walk_line_start(xc, yc, x2, yc - radius); has_next; has_next = rl_walk_line_next(&x, &y)) {
			if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
				int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
				if(distance > radius_sqr || array_value(a, x, y) == blocking) {
					break;
				} else {
					array_value(result, x, y) = 1;
				}
			} else break;
		}
		x = xc, y = yc, has_next = 1;
		for(rl_walk_line_start(xc, yc, x2, yc + radius); has_next; has_next = rl_walk_line_next(&x, &y)) {
			if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
				int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
				if(distance > radius_sqr || array_value(a, x, y) == blocking) {
					break;
				} else {
					array_value(result, x, y) = 1;
				}
			} else break;
		}
	}
	for(y2 = yc - radius; y2 <= yc + radius; y2++) {
		int x = xc, y = yc, has_next = 1;
		for(rl_walk_line_start(xc, yc, xc - radius, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
			if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
				int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
				if(distance > radius_sqr || array_value(a, x, y) == blocking) {
					break;
				} else {
					array_value(result, x, y) = 1;
				}
			} else break;
		}
		x = xc, y = yc, has_next = 1;
		for(rl_walk_line_start(xc, yc, xc + radius, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
			if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
				int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
				if(distance > radius_sqr || array_value(a, x, y) == blocking) {
					break;
				} else {
					array_value(result, x, y) = 1;
				}
			} else break;
		}
	}
	if(light_walls) {
		const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
		const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
		for(y = yc - radius; y <= yc + radius; y++) {
			for(x = xc - radius; x <= xc + radius; x++) {
				if(x >= 0 && x < result->width && y >= 0 && y < result->height && array_value(a, x, y) == blocking) {
					int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
					if(distance < radius * radius) {
						int sum = 0;
						for(int k = 0; k < 8; k++) {
							int i = x + dx[k];
							int j = y + dy[k];
							if(i >= 0 && i < result->width && j >= 0 && j < result->height) {
								if(array_value(a, i, j) != blocking && array_value(result, i, j)) {
									sum++;
									break;
								}
							}
						}
						if(sum > 0) array_value(result, x, y) = 1;
					}
				}
			}
		}
	}
	return result;
}

#define coords2index(x, y) (x + y * a->width)
path_t* rl_array_shortest_path(array_t* a, int x1, int y1, int x2, int y2, VALUE blocking) {
	if(x1 < 0 || x1 >= a->width || y1 < 0 || y1 >= a->height) return NULL;
	if(x2 < 0 || x2 >= a->width || y2 < 0 || y2 >= a->height) return NULL;
	uint32_t num = a->width * a->height;
	uint32_t* came_from = malloc(sizeof(uint32_t) * num);
	VALUE* cost_so_far = malloc(sizeof(VALUE) * num);
	for(int i = 0; i < num; i++) cost_so_far[i] = VALUE_MAX; // unprocessed
	int queue_allocated = 16;
	uint32_t* queue = malloc(sizeof(uint32_t) * queue_allocated);
	VALUE* priority = malloc(sizeof(VALUE) * queue_allocated);
	uint32_t size = 0;
	priority[size] = 0;
	queue[size++] = coords2index(x1, y1);
	cost_so_far[queue[0]] = 0;
	int neighbor_x[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int neighbor_y[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	while(size > 0) {
		// find lowest cost node
		VALUE min = priority[0];
		uint32_t argmin = 0;
		for(uint32_t i = 1; i < size; i++) {
			if(priority[i] < min) {
				min = priority[i];
				argmin = i;
			}
		}
		uint32_t current = queue[argmin];
		queue[argmin] = queue[size - 1];
		priority[argmin] = priority[size - 1];
		size--;
		int current_x = current % a->width;
		int current_y = current / a->width;
		for(uint32_t i = 0; i < 8; i++) {
			int next_x = current_x + neighbor_x[i];
			int next_y = current_y + neighbor_y[i];
			uint32_t next = next_y * a->width + next_x;
			if(next_x == x2 && next_y == y2) {
				cost_so_far[next] = cost_so_far[current] + 1;
				came_from[next] = current;
				size = 0;
				break;
			}
			if(next_x >= 0 && next_x < a->width && next_y >= 0 && next_y < a->height) {
				VALUE new_cost = cost_so_far[current] + 1;
				if(array_value(a, next_x, next_y) != blocking && new_cost < cost_so_far[next]) {
					cost_so_far[next] = new_cost;
					int dx = x2 - next_x;
					int dy = y2 - next_y;
					int distance = abs(dx) + abs(dy);
					priority[size] = new_cost + distance;
					queue[size] = next;
					came_from[next] = current;
					size++;
					if(size >= queue_allocated) {
						queue_allocated += queue_allocated / 2;
						if(queue_allocated > num) queue_allocated = num;
						queue = realloc(queue, sizeof(uint32_t) * queue_allocated);
						priority = realloc(priority, sizeof(VALUE) * queue_allocated);
					}
				}
			}
		}
	}
	uint32_t current = x2 + y2 * a->width;
	path_t* path = NULL;
	if(cost_so_far[current] != VALUE_MAX) {
		uint32_t path_size = (uint32_t) cost_so_far[current];
		path = malloc(sizeof(path_t) + sizeof(point_t) * (path_size - 1));
		path->size = path_size;
		while(cost_so_far[current] != 0 && cost_so_far[current] != VALUE_MAX) {
			path_size--;
			path->points[path_size].x = current % a->width;
			path->points[path_size].y = current / a->width;
			current = came_from[current];
		}
	}
	free(queue);
	free(priority);
	free(came_from);
	free(cost_so_far);
	return path;
}

void rl_array_dijkstra(array_t* a) {
  const int offset_x[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  const int offset_y[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

  int converged = 0;
  do {
    converged = 1;
    for(int y = 0; y < a->height; y++) {
      for(int x = 0; x < a->width; x++) {
        if(array_value(a, x, y) <= 0) continue;
        VALUE min = VALUE_MAX;
        for(int i = 0; i < 8; i++) {
          if(x + offset_x[i] >= 0 && x + offset_x[i] < a->width
              && y + offset_y[i] >= 0 && y + offset_y[i] < a->height) {
						VALUE value = array_value(a, x + offset_x[i], y + offset_y[i]);
            if(value >= 0 && value < min) {
              min = value;
            }
          }
        }
        if(min + 1 < array_value(a, x, y)) {
					array_value(a, x, y) = min + 1;
          converged = 0;
        }
      }
    }
  } while(!converged);
}

void rl_array_add(array_t* a, VALUE value, VALUE blocking) {
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			if(array_value(a, i, j) != blocking) array_value(a, i, j) += value;
}

void rl_array_mul(array_t *a, VALUE value, VALUE blocking) {
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++)
			if(array_value(a, i, j) != blocking) array_value(a, i, j) *= value;
}

VALUE rl_array_min(array_t *a, VALUE blocking) {
  VALUE min = VALUE_MAX;
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++) {
			VALUE value = array_value(a, i, j);
			if(value != blocking && value < min) min = value;
		}
	return min;
}

VALUE rl_array_max(array_t* a, VALUE blocking) {
  VALUE max = VALUE_MIN;
	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++) {
			VALUE value = array_value(a, i, j);
			if(value != blocking && value > max) max = value;
		}
  return max;
}

point_t rl_array_argmin(array_t* a, VALUE blocking) {
  VALUE min = VALUE_MAX;
	int argmin_x = 0, argmin_y = 0;
	for(uint32_t y = 0; y < a->height; y++) {
		for(uint32_t x = 0; x < a->height; x++) {
			VALUE value = array_value(a, x, y);
			if(value != blocking && value < min) {
				min = value; argmin_x = x; argmin_y = y;
			}
		}
	}
	point_t result = {argmin_x, argmin_y};
	return result;
}

// TODO: remove point_t 
point_t rl_array_argmax(array_t* a, VALUE blocking) {
  VALUE max = VALUE_MIN;
	int argmax_x = 0, argmax_y = 0;
	for(uint32_t y = 0; y < a->height; y++) {
		for(uint32_t x = 0; x < a->height; x++) {
			VALUE value = array_value(a, x, y);
			if(value != blocking && value > max) {
				max = value; argmax_x = x; argmax_y = y;
			}
		}
	}
  point_t result = {argmax_x, argmax_y};
	return result;
}

int rl_array_find_random(array_t* a, VALUE needle, int tries, int* rx, int* ry) {
	for(int i = 0; i < tries; i++) {
		int x = rl_random_next() % a->width;
		int y = rl_random_next() % a->height;
		if(array_value(a, x, y) == needle) {
			*rx = x;
			*ry = y;
			return 1;
		}
	}
	*rx = -1;
	*ry = -1;
	return 0;
}

// TODO deprecate: can be obtained with find_random + set
int rl_array_place_random(array_t* a, VALUE needle, VALUE value, int tries, int* rx, int *ry) {
	for(int i = 0; i < tries; i++) {
		int x = rl_random_next() % a->width;
		int y = rl_random_next() % a->height;
		if(array_value(a, x, y) == needle) {
			array_value(a, x, y) = value;
			*rx = x;
			*ry = y;
			return 1;
		}
	}
	*rx = -1;
	*ry = -1;
	return 0;
}

// TODO deprecate: can be obtained with view + copy
array_t* rl_array_get_sub(array_t* a, int x, int y, uint32_t width, uint32_t height, VALUE default_value) {
  array_t *b = rl_array_new(width, height);

  for(uint32_t j = 0; j < height; j++) {
    for(uint32_t i = 0; i < width; i++) {
      if(x + i >= 0 && x + i < a->width && y + j >= 0 && y + j < a->height) {
				array_value(b, i, j) = array_value(a, x + i, y + j);
      } else {
				array_value(b, i, j) = default_value;
      }
    }
  }

  return b;
}

// TODO deprecate: can be obtained with view + assign
void rl_array_set_sub(array_t *a, int x, int y, array_t* b) {
  for(uint32_t j = 0; j < b->height; j++) {
    for(uint32_t i = 0; i < b->width; i++) {
      if(x + i >= 0 && x + i < a->width
          && y + j >= 0 && y + j < a->height) {
				array_value(a, x + i, y + j) = array_value(b, i, j);
      }
    }
  }
}

array_t* rl_array_copy(array_t* a) {
	if(a->is_view) {
		return rl_array_view(a, 0, 0, a->width, a->height);
	} else {
		array_t *b = rl_array_new(a->width, a->height);
		for(int i = 0; i < a->width * a->height; i++) b->values[i] = a->values[i];
		return b;
	}
}

int rl_array_copy_masked(array_t* src, array_t* dest, array_t* mask, VALUE keep) {
	if(mask == NULL) {
		if(src->width != dest->width || src->height != dest->height) {
			printf("error: sizes mismatch");
			return 0;
		}
		for(int i = 0; i < dest->width * dest->height; i++) {
			dest->values[i] = src->values[i];
		}
	} else {
		if(src->width != dest->width || src->width != mask->width || src->height != dest->height || src->height != mask->height) {
			printf("error: sizes mismatch");
			return 0;
		}
		for(int i = 0; i < dest->width * dest->height; i++) {
			if(mask->values[i] == keep) dest->values[i] = src->values[i];
		}
	}
	return 1;
}

array_t* rl_array_equals(array_t* a, VALUE value) {
  array_t *b = rl_array_new(a->width, a->height);

	for(int j = 0; j < a->height; j++)
		for(int i = 0; i < a->width; i++) 
			array_value(b, i, j) = (array_value(a, i, j) == value);

	return b;
}

/* TODO */
/*static int array_cell_automaton (lua_State *L) {
  array_t *a = check_array(L, 1);
  VALUE alive_value = luaL_optnumber(L, 2, 1);
  VALUE dead_value = luaL_optnumber(L, 3, 0);
  int initial_alive = luaL_optinteger(L, 4, 44);
	int num_iterations = luaL_optinteger(L, 5, 5);
	int t1_neighbors = luaL_optinteger(L, 6, 5);
	int t1_value = luaL_optinteger(L, 7, alive_value);
	int t2_neighbors = luaL_optinteger(L, 8, 5);
	int t2_value = luaL_optinteger(L, 9, dead_value);

	for(int i = 0; i < a->width * a->height; i++) {
		if(rl_random_next() % 100 < initial_alive) a->values[i] = alive_value;
		else a->values[i] = dead_value;
	}

	for(int i = 0; i < a->width; i++) a->values[i] = a->values[(a->height - 1) * a->width + i] = alive_value;
	for(int i = 0; i < a->height; i++) a->values[i * a->width] = a->values[i * a->width + a->width - 1] = alive_value;
	
	int offset_x[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
	int offset_y[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
	for(int i = 0; i < num_iterations; i++) {
		for(int y = 1; y < a->height - 1; y++)
			for(int x = 1; x < a->width - 1; x++) {
				int neighbors = 0;
				for(int j = 0; j < 9; j++) if(a->values[(y + offset_y[j]) * a->width + x + offset_x[j]] == alive_value) neighbors++;
				if(neighbors >= t1_neighbors) a->values[y * a->width + x] = t1_value;
				else if(neighbors < t2_neighbors) a->values[y * a->width + x] = t2_value;
			}
	}
	lua_settop(L, 1);
  return 1;
}*/

