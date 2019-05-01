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

static void* _realloc_wrapper(void* mem, size_t old_size, size_t new_size) {
	return realloc(mem, new_size);
}

static void _free_wrapper(void* mem, size_t size) {
	free(mem);
}

void rl_set_allocator(void* (*malloc_in)(size_t size), void* (*realloc_in)(void* mem, size_t old_size, size_t new_size), void (*free_in)(void* mem, size_t size)) {
	if(malloc_in != NULL) {
		rl_malloc = malloc_in;
		rl_realloc = realloc_in;
		rl_free = free_in;
	} else {
		printf("unset allocator\n");
		rl_malloc = malloc;
		rl_realloc = _realloc_wrapper;
		rl_free = _free_wrapper;
	}
}

void* (*rl_malloc)(size_t size) = malloc;
void* (*rl_realloc)(void* mem, size_t old_size, size_t new_size) = _realloc_wrapper;
void (*rl_free)(void* mem, size_t size) = _free_wrapper;
