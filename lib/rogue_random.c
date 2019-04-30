#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "rogue_array.h"

static uint64_t random_state = 123;

// PCG generator
uint32_t rl_random_next() {
    uint64_t oldstate = random_state;
    random_state = oldstate * 6364136223846793005ULL + 1;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// static 1d-3d noise
uint32_t rl_random_3d(int32_t x, int32_t y, int32_t z) {
	uint64_t start = (x * 1619) ^ (y * 31337) ^ (z * 6971);
	uint64_t state = (start * 6364136223846793005ULL) + 1875;
	uint32_t xorshifted = ((state >> 18u) ^ state) >> 27u;
	uint32_t rot = state >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint32_t rl_random_2d(int32_t x, int32_t y) {
	return rl_random_3d(x, y, 0);
}

uint32_t rl_random_1d(int32_t x) {
	return rl_random_3d(x, 0, 0);
}

void rl_set_seed(uint64_t start) {
	if(start == 0) {
		/*struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		random_state = now.tv_nsec | 1;*/
		random_state = time(NULL) ^ random_state;
	} else {
		random_state = start;
	}
}

uint64_t rl_get_seed() {
	return random_state;
}

int rl_random_int(int a, int b) {
	if(b < a) return a;
	return ((rl_random_next() % (1 + b - a)) + a);
}

float rl_random() {
	return (float)(rl_random_next() & 0xffff) / 0xffff;
}

int rl_roll(const char* dice) {
	if(!strcmp(dice, "0")) { // special case for "0"
		return 0;
	}
	char* start = NULL;
	int rolls = strtol(dice, &start, 10);
	if (rolls <= 0 || (*start != 'd' && *start != 'D'))  {
		fprintf(stderr, "invalid dice definition '%s'\n", dice);
		return 0;
	}
	char* end = NULL;
	int faces = strtol(start + 1, &end, 10);
	if (faces <= 0 || end == NULL || *end != '\0') {
		fprintf(stderr, "invalid dice definition '%s'\n", dice);
		return 0;
	}
	int total = 0;
	for(int i = 0; i < rolls; i++) total += 1 + (rl_random_next() % faces);
	return total;
}

