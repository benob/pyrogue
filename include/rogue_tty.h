#ifndef __ROGUE_TTY_H__
#define __ROGUE_TTY_H__

#include "rogue_array.h"

#if defined(__MINGW32__) || defined(__MINGW64__)
#warning tty functions disabled on windows
#else

enum {TTY_LEFT=256, TTY_RIGHT, TTY_UP, TTY_DOWN, TTY_RETURN=10, TTY_ESCAPE=27};

void tty_init();
void tty_clear();
void tty_move(int x, int y);
void tty_put(const char* text, const char* fg, const char* bg);
int tty_wait_key();

#endif

void tty_print_array(array_t *a, const char *mapping, const char *fg, const char* bg);


#endif
