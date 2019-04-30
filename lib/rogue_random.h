#ifndef __ROGUE_RANDOM_H__
#define __ROGUE_RANDOM_H__

uint32_t rl_random_next();
int rl_random_int(int a, int b);
uint32_t rl_random_3d(int32_t x, int32_t y, int32_t z);
uint32_t rl_random_2d(int32_t x, int32_t y);
uint32_t rl_random_1d(int32_t x);
float rl_random();
int rl_roll(const char* dice);
void rl_set_seed(uint64_t start);
uint64_t rl_get_seed();

#endif
