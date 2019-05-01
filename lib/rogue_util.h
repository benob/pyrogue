#ifndef __ROGUE_UTIL_H__
#define __ROGUE_UTIL_H__

#include <stdarg.h>
#include <stdio.h>

typedef void (*error_handler_t)(const char* message);

#define rl_error(format, ...) { char buffer[1024]; const char* name = strrchr(__FILE__, '/'); if(!name) { name = strrchr(__FILE__, '\\'); if(!name) name = __FILE__; else name++; } else name++; snprintf(buffer, 1024, "%s:%d " format, name, __LINE__, ##__VA_ARGS__); _rl_current_error_handler(buffer); }

void rl_set_error_handler(error_handler_t error_handler);
extern error_handler_t _rl_current_error_handler; // do not call directly

// interop with a GC controlled memory pool
// warning: all pointers to this memory need to be stored in memory owned by the GC, otherwise it will free stuff that it does not see having a pointer to
void rl_set_allocator(void* (*malloc)(size_t size), void* (*realloc)(void* mem, size_t old_size, size_t new_size), void (*free)(void* mem, size_t size));
extern void* (*rl_malloc)(size_t size);
extern void* (*rl_realloc)(void* mem, size_t old_size, size_t new_size);
extern void (*rl_free)(void* mem, size_t size);

#endif
