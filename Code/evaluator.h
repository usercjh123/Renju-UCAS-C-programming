// 该模块用于评分
#pragma once
#include "game.h"

#define SCORE_GAME_OVER 10000

int eval_score(Game const* game, int player); // 获取局面评分
void scoring_board(Game const* game, int score_map[2][BOARD_SIZE][BOARD_SIZE]); // 为棋盘上的所有空点启发式打分