#include <stdio.h>
#include <windows.h>
#include <process.h>
#include "VCX.h"
#include "AI.h"
#include "checker.h"
#include "counting.h"
#include "evaluator.h"
#include "game.h"
#include "utils.h"

// һЩ��������С
#define MAX_N_THREAT 15
#define MAX_N_DEFENCE 36
#define MAX_N_ATTACK_POS 36

// �����ɹ���ʧ�ܷ���ֵ
#define SUCCESS 1
#define FAIL 0

// �ȼ���������ȶ��ձ�
static int level2depth[3] = { 8, 10, 12 };

static int const mv_row[4] = { 0, 1, 1, -1 };
static int const mv_col[4] = { 1, 0, 1,  1 };

static Game new_game[MAX_N_THREAD]; // ���߳������ڴ�

// ���ߺ������ж�ĳ�����Ƿ񹹳���в
static inline int is_dir_threat(Game const* game, Pos const* pos, Direction dir, int player) {
	Lines const* res;
	int len, blockage;
	res = &game->board_cnt[pos->row][pos->col][P2I(player)];
	len = res->hline[dir][0].len + res->hline[dir][1].len;
	blockage = res->hline[dir][0].blockage | res->hline[dir][1].blockage;
	if ((len == 3 && blockage == 0) || len == 4) return 1;
	return 0;
}

// defend�ĸ������������ڼ��һ����в���Ƿ���Ȼ������в
static inline int is_threat(Game const* game, Pos const* pos) {
	if (game->board[pos->row][pos->col] != 0) return 0;
	Direction dir;
	for_all_dir(dir) {
		if (is_dir_threat(game, pos, dir, -game->current_player)) return 1;
	}
	return 0;
}

// ���������в���Ƿ񻹹�����в
static inline int check_defence(Game const* game, Pos const threat_pos[MAX_N_THREAT], int n_threat_pos) {
	for (int i = 0; i < n_threat_pos; ++i) {
		if (is_threat(game, &threat_pos[i])) return 0;
	}
	return 1;
}

// ���������Ƿ������в����������в���ͺ���в��
// �������VCX�����ĺ���
static inline void find_threat_pos(Game const* game, Pos const* pos, Pos threat_pos[MAX_N_THREAT], int* n_threat_pos) {
	/*
	���ĳһ��������в��threat_pos�н������һ���ġ���в�㡱
	���磬���������(w->��, b->��, 0->��)�У����幹���˻�����Ҳ��������һ�����γɻ���
	w0bbb00w
	12345678
	��ֻ����6λ������һ�������γɻ��ģ����º���ʤ�������6λ�þ���������εġ���в�㡱��threat_pos�����6λ��
	��ʱ�������ϣ��������ֻ��Ҫʹ��6λ�ò��ٹ�����в�㣨����6λ������һ�������γɻ��ģ�����˿�����2,6,7λ�ã����Բ�������6λ�ã�����˰������ķ�����ʽ��
	���磬������ĺ�����������У���в����3��7
	w00bbb00w
	123456789
	��������������Ҫʹ��������в��ͬʱ��ʧ�����������8��2λ��ֻ��ʹ��һ��λ�õ���в����ʧ�������Ҫ����3��7λ��
	���磬������ĺ�����������У���в����6��������һ���γ��壩������Ϊ��ʹ����в����ʧ��ֻ������6λ��
	wbbbb00w
	12345678
	*/
	*n_threat_pos = 0;
	// ����Ĵ����checker.c�е�check_three_four���������˼򵥵��޸�
	Direction dir; Pos adj_pos;
	int p;
	for_all_dir(dir) {
		for (p = -1; p <= 1; p += 2) {
			if (adjacent_blank(&adj_pos, game, pos->row, pos->col, p * mv_row[dir], p * mv_col[dir])) {
				if (is_dir_threat(game, &adj_pos, dir, game->current_player)) {
					threat_pos[*n_threat_pos] = adj_pos;
					++(*n_threat_pos);
				}
			}
		}
	}
}

// ����ܹ��ɹ�����������1�����򷵻�0
static int defend(Game* game, int depth, Pos const threat_pos[MAX_N_THREAT], int const n_threat_pos, int const threat_type, int const max_depth) {
	int state;
	Pos attack_pos, nxt;
	Pos defence[MAX_N_DEFENCE]; int n_defence = 0;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) continue;
			add_and_recount(game, row, col, game->current_player);
			// �������ֵ�
			nxt.row = row; nxt.col = col;
			state = check(game, &nxt, game->current_player == BLACK);
			if (state == FORBIDDEN_PLAY) goto next;
			// ���ĳ�������Լ���ʤ��Ҳ�ǳɹ�����
			// ���ڽ������Ѿ�������һ����в�����ֵķ��ط�����ͨ�����������أ�ֻ��ͨ������һ��������в�ſ��ܴ��Ŀ��
			// Ϊ�˼�������������Ϊ�ڷ��ط�������һ��������в�ҽ���������������һ��������вʱ������������ʧ�ܣ���Ϊ��ʱ���������ò�ͣ�������أ�
			// �ڱ����У�ò�Ʒ��������и�bug�����ڶԷ�����ʱ�����жϲ��������������Ǳ��������ˣ���֪������Ϊʲô��bug
			else if (state == GAME_OVER || (state == ONE_STEP_THREAT && threat_type == TWO_STEP_THREAT)) {
				//printf("VCX: break\n");
				del_and_recount(game, row, col);
				return SUCCESS;
			}
			else if (check_defence(game, threat_pos, n_threat_pos)) {
				defence[n_defence] = nxt;
				++n_defence;
			}
		next:del_and_recount(game, row, col);
		}
	}
	for (--n_defence; n_defence >= 0; --n_defence) {
		add_and_recount(game, defence[n_defence].row, defence[n_defence].col, game->current_player);
		game->current_player *= -1;
		state = attack(game, depth + 1, &attack_pos, max_depth);
		game->current_player *= -1;
		del_and_recount(game, defence[n_defence].row, defence[n_defence].col);
		if (!state) return SUCCESS;
	}
	return FAIL;
}

