#include <time.h>
#include <stdio.h>
#include "AI.h"
#include "game.h"
#include "VCX.h"
#include "search.h"
#include "utils.h"

// output �����Ƿ����������Ϣ
Pos AI_play(Game* game, int const level, int const width, int const threshold, int output, int rnd) {
	clock_t st, ed;
	Pos attack_pos; Move move;
	if (rnd == 0) {
		SET_POS(attack_pos, 7, 7);
		return attack_pos;
	}
	int state;
	st = clock();
	state = multithreading_attack(game, &attack_pos, level, output);
	ed = clock();
	if (output) printf("VCX������ʱ��%f\n", 1.0 * (ed - st) / CLOCKS_PER_SEC);
	if (state) {
		if (output) printf("VCX�����ҵ��������ԣ�%c%d\n", 'A' + attack_pos.col, 15 - attack_pos.row);
		return attack_pos;
	}
	st = clock();
	if(rnd <= 4) state = optimal_move(game, width, 1, threshold, &move, output);
	else state = optimal_move(game, width, level, threshold, &move, output);
	ed = clock();
	if (output) printf("Min-Max������ʱ��%f\n", 1.0 * (ed - st) / CLOCKS_PER_SEC);
	if (state == 1) {
		if (output) printf("Min-Max�������Ų���: %c%d\n", 'A' + move.pos.col, 15 - move.pos.row);
		return move.pos;
	}
	else {
		if (output) printf("Min-Max����δ�ҵ����Ų���\n");
		if (state == 0) return move.pos;
		else naive_strategy(game, &move);
		return move.pos;
	}
}