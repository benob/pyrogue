#ifndef __ROGUE_RANDOM_H__
#define __ROGUE_RANDOM_H__

uint32_t rl_random_next();
int rl_random_int(int a, int b);
float rl_random();
int rl_roll(const char* dice);
void rl_set_seed(uint32_t start);
uint64_t rl_get_seed();

#endif