// ���ýṹ�崫�ݶ����
static void attack_at(void* arg) {
	AttackInfo* info = (AttackInfo*)arg;
	Pos threat_pos[MAX_N_THREAT]; int n_threat_pos;
	find_threat_pos(info->game, &(info->attack_pos), threat_pos, &n_threat_pos); // �õ㹹�������ģ��Ҳ���Ҷ�ӽڵ㣬����find_threat_pos�����ҳ���в��
	info->game->current_player *= -1; // ������һ���״̬��������һ������(defend)
	info->state = defend(info->game, (info->depth) + 1, threat_pos, n_threat_pos, info->threat_type, info->max_depth);
	info->game->current_player *= -1; // ���ر���
}

// branching-factor ���ԼΪ10���кܶ�0
// ����ܹ��ɹ�����������1�����򷵻�0
int attack(Game* game, int depth, Pos* attack_pos, int const max_depth) {
	//printf("%d\n", depth);
	Pos nxt; int state;
	AttackInfo info;
	int score_map[2][BOARD_SIZE][BOARD_SIZE]; scoring_board(game, score_map);
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			// �ڲ����ϴ�ʱ������չȫ�������ڵ㣨���������ռ䣩
			if (game->board[row][col] != 0 || (depth > 0 && score_map[P2I(game->current_player)][row][col] < 30)) continue;
			// ��������в�����ֵ�
			add_and_recount(game, row, col, game->current_player);
			nxt.row = row; nxt.col = col;
			state = check(game, &nxt, game->current_player == BLACK);
			if (state == FORBIDDEN_PLAY || state == NORMAL_PLAY) goto next; // ������һ��������
			if (state == GAME_OVER) { // ������ʤ��
				del_and_recount(game, row, col);
				*attack_pos = nxt; // ��¼��ʤ�߷�
				return SUCCESS;
			}
			if (depth == max_depth) goto next; // Ҷ�ӽڵ㣬����������
			info.game = game;
			SET_POS(info.attack_pos, row, col); 
			info.depth = depth; info.threat_type = state; info.max_depth = max_depth;
			attack_at((void*)&info);
			if (!(info.state)) { // ����ý������޷��ɹ�������������ɹ�
				del_and_recount(game, row, col);
				*attack_pos = nxt;
				return SUCCESS;
			}
		next:del_and_recount(game, row, col);
		}
	}
	return FAIL;
}

int multithreading_attack(Game* game, Pos* attack_pos, int const level, int const output)
{
	Pos nxt; int state;
	int score_map[2][BOARD_SIZE][BOARD_SIZE]; scoring_board(game, score_map);
	AttackInfo info[MAX_N_ATTACK_POS]; HANDLE threads[MAX_N_THREAD]; unsigned int threadID[MAX_N_THREAD];
	int n = 0, i = 0;
	// ��attack����˼·��ͬ�����������¼���˿��ܵĹ���λ��
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) continue;
			add_and_recount(game, row, col, game->current_player);
			nxt.row = row; nxt.col = col;
			state = check(game, &nxt, game->current_player == BLACK);
			if (state == FORBIDDEN_PLAY || state == NORMAL_PLAY) goto next;
			if (state == GAME_OVER) {
				del_and_recount(game, row, col);
				*attack_pos = nxt;
				return SUCCESS;
			}
			SET_POS(info[n].attack_pos, row, col);
			info[n].depth = 0; info[n].threat_type = state; info[n].max_depth = level2depth[level];
			++n;
		next:del_and_recount(game, row, col);
		}
	}
	if (n == 0) return FAIL;
	if(output) printf("VCX������������λ�ã�");
	do {
		// ��������ջ���������ȷ�������Ƿ��ȶ�
		int n_thread;
		for (n_thread = 0; n_thread < MAX_N_THREAD && i < n; ++n_thread) {
			new_game[n_thread] = *game; // ��״̬��Ϣ����һ�飬��ֹ�̳߳�ͻ
			info[i].game = &new_game[n_thread];
			add_and_recount(info[i].game, info[i].attack_pos.row, info[i].attack_pos.col, info[i].game->current_player);
			threads[n_thread] = (HANDLE)_beginthreadex(NULL, 0, attack_at, (void*)&info[i], 0, &threadID[n_thread]);
			if(output) printf("%c%d ", 'A' + info[i].attack_pos.col, 15 - info[i].attack_pos.row);
			++i;
		}
		for (int j = 0; j < n_thread; ++j) WaitForSingleObject(threads[j], INFINITE);
		if(output) printf("\n");
		for (; n_thread > 0; --n_thread) {
			if (!info[i - n_thread].state) {
				*attack_pos = info[i - n_thread].attack_pos;
				return SUCCESS;
			}
		}
	} while (n - i > 0);
	return FAIL;
}