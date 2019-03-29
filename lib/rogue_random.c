#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rogue_array.h"

// MINSTD3 Lehmer generator
static uint32_t random_state = 123;

uint32_t rl_random_next() {
  uint64_t state = random_state;
  state *= 69621;
  random_state = state % 2147483647;
  return random_state;
}

uint32_t rl_set_seed(uint32_t start) {
	if(start == 0) {
		/*struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		random_state = now.tv_nsec | 1;*/
		random_state = time(NULL);
	} else {
		random_state = start;
	}
	return random_state;
}

uint32_t rl_get_seed() {
	return random_state;
}

int rl_random_int(int a, int b) {
	if(b < a) return a;
	return ((rl_random_next() % (1 + b - a)) + a);
}

float rl_random() {
	return (float)rl_random_next() / UINT_MAX;
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

