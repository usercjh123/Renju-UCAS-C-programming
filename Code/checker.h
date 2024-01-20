// 该模块用于判断胜利与禁手
#pragma once
#include "game.h"

// 返回值：NORMAL_PLAY（无威胁）, FORBIDDEN_PLAY（禁手）, ONE/TWO_THREAT_PLAY（有威胁）, GAME_OVER（player胜利）
int check(Game const* game, Pos const* pos, int forb);