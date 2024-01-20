// 该模块用于在棋盘上数子
#pragma once
#include "game.h"

typedef struct Game Game; // 解决超前依赖

typedef enum Direction {
	HORZ, VERT, pDIAG, sDIAG
	// pDIAG : 主对角线（左上--右下）
	// sDIAG : 副对角线（左下--右上）
} Direction;

// 实现数子功能的基本函数
void count_all_dir(Lines* res, int const board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player); // 对某点所在的四条线上的棋子进行计数
int adjacent_blank(Pos* pos, Game const* game, int row, int col, int mv_r, int mv_c);
// 实验表明，单次add/remove约耗时1e-7s量级
void add_and_recount(Game* game, int row, int col, int player); // 增加一个棋子，并重新评估各个点
void del_and_recount(Game* game, int row, int col); // 移除棋子
