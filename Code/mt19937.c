#include "mt19937.h"

static uint64 mt[MT_n];
static uint index;

void mt_initialize(uint64 seed)
{
	mt[0] = seed;
	for (uint i = 1; i < MT_n; ++i) {
		mt[i] = MT_f * (mt[i - 1] ^ (mt[i - 1] >> (MT_w - 2))) + i;
	}
	index = MT_n;
}

static void twist()
{
    uint64 x, y;
    for (uint i = 0; i < MT_n; ++i) {
        x = (uint64)((mt[i] & MT_UPPER_MASK) + (mt[(i + 1) % MT_n] & MT_LOWER_MASK));
        y = (x >> 1);
        if (x & 1) y = (y ^ MT_a);
        mt[i] = (mt[(i + MT_m) % MT_n] ^ y);
    }
    index = 0;
}

uint64 mt_rand()
{
    uint64 x;
    if (index >= MT_n) twist();
    uint i = index++;
    x = mt[i];
    x ^= (x >> MT_u) & MT_d;
    x ^= (x << MT_s) & MT_b;
    x ^= (x << MT_t) & MT_c;
    x ^= (x >> MT_l);
    return x;
}