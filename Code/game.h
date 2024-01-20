// ��ģ������Ϸ�ӿ�
#pragma once
#include "mt19937.h"

#define BOARD_SIZE 15 // ���̴�С

#define BLACK 1
#define WHITE -1

// ��Ϸ״̬����
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
	uint64 hash; // ���ڼ�¼���������ӵ�Zobrist hashֵ
} HalfLine;

typedef struct Lines {
	HalfLine hline[4][2]; // ��һ���±���������ߣ��ڶ����±������巽��
} Lines;

typedef struct Game {
	// ��Ϸ״̬��Ϣ
	int board[BOARD_SIZE][BOARD_SIZE];
	Pos recent_play;
	int current_player;
	uint64 board_hash;
	// ��Ϸ������Ϣ
	Lines board_cnt[BOARD_SIZE][BOARD_SIZE][2];
} Game;

void initialize_game(Game* game);
int play_at(Game* game, Pos const* pos);