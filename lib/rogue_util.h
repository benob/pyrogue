#ifndef __ROGUE_UTIL_H__
#define __ROGUE_UTIL_H__

#include <stdarg.h>
#include <stdio.h>

typedef void (*error_handler_t)(const char* message);

#define rl_error(format, ...) { char buffer[1024]; const char* name = strrchr(__FILE__, '/'); if(!name) { name = strrchr(__FILE__, '\\'); if(!name) name = __FILE__; else name++; } else name++; snprintf(buffer, 1024, "%s:%d " format, name, __LINE__, ##__VA_ARGS__); _rl_current_error_handler(buffer); }

void rl_set_error_handler(error_handler_t error_handler);
extern error_handler_t _rl_current_error_handler; // do not call directly

#endif
