// 该模块是游戏接口
#pragma once
#include "mt19937.h"

#define BOARD_SIZE 15 // 棋盘大小

#define BLACK 1
#define WHITE -1

// 游戏状态常量
#define NORMAL_PLAY 0
#define FORBIDDEN_PLAY 1
#define TWO_STEP_THREAT 2
#define ONE_STEP_THREAT 3
#define GAME_OVER 4
#define OCCUPATION 5

typedef struct Pos {
	int row, col;
} Pos;

typedef struct HalfLine {
	int len;
	int blockage;
	uint64 hash; // 用于记录数出的连子的Zobrist hash值
} HalfLine;

typedef struct Lines {
	HalfLine hline[4][2]; // 第一个下标代表四条线，第二个下标代表具体方向
} Lines;

typedef struct Game {
	// 游戏状态信息
	int board[BOARD_SIZE][BOARD_SIZE];
	Pos recent_play;
	int current_player;
	uint64 board_hash;
	// 游戏评估信息
	Lines board_cnt[BOARD_SIZE][BOARD_SIZE][2];
} Game;

void initialize_game(Game* game);
int play_at(Game* game, Pos const* pos);