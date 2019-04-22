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

/* MINSTD3 Lehmer generator
uint32_t rl_random_next() {
  uint64_t state = random_state;
  state *= 69621;
  random_state = state % 2147483647;
  return random_state;
}*/

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

