// ��ģ��ʵ����alpha-beta��֦�Ż��˵�MIN-MAX������
#pragma once
#include "game.h"

#define MAX_WIDTH 18
// MIN/MAX ״̬
#define STRATEGY_MIN -1
#define STRATEGY_MAX 1

typedef struct Move {
	int val;
	Pos pos;
} Move;

// ���ڶ��߳�����ʱ���ݲ���
typedef struct SearchInfo {
	Game* game;
	Move* move;
	int level;
} SearchInfo;

void search(Game* game, Move* res, int stragety, int depth, int alpha, int beta, int max_depth);
int optimal_move(Game* game, int const width, int const level, int const threshold, Move* move, int const output);
void naive_strategy(Game* game, Move* res);