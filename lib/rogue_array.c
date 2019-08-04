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

array_t* rl_array_new(uint32_t width, uint32_t height) {
  array_t *a = rl_malloc(sizeof(array_t));
  a->values = rl_malloc(sizeof(VALUE) * width * height);
  memset(a->values, 0, sizeof(VALUE) * width * height);
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
  if(x >= (int) b->width || y >= (int) b->height) rl_error("empty view");
  if(x < 0) { width -= -x; x = 0; }
  if(y < 0) { height -= -y; y = 0; }
  if(x + width > b->width) width = b->width - x;
  if(y + height > b->height) height = b->height - y;
  array_t *a = rl_malloc(sizeof(array_t));
  a->values = &rl_array_value(b, x, y);
  a->stride = b->stride + b->width - width;
  a->is_view = 1;
  a->width = width;
  a->height = height;
  return a;
}

void rl_array_free(array_t* a) {
  if(!a->is_view) rl_free(a->values, sizeof(VALUE) * a->width * a->height);
  rl_free(a, sizeof(array_t));
}
uint32_t rl_array_width(array_t* a) {
  return a->width;
}

uint32_t rl_array_height(array_t* a) {
  return a->height;
}

VALUE rl_array_get(array_t* a, int x, int y) {
  if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
    return rl_array_value(a, x, y);
  }
  fprintf(stderr, "invalid array get at (%d, %d)\n", x, y);
  return VALUE_MIN;
}

void rl_array_set(array_t* a, int x, int y, VALUE value) {
  if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
    rl_array_value(a, x, y) = value;
  } else {
    fprintf(stderr, "invalid array set at (%d, %d)\n", x, y);
  }
}

void rl_array_fill(array_t *a, VALUE value) {
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(a, i, j) = value;
}

void rl_array_replace(array_t *a, VALUE value1, VALUE value2) {
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      if(rl_array_value(a, i, j) == value1) rl_array_value(a, i, j) = value2;
}

void rl_array_random_int(array_t *a, int lower, int upper) {
  if(lower > upper) lower = upper;
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(a, i, j) = rl_random_next() % (1 + upper - lower) + lower;
}

void rl_array_random(array_t *a) {
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(a, i, j) = (VALUE) rl_random_next(); // / UINT_MAX;
}

void rl_array_random_2d(array_t *a, int32_t x, int32_t y) {
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(a, i, j) = (VALUE) rl_random_2d(x + i, y + j); // / UINT_MAX;
}

void rl_array_print(array_t *a) {
  for(int y = 0; y < a->height; y++) {
    for(int x = 0; x < a->width; x++) {
      VALUE value = rl_array_value(a, x, y);
      if(value == VALUE_MAX) printf("∞ ");
      else if(value == VALUE_MIN) printf("-∞ ");
      else printf(VALUE_FORMAT " ", value);
    }
    printf("\n");
  }
}

void rl_array_draw_line(array_t* a, int x1, int y1, int x2, int y2, VALUE value) {
  int x = x1, y = y1, has_next = 1;
  for(rl_walk_line_start(x1, y1, x2, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
    if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
      rl_array_value(a, x, y) = value;
    }
  }
}

void rl_array_draw_rect(array_t *a, int x, int y, uint32_t width, uint32_t height, VALUE value) {
  if(x > a->width || y > a->height) return;
  if(x < 0) { width += x; x = 0; }
  if(y < 0) { height += y; y = 0; }
  if(x + width >= a->width) width = a->width - x;
  if(y + height >= a->height) height = a->height - y;
  for(int j = y; j < y + height; j++) {
    rl_array_value(a, x, j) = value;
    rl_array_value(a, x + width - 1, j) = value;
  }
  for(int i = x; i < x + width; i++) {
    rl_array_value(a, i, y) = value;
    rl_array_value(a, i, y + height - 1) = value;
  }
}

void rl_array_fill_rect(array_t *a, int x, int y, uint32_t width, uint32_t height, VALUE value) {
  if(x > a->width || y > a->height) return;
  if(x < 0) { width += x; x = 0; }
  if(y < 0) { height += y; y = 0; }
  if(x + width >= a->width) width = a->width - x;
  if(y + height >= a->height) height = a->height - y;
  for(int j = y; j < y + height; j++) {
    for(int i = x; i < x + width; i++) {
      if(i >= 0 && i < a->width && j > 0 && j < a->height) {
        rl_array_value(a, i, j) = value;
      }
    }
  }
}

// TODO: symetric line of sight
int rl_array_can_see(array_t *a, int x1, int y1, int x2, int y2, VALUE blocking) {
  int x = x1, y = y1, has_next = 1;
  for(rl_walk_line_start(x1, y1, x2, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
    if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
      if(rl_array_value(a, x, y) == blocking) return 0;
    } else return 0;
  }
  //if(x1 == x2 && y1 == y2 && rl_array_value(a, x1, y1) != blocking) return 1;
  //if(x1 == x2 && y1 == y2) return 1;
  return 1;

  /*int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = x1 < x2 ? 1 : -1;
  int sy = y1 < y2 ? 1 : -1;
  int err = dx - dy;

  while(1) {
    if(x1 >= 0 && x1 < a->width && y1 >= 0 && y1 < a->height) {
      if(rl_array_value(a, x1, y1) == blocking) break;
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
  if(x1 == x2 && y1 == y2 && rl_array_value(a, x1, y1) != blocking) return 1;
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
          rl_array_value(result, x, y) = rl_array_can_see(a, xc, yc, x, y, blocking);
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
        if(distance > radius_sqr || rl_array_value(a, x, y) == blocking) {
          break;
        } else {
          rl_array_value(result, x, y) = 1;
        }
      } else break;
    }
    x = xc, y = yc, has_next = 1;
    for(rl_walk_line_start(xc, yc, x2, yc + radius); has_next; has_next = rl_walk_line_next(&x, &y)) {
      if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
        int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
        if(distance > radius_sqr || rl_array_value(a, x, y) == blocking) {
          break;
        } else {
          rl_array_value(result, x, y) = 1;
        }
      } else break;
    }
  }
  for(y2 = yc - radius; y2 <= yc + radius; y2++) {
    int x = xc, y = yc, has_next = 1;
    for(rl_walk_line_start(xc, yc, xc - radius, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
      if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
        int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
        if(distance > radius_sqr || rl_array_value(a, x, y) == blocking) {
          break;
        } else {
          rl_array_value(result, x, y) = 1;
        }
      } else break;
    }
    x = xc, y = yc, has_next = 1;
    for(rl_walk_line_start(xc, yc, xc + radius, y2); has_next; has_next = rl_walk_line_next(&x, &y)) {
      if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
        int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
        if(distance > radius_sqr || rl_array_value(a, x, y) == blocking) {
          break;
        } else {
          rl_array_value(result, x, y) = 1;
        }
      } else break;
    }
  }
  if(light_walls) {
    const int dx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    const int dy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    for(y = yc - radius; y <= yc + radius; y++) {
      for(x = xc - radius; x <= xc + radius; x++) {
        if(x >= 0 && x < result->width && y >= 0 && y < result->height && rl_array_value(a, x, y) == blocking) {
          int distance = (x - xc) * (x - xc) + (y - yc) * (y - yc);
          if(distance < radius * radius) {
            int sum = 0;
            for(int k = 0; k < 8; k++) {
              int i = x + dx[k];
              int j = y + dy[k];
              if(i >= 0 && i < result->width && j >= 0 && j < result->height) {
                if(rl_array_value(a, i, j) != blocking && rl_array_value(result, i, j)) {
                  sum++;
                  break;
                }
              }
            }
            if(sum > 0) rl_array_value(result, x, y) = 1;
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
  if(x1 == x2 && y1 == y2) return NULL;
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
        if(rl_array_value(a, next_x, next_y) != blocking && new_cost < cost_so_far[next]) {
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
    while(path_size > 0 && cost_so_far[current] != 0 && cost_so_far[current] != VALUE_MAX) {
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
        if(rl_array_value(a, x, y) <= 0) continue;
        VALUE min = VALUE_MAX;
        for(int i = 0; i < 8; i++) {
          if(x + offset_x[i] >= 0 && x + offset_x[i] < a->width
              && y + offset_y[i] >= 0 && y + offset_y[i] < a->height) {
            VALUE value = rl_array_value(a, x + offset_x[i], y + offset_y[i]);
            if(value >= 0 && value < min) {
              min = value;
            }
          }
        }
        if(min != INT_MAX && min + 1 < rl_array_value(a, x, y)) {
          rl_array_value(a, x, y) = min + 1;
          converged = 0;
        }
      }
    }
  } while(!converged);
}

VALUE rl_array_min(array_t *a, VALUE blocking) {
  VALUE min = VALUE_MAX;
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++) {
      VALUE value = rl_array_value(a, i, j);
      if(value != blocking && value < min) min = value;
    }
  return min;
}

VALUE rl_array_max(array_t* a, VALUE blocking) {
  VALUE max = VALUE_MIN;
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++) {
      VALUE value = rl_array_value(a, i, j);
      if(value != blocking && value > max) max = value;
    }
  return max;
}

point_t rl_array_argmin(array_t* a, VALUE blocking) {
  VALUE min = VALUE_MAX;
  int argmin_x = 0, argmin_y = 0;
  for(uint32_t y = 0; y < a->height; y++) {
    for(uint32_t x = 0; x < a->height; x++) {
      VALUE value = rl_array_value(a, x, y);
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
      VALUE value = rl_array_value(a, x, y);
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
    if(rl_array_value(a, x, y) == needle) {
      *rx = x;
      *ry = y;
      return 1;
    }
  }
  *rx = -1;
  *ry = -1;
  return 0;
}

array_t* rl_array_copy(array_t* src, array_t* mask) {
  if(mask == NULL) {
    if(src->is_view) {
      return rl_array_view(src, 0, 0, src->width, src->height);
    } else {
      array_t *dest = rl_array_new(src->width, src->height);
      for(int i = 0; i < src->width * src->height; i++) dest->values[i] = src->values[i];
      return dest;
    }
  } else {
    if(src->width != mask->width || src->height != mask->height)
      rl_error("size mismatch");
    array_t* dest = rl_array_new(src->width, src->height);
    for(int j = 0; j < src->height; j++)
      for(int i = 0; i < src->width; i++)
        if(mask->values[i]) rl_array_value(dest, i, j) = rl_array_value(src, i, j);
    return dest;
  }
}

void rl_array_copy_to(array_t* src, array_t* dest, array_t* mask) {
  if(mask == NULL) {
    if(src->width != dest->width || src->height != dest->height) 
      rl_error("size mismatch");
    for(int j = 0; j < src->height; j++)
      for(int i = 0; i < src->width; i++)
        rl_array_value(dest, i, j) = rl_array_value(src, i, j);
  } else {
    if(src->width != dest->width || src->width != mask->width || src->height != dest->height || src->height != mask->height) 
      rl_error("size mismatch");
    for(int j = 0; j < src->height; j++)
      for(int i = 0; i < src->width; i++)
        if(rl_array_value(mask, i, j)) rl_array_value(dest, i, j) = rl_array_value(src, i, j);
  }
}

void rl_array_cell_automaton(array_t* a, const char* definition, int warp) {
  VALUE value_dead = 0;
  VALUE value_alive = 1;
  VALUE value_will_die = 2;
  VALUE value_will_live = 3;
  char* birth_states = strchr(definition, 'B');
  char* survive_states = strchr(definition, 'S');
  if(birth_states == NULL) rl_error("invalid definition, no B specified");
  if(survive_states == NULL) rl_error("invalid definition, no S specified");
  int offset_x[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
  int offset_y[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
  for(int y = 0; y < a->height; y++) {
    for(int x = 0; x < a->width; x++) {
      int neighbors = 0;
      for(int k = 0; k < 8; k++) {
        int i = x + offset_x[k];
        int j = y + offset_y[k];
        if(warp) {
          i %= a->width;
          j %= a->height;
        }
        if(i < 0 || i >= a->width || j < 0 || j >= a->height) continue;
        VALUE value = rl_array_value(a, i, j);
        if(value == value_alive || value == value_will_die) neighbors++;
      }

      VALUE value = rl_array_value(a, x, y);
      if(value == value_dead) {
        for(char* c = birth_states + 1; *c && *c >= '0' && *c <= '8'; c++)
          if(neighbors == *c - '0') {
            value = value_will_live;
            break;
          }
      } else if(value == value_alive) {
        int found = 0;
        for(char* c = survive_states + 1; *c && *c >= '0' && *c <= '8'; c++)
          if(neighbors == *c - '0') {
            found = 1; 
            break;
          }
        if(!found) value = value_will_die;
      }
      rl_array_value(a, x, y) = value;
    }
  }
  for(int y = 0; y < a->height; y++) {
    for(int x = 0; x < a->width; x++) {
      VALUE value = rl_array_value(a, x, y);
      if(value == value_will_die) rl_array_value(a, x, y) = value_dead;
      else if(value == value_will_live) rl_array_value(a, x, y) = value_alive;
    }
  }
}

#define rl_array_right_op_value(name, operator) \
  array_t* rl_array_right_##name##_value(array_t* a, VALUE value) { \
    array_t *b = rl_array_new(a->width, a->height); \
    for(int j = 0; j < a->height; j++) \
      for(int i = 0; i < a->width; i++) \
        rl_array_value(b, i, j) = (rl_array_value(a, i, j) operator value); \
    return b; \
  }

#define rl_array_right_op_other(name, operator) \
  array_t* rl_array_right_##name##_other(array_t* a, array_t* other) { \
    if(a->width != other->width || a->height != other->height) rl_error("size mismatch"); \
    array_t *b = rl_array_new(a->width, a->height); \
    for(int j = 0; j < a->height; j++) \
      for(int i = 0; i < a->width; i++) \
        rl_array_value(b, i, j) = (rl_array_value(a, i, j) operator rl_array_value(other, i, j)); \
    return b; \
  }

#define rl_array_inplace_op_value(name, operator) \
  void rl_array_inplace_##name##_value(array_t* a, VALUE value) { \
    for(int j = 0; j < a->height; j++) \
      for(int i = 0; i < a->width; i++) \
        rl_array_value(a, i, j) = (rl_array_value(a, i, j) operator value); \
  }

#define rl_array_inplace_op_other(name, operator) \
  void rl_array_inplace_##name##_other(array_t* other, array_t* a) { \
    if(a->width != other->width || a->height != other->height) rl_error("size mismatch"); \
    for(int j = 0; j < a->height; j++) \
      for(int i = 0; i < a->width; i++) \
        rl_array_value(a, i, j) = (rl_array_value(a, i, j) operator rl_array_value(other, i, j)); \
  }

#define rl_array_left_op_value(name, operator) \
  array_t* rl_array_left_##name##_value(array_t* a, VALUE value) { \
    array_t *b = rl_array_new(a->width, a->height); \
    for(int j = 0; j < a->height; j++) \
      for(int i = 0; i < a->width; i++) \
        rl_array_value(b, i, j) = (value operator rl_array_value(a, i, j)); \
    return b; \
  }

#define rl_array_left_op_other(name, operator) \
  array_t* rl_array_left_##name##_other(array_t* other, array_t* a) { \
    if(a->width != other->width || a->height != other->height) rl_error("size mismatch"); \
    array_t *b = rl_array_new(a->width, a->height); \
    for(int j = 0; j < a->height; j++) \
      for(int i = 0; i < a->width; i++) \
        rl_array_value(b, i, j) = (rl_array_value(other, i, j) operator rl_array_value(a, i, j)); \
    return b; \
  }

#define rl_array_op_impl(name, operator) \
  rl_array_left_op_value(name, operator); \
  rl_array_left_op_other(name, operator); \
  rl_array_right_op_value(name, operator); \
  rl_array_right_op_other(name, operator); \
  rl_array_inplace_op_value(name, operator); \
  rl_array_inplace_op_other(name, operator);

rl_array_op_impl(equal, ==);
rl_array_op_impl(greater_than, >);
rl_array_op_impl(less_than, <);
rl_array_op_impl(greater_or_equal_than, >=);
rl_array_op_impl(less_or_equal_than, <=);
rl_array_op_impl(not_equal, !=);
rl_array_op_impl(and, &);
rl_array_op_impl(or, |);
rl_array_op_impl(xor, ^);
rl_array_op_impl(lshift, <<);
rl_array_op_impl(rshift, >>);
rl_array_op_impl(add, +);
rl_array_op_impl(sub, -);
rl_array_op_impl(mul, *);
rl_array_op_impl(div, /);
rl_array_op_impl(mod, %);

array_t* rl_array_unary_minus(array_t* a) {
  array_t *b = rl_array_new(a->width, a->height);
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(b, i, j) = -rl_array_value(a, i, j);
  return b;
}

array_t* rl_array_unary_not(array_t* a) {
  array_t *b = rl_array_new(a->width, a->height);
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(b, i, j) = !rl_array_value(a, i, j);
  return b;
}

array_t* rl_array_unary_invert(array_t* a) {
  array_t *b = rl_array_new(a->width, a->height);
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      rl_array_value(b, i, j) = ~rl_array_value(a, i, j);
  return b;
}

int rl_array_any_equals(array_t* a, VALUE value) {
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      if(rl_array_value(a, i, j) == value) return 1;
  return 0;
}

int rl_array_all_equal(array_t* a, VALUE value) {
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      if(rl_array_value(a, i, j) != value) return 0;
  return 1;
}

int rl_array_count(array_t* a, VALUE value) {
  int num = 0;
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      if(rl_array_value(a, i, j) == value) num++;
  return num;
}

uint64_t rl_array_sum(array_t* a) {
  uint64_t sum = 0;
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++)
      sum += rl_array_value(a, i, j);
  return sum;
}

array_t* rl_array_abs(array_t* a) {
  array_t* b = rl_array_new(a->width, a->height);
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < a->width; i++) {
      VALUE value = rl_array_value(a, i, j);
      rl_array_value(b, i, j) = (value > 0 ? value : -value);
    }
  return b;
}

// TODO: need a way to normalize for range of uint32_t
array_t* rl_array_matmul(array_t* a, array_t* b) {
  if(a->width != b->height) rl_error("size mismatch %d != %d", a->width, b->height);
  array_t* c = rl_array_new(b->width, a->height);
  for(int j = 0; j < a->height; j++)
    for(int i = 0; i < b->width; i++) {
      uint64_t sum = 0; 
      for(int k = 0; k < a->width; k++) 
        sum += rl_array_value(a, k, j) * rl_array_value(b, i, k);
      rl_array_value(c, i, j) = (VALUE) sum;
    }
  return b;
}

array_t* rl_array_apply_kernel(array_t* a, array_t* kernel) {
  array_t* result = rl_array_new(a->width, a->height);
  for(int j = 0; j < a->height; j++) {
    for(int i = 0; i < a->width; i++) {
      int64_t sum = 0;
      int num = 0;
      for(int k_j = 0; k_j < kernel->height; k_j++) {
        for(int k_i = 0; k_i < kernel->width; k_i++) {
          int x = i + k_i - kernel->width / 2;
          int y = j + k_j - kernel->height / 2;
          if(x >= 0 && x < a->width && y >= 0 && y < a->height) {
            VALUE value = rl_array_value(kernel, k_i, k_j);
            sum += rl_array_value(a, x, y) * value;
            num += abs(value);
          }
        }
      }
      if(num != 0) rl_array_value(result, i, j) = sum / num;
    }
  }
  return result;
}

int rl_array_flood_fill(array_t* a, int x, int y, VALUE fill_value, int use_diagonals) {
  const int offset_x[8] = {0, -1, 1, 0, -1, 1, -1, 1};
  const int offset_y[8] = {-1, 0, 0, 1, -1, -1, 1, 1};
  int offset_size = use_diagonals ? 8 : 4;
  
  if(x < 0 || x >= a->width || y < 0 || y >= a->height) 
    rl_error("out of bounds");
  VALUE initial_value = rl_array_value(a, x, y);
  int size = 0;
  point_t* stack = malloc(sizeof(point_t) * a->width * a->height);
  int stack_size = 1;
  stack[0].x = x;
  stack[0].y = y;
  rl_array_value(a, x, y) = fill_value;
  while(stack_size > 0) {
    stack_size--;
    x = stack[stack_size].x;
    y = stack[stack_size].y;
    size++;
    for(int k = 0; k < offset_size; k++) {
      int i = x + offset_x[k];
      int j = y + offset_y[k];
      if(i < 0 || i >= a->width || j < 0 || j >= a->height) continue;
      VALUE value = rl_array_value(a, i, j);
      if(value == initial_value) {
        rl_array_value(a, i, j) = fill_value;
        stack[stack_size].x = i;
        stack[stack_size].y = j;
        stack_size++;
      }
    }
  }
  free(stack);
  return size;
}
