#include <stdio.h>
#include <stdlib.h>

#include "rogue_util.h"

static void default_error_handler(const char* message) {
	fprintf(stderr, "ERROR: %s\n", message);
	exit(1);
}

error_handler_t _rl_current_error_handler = default_error_handler;

void rl_set_error_handler(error_handler_t error_handler) {
	if(error_handler == NULL) _rl_current_error_handler = default_error_handler;
	else _rl_current_error_handler = error_handler;
}
