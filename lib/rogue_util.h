#ifndef __ROGUE_UTIL_H__
#define __ROGUE_UTIL_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef void (*error_handler_t)(const char* message);
extern char _error_buffer[1024];

#define rl_error(format, ...) { const char* name = strrchr(__FILE__, '/'); if(!name) { name = strrchr(__FILE__, '\\'); if(!name) name = __FILE__; else name++; } else name++; snprintf(_error_buffer, 1024, "%s:%d " format, name, __LINE__, ##__VA_ARGS__); _rl_current_error_handler(_error_buffer); }

void rl_set_error_handler(error_handler_t error_handler);
extern error_handler_t _rl_current_error_handler; // do not call directly

// interop with a GC controlled memory pool
// warning: all pointers to this memory need to be stored in memory owned by the GC, otherwise it will free stuff that it does not see having a pointer to
void rl_set_allocator(void* (*malloc)(size_t size), void* (*realloc)(void* mem, size_t old_size, size_t new_size), void (*free)(void* mem, size_t size));
extern void* (*rl_malloc)(size_t size);
extern void* (*rl_realloc)(void* mem, size_t old_size, size_t new_size);
extern void (*rl_free)(void* mem, size_t size);

float rl_distance(float x1, float y1, float x2, float y2);
void rl_walk_line_start(int x1, int y1, int x2, int y2);
int rl_walk_line_next(int *x, int *y);

#endif
