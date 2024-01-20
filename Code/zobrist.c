#include <time.h>
#include "zobrist.h"
#include "mt19937.h"
#include "game.h"

static uint64 zobrist_black[BOARD_SIZE][BOARD_SIZE];
static uint64 zobrist_white[BOARD_SIZE][BOARD_SIZE];

void zobrist_initialize()
{
	//mt_initialize((uint64)time(0));
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			zobrist_black[row][col] = mt_rand();
			zobrist_white[row][col] = mt_rand();
		}
	}
}

void zobrist_change(uint64* hash, int r, int c, int player)
{
	*hash ^= (player == BLACK) ? zobrist_black[r][c] : zobrist_white[r][c];
}

