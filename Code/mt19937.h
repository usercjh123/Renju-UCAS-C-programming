// ��ģ��ʵ����÷ɭ��ת�㷨mt19937_64�������ɸ�������64λ�����������Zobrist�㷨�Լ��������㷨��
// https://en.wikipedia.org/wiki/Mersenne_Twister
#pragma once

typedef unsigned long long uint64; // ����64λ�޷�������
typedef unsigned int uint;

#define MT_w 64
#define MT_n 312
#define MT_m 156
#define MT_r 31
#define MT_a 0xb5026f5aa96619e9
#define MT_f 6364136223846793005
#define MT_u 29
#define MT_d 0x5555555555555555
#define MT_s 17
#define MT_b 0x71d67fffeda60000
#define MT_t 37
#define MT_c 0xfff7eee000000000
#define MT_l 43
#define MT_LOWER_MASK ((1ull << MT_r) - 1)
#define MT_UPPER_MASK (1ull << MT_r)

void mt_initialize(uint64 seed);
uint64 mt_rand();