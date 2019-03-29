#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "rogue_tty.h"

static int hex2int(char ch) {
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	return 7;
}

static int palette[16] = {
	30, 31, 32, 33, 34, 35, 36, 37, 90, 91, 92, 93, 94, 95, 96, 97
};

void tty_clear() {
	printf("\33[H\33[2J");
}

void tty_move(int x, int y) {
	printf("\33[%d;%dH", y, x);
}

void tty_put(const char* text, const char* fg, const char* bg) {
	if(bg) printf("\33[%dm", 10 + palette[hex2int(bg[0])]);
	if(fg) printf("\33[%dm", palette[hex2int(fg[0])]);
	printf("%s\33[m", text);
}

int tty_wait_key() {
	int c = getchar();
	if(c == 27) {
		c = getchar();
		if(c == '[') {
			c = getchar();
			if(c == 'D') return TTY_LEFT;
			else if(c == 'C') return TTY_RIGHT;
			else if(c == 'A') return TTY_UP;
			else if(c == 'B') return TTY_DOWN;
			else return TTY_ESCAPE;
		} 
	} else if(c == 10) return TTY_RETURN;
	return c;
}

static struct termios orig_termios;

static void restore_terminal() {
	printf("\33[?25h");
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void handle_signal(int sig) {
	exit(sig);
}

static int needs_terminal_restore = 0;
static void __attribute__((destructor)) _fini() {
	if(needs_terminal_restore) restore_terminal();
}

void tty_init() {
	needs_terminal_restore = 1;
	struct termios new_termios;
	tcgetattr(STDIN_FILENO, &orig_termios);
	new_termios = orig_termios;
  new_termios.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
	signal(SIGINT, handle_signal);
	printf("\33[?25l");
}

void tty_print_array(array_t *a, const char *mapping, const char *fg, const char* bg) {
	int max_mapping = mapping ? strlen(mapping) : 0;
	int max_fg = fg ? strlen(fg) : 0;
	int max_bg = bg ? strlen(bg) : 0;
  for(int y = 0; y < a->height; y++) {
    for(int x = 0; x < a->width; x++) {
			int value = (int) rl_array_get(a, x, y);
			if(value >= 0 && max_fg > 0) printf("\33[%dm", palette[hex2int(fg[value % max_fg])]);
			if(value >= 0 && max_bg > 0) printf("\33[%dm", 10 + palette[hex2int(bg[value % max_bg])]);
			if(value >= 0 && max_mapping > 0) printf("%c", mapping[value % max_mapping]);
			else printf(" ");
			printf("\33[m");
    }
    printf("\n");
  }
}

