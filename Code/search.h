// 该模块实现了alpha-beta剪枝优化了的MIN-MAX搜索树
#pragma once
#include "game.h"

#define MAX_WIDTH 18
// MIN/MAX 状态
#define STRATEGY_MIN -1
#define STRATEGY_MAX 1

typedef struct Move {
	int val;
	Pos pos;
} Move;

// 用于多线程搜索时传递参数
typedef struct SearchInfo {
	Game* game;
	Move* move;
	int level;
} SearchInfo;

void search(Game* game, Move* res, int stragety, int depth, int alpha, int beta, int max_depth);
int optimal_move(Game* game, int const width, int const level, int const threshold, Move* move, int const output);
void naive_strategy(Game* game, Move* res);