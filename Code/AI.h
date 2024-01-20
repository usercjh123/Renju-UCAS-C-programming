#pragma once
#include "game.h"

#define LEVEL_EASY 0
#define LEVEL_MID 1
#define LEVEL_HARD 2

#define MAX_N_THREAD 12

Pos AI_play(Game* game, int const level, int const width, int const threshold, int output, int rnd);