// 该模块实现了Zobrist hash算法
// https://en.wikipedia.org/wiki/Zobrist_hashing
#pragma once
#include "mt19937.h"

void zobrist_initialize();
void zobrist_change(uint64* hash, int r, int c, int player);