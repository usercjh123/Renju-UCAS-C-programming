#include <stdio.h>
#include "game.h"

static Pos move_lst[BOARD_SIZE * BOARD_SIZE];
static int rnd = 0;

void record(Pos const* pos) {
	move_lst[rnd++] = *pos;
}

int save(char* filedir) {
	FILE* fp = fopen(filedir, "w");
	if (fp == NULL) return -1;
	fprintf(fp, "%d\n%d\n%d\n", BOARD_SIZE, BOARD_SIZE, rnd);
	for (int i = 0; i < rnd; ++i) {
		fprintf(fp, "%d %d\n", move_lst[i].row, move_lst[i].col);
	}
	fclose(fp);
	return 0;
}

