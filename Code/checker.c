#include <stdio.h>
#include <stdlib.h>
#include "checker.h"
#include "game.h"
#include "counting.h"
#include "zobrist.h"
#include "utils.h"

static int const mv_row[4] = { 0, 1, 1, -1 };
static int const mv_col[4] = { 1, 0, 1,  1 };

// ����Ƿ��γ�����������forb = 0ʱ����鳤����
static inline int check_five(Game const* game, Pos const* pos, int forb)
{
	Lines res; Direction dir;
	count_all_dir(&res, game->board, pos->row, pos->col, game->current_player);
	int max_cnt = 0;
	for_all_dir(dir) max_cnt = MAX(max_cnt, res.hline[dir][0].len + res.hline[dir][1].len);
	if (max_cnt >= 5) return forb ? FORBIDDEN_PLAY : GAME_OVER; // �����cnt���������posλ�õ���
	else if (max_cnt == 4) return GAME_OVER;
	return NORMAL_PLAY;
}

// ���ߺ������������ظ������������漰����������С��ֱ��ʹ��O(n^2)�㷨���ɣ�
static inline int count_without_rep(uint64 const arr[], const int n) {
	if (n == 0) return 0;
	int cnt = 1;
	for (int i = 1; i < n; ++i) {
		for (int j = 0; j < i; ++j) {
			if (arr[i] == arr[j]) goto skip;
		}
		++cnt;
	skip:;
	}
	return cnt;
}

// �������������
static int check_three_four(Game const* game, Pos const* pos, int forb)
{
	uint64 hash_three[50], hash_four[50];
	int idx_three = 0, idx_four = 0;
	// ����ĳһ����ֱ�ӵ��µ�������ֻ������������
	Lines const* res; Direction dir; Pos adj_pos;
	int p;
	int len, blockage;
	for_all_dir(dir) {
		for (p = -1; p <= 1; p += 2) { // ���������෴����
			if (adjacent_blank(&adj_pos, game, pos->row, pos->col, p * mv_row[dir], p * mv_col[dir])) {
				res = &game->board_cnt[adj_pos.row][adj_pos.col][P2I(game->current_player)];
				len = res->hline[dir][0].len + res->hline[dir][1].len;
				blockage = res->hline[dir][0].blockage | res->hline[dir][1].blockage;
				if (len == 3 && blockage == 0) {
					hash_three[idx_three++] = res->hline[dir][0].hash ^ res->hline[dir][1].hash; // ͬһ�����������ڲ�ͬ�Ŀյ㱻�����ϣ������Ҫ��¼hashֵ���ȥ��
				}
				else if (len == 4) {
					hash_four[idx_four++] = res->hline[dir][0].hash ^ res->hline[dir][1].hash; // ͬ��
				}
			}
		}
	}
	int three_cnt = count_without_rep(hash_three, idx_three);
	int four_cnt = count_without_rep(hash_four, idx_four);
	idx_three = 0; idx_four = 0;
	if (forb && (four_cnt > 1 || three_cnt > 1)) return FORBIDDEN_PLAY;
	// ����ONE_STEP_THREAT��ָ����һ���Ϳ���ȡʤ���߷������ģ�����TWO_STEP_THREAT��ָ���������ſ���ȡʤ���߷���������
	if (four_cnt != 0) return ONE_STEP_THREAT;
	if (three_cnt != 0) return TWO_STEP_THREAT;
	return NORMAL_PLAY;
}

// forb��ʾ�Ƿ������
int check(Game const* game, Pos const* pos, int forb) {
	int state;
	state = check_five(game, pos, forb);
	if (state != NORMAL_PLAY) return state; // ����>����>���������
	state = check_three_four(game, pos, forb);
	return state;
}