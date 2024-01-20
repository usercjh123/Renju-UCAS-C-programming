#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <process.h>
#include "search.h"
#include "AI.h"
#include "game.h"
#include "counting.h"
#include "evaluator.h"
#include "checker.h"
#include "utils.h"

#define LST_MAX_LEN (BOARD_SIZE * BOARD_SIZE)
// MAX���MIN��������ӷ���
// ������ֵ�ĺ���������Min-Max�����������ӷ���
#define DEFENCE_MAX 0.9
#define DEFENCE_MIN 0.7

#define ROOT_PLAYER(strategy, player) ((strategy) == STRATEGY_MAX ? (player) : -(player))
#define SCORE(r, c, p, s) (int)(score_map[P2I((p))][r][c] + ceil(((s) == STRATEGY_MIN ? DEFENCE_MIN : DEFENCE_MAX) * score_map[P2I(-(p))][r][c]))

static Game new_game[MAX_N_THREAD]; // ���߳������ڴ�

// �ȼ���������ȶ��ձ�
static int const max_width[3][12] = {
	{16, 16, 16, 16, 12, 12, 0, 0, 0, 0, 0, 0}, // LEVEL_EASY & Ԥ����
	{16, 16, 16, 16, 12, 12, 12, 12, 0, 0, 0, 0}, // LEVEL_MID
	{16, 16, 16, 16, 12, 12, 12, 12, 9, 9, 0, 0} // LEVEL_HARD
};
// �ȼ���������ȶ��ձ�
static int const max_depth[3] = { 6, 8, 10 };

static int move_cmp(const void* pmove1, const void* pmove2) {
	return ((*(Move*)pmove2).val - (*(Move*)pmove1).val);
}

// ����ĳ��״̬�����ɽ����������ӷ����������ǽ��֣�
static inline int generate_move(Game const* game, Pos move[MAX_WIDTH], int const width, int const strategy) {
	Move move_lst[LST_MAX_LEN]; int n_move = 0;
	int score_map[2][BOARD_SIZE][BOARD_SIZE];
	scoring_board(game, score_map);
	int val;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) continue;
			if ((val = SCORE(row, col, game->current_player, strategy)) != 0) {
				SET_POS(move_lst[n_move].pos, row, col);
				move_lst[n_move].val = val;
				++n_move;
			}
		}
	}
	qsort(move_lst, n_move, sizeof(Move), move_cmp);
	n_move = MIN(n_move, width);
	for (int i = 0; i < n_move; ++i) move[i] = move_lst[i].pos;
	return n_move;
}

// alpha: ��ǰ���棨���ڵ㣩���ٻ�õķ���
// beta: ��ǰ���棨���ڵ㣩�����õķ���
void search(Game* game, Move* res, int strategy, int depth, int alpha, int beta, int const level) {
	if (depth == max_depth[level]) { // ��Ҷ�ӽڵ�ֱ�ӷ���evalֵ
		// Ҷ�ӽڵ㵱ǰ����Ǽ���
		res->val = eval_score(game, ROOT_PLAYER(strategy, game->current_player));
		return;
	}
	int state;
	res->val = (strategy == STRATEGY_MAX ? -INF : INF);
	Pos available_move[MAX_WIDTH];
	Move nxt;
	int n_child = generate_move(game, available_move, max_width[level][depth], strategy); // ���ɿ����߷��������ǽ���
	for (int i = 0; i < n_child; ++i) {
		// ������һ���״̬��Ϊ������һ����׼��
		add_and_recount(game, available_move[i].row, available_move[i].col, game->current_player);
		state = check(game, &available_move[i], game->current_player == BLACK);
		switch (state) {
		case GAME_OVER:
			del_and_recount(game, available_move[i].row, available_move[i].col);
			res->val = (strategy == STRATEGY_MAX ? SCORE_GAME_OVER : -SCORE_GAME_OVER);
			res->pos = available_move[i];
			return;
		case FORBIDDEN_PLAY: // ��������
			del_and_recount(game, available_move[i].row, available_move[i].col);
			continue; // �����Ǽ�������forѭ��
		default:
			// ������һ��
			game->current_player *= -1;
			search(game, &nxt, -strategy, depth + 1, alpha, beta, level);
			// �����һ���״̬���ص�����
			game->current_player *= -1;
			del_and_recount(game, available_move[i].row, available_move[i].col);
		}
		// ����״̬
		switch (strategy) {
		case STRATEGY_MIN:
			if (nxt.val < res->val) {
				res->val = nxt.val;
				res->pos = available_move[i];
			}
			if (res->val < beta) beta = res->val;
			if (alpha >= beta) return;
			break;
		case STRATEGY_MAX:
			if (nxt.val > res->val) {
				res->val = nxt.val;
				res->pos = available_move[i];
			}
			if (res->val > alpha) alpha = res->val;
			if (alpha >= beta) return;
			break;
		}
	}
}

int pre_search(Game* game, Move move[MAX_WIDTH]) {
	// �ú�������һ�������ϵ͵�Min-Max�����Կ��ܵ�����λ�ý�������
	// generate_move�൱���ǶԷ������㼶���ƣ���pre_search�൱�ڶԷ�������һ������
	// ���ﱾ�����ظ���search�ڵ�0��Ĵ��룬�������Ķ�
	Pos available_move[MAX_WIDTH];
	Move nxt;
	int n_move = 0, n_child = generate_move(game, available_move, max_width[0][0], STRATEGY_MAX);
	int state;
	for (int i = 0; i < n_child; ++i) {
		add_and_recount(game, available_move[i].row, available_move[i].col, game->current_player);
		state = check(game, &available_move[i], game->current_player == BLACK);
		switch (state) {
		case GAME_OVER:
			// ������߷�����ʹ����ֱ��ʤ�������Ȼѡ�������߷�
			del_and_recount(game, available_move[i].row, available_move[i].col);
			move[0].pos = available_move[i];
			move[0].val = SCORE_GAME_OVER;
			return 1;
		case FORBIDDEN_PLAY:
			del_and_recount(game, available_move[i].row, available_move[i].col);
			continue;
		default:
			game->current_player *= -1;
			search(game, &nxt, STRATEGY_MIN, 1, -INF, INF, 0);
			game->current_player *= -1;
			del_and_recount(game, available_move[i].row, available_move[i].col);
			move[n_move].pos = available_move[i]; move[n_move].val = nxt.val;
			++n_move;
		}
	}
	qsort(move, n_move, sizeof(Move), move_cmp);
	return n_move;
}

// ���ڶ��߳�����
static void search_at(void* arg) {
	SearchInfo* info = (SearchInfo*)arg;
	Move res;
	search(info->game, &res, STRATEGY_MIN, 1, -INF, INF, info->level);
	(info->move)->val = res.val;
}

static int multithreading_search(Game* game, int width, int level, Move move[MAX_WIDTH], int const output) {
	assert(width <= MAX_N_THREAD);
	SearchInfo info[MAX_N_THREAD]; HANDLE threads[MAX_N_THREAD]; unsigned int thread_ID[MAX_N_THREAD];
	int n_move = pre_search(game, move);
	n_move = MIN(n_move, width);
	// �������
	if (n_move == 0 || move[0].val <= -SCORE_GAME_OVER) return 0;
	// ��ʤ�������ʱ������ʤ�߷�û������
	// ��������ƺ����ȱ�VCX���֣����������ƺ�û�б�Ҫ��
	if (move[0].val >= SCORE_GAME_OVER) return 1;
	// level == 0 ��ֻ����Ԥ����
	if (output) {
		printf("Ԥ���������\n");
		for (int i = 0; i < n_move; ++i) printf("%c%d: %d\n", 'A' + move[i].pos.col, 15 - move[i].pos.row, move[i].val);
		if (level != 0) printf("��������λ�ã�");
	}
	for (int i = 0; i < n_move; ++i) {
		// �����Ǳ������
		if (move[i].val <= -SCORE_GAME_OVER) {
			n_move = i;
			break;
		}
		if (level == 0) continue;
		info[i].level = level; info[i].move = &move[i];
		new_game[i] = *game; info[i].game = &new_game[i];
		// ����new_game��״̬
		add_and_recount(&new_game[i], move[i].pos.row, move[i].pos.col, new_game[i].current_player);
		new_game[i].current_player *= -1;
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, search_at, (void*)&info[i], 0, &thread_ID[i]);
		printf("%c%d ", 'A' + move[i].pos.col, 15 - move[i].pos.row);
	}
	// level == 0 ʱ��û�д����κ��߳�
	if (level != 0) {
		for (int i = 0; i < n_move; ++i) WaitForSingleObject(threads[i], INFINITE);
		qsort(move, n_move, sizeof(Move), move_cmp);
		if (output) {
			printf("\n�����������\n");
			for (int i = 0; i < n_move; ++i) printf("%c%d: %d\n", 'A' + move[i].pos.col, 15 - move[i].pos.row, move[i].val);
		}
	}
	return n_move;
}

int optimal_move(Game* game, int const width, int const level, int const threshold, Move* move, int const output)
{
	Move move_lst[MAX_WIDTH];
	int n_move = multithreading_search(game, width, level, move_lst, output);
	if (n_move == 0) return -1;
	int best_score = move_lst[0].val;
	if (best_score <= -SCORE_GAME_OVER) {
		*move = move_lst[0];
		return 0;
	}
	for (int i = 1; i < n_move; ++i) {
		if (move_lst[i].val <= -SCORE_GAME_OVER || best_score - move_lst[i].val > threshold) {
			n_move = i;
			break;
		}
	}
	*move = move_lst[mt_rand() % n_move];
	return 1;
}

// �ʼʵ�ֵĸ����㷨���������ڡ�����������
void naive_strategy(Game* game, Move* res)
{
	int score_map[2][BOARD_SIZE][BOARD_SIZE];
	scoring_board(game, score_map);
	res->val = 0;
	int val; Pos nxt; int state;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) continue;
			add_and_recount(game, row, col, game->current_player);
			nxt.row = row; nxt.col = col;
			state = check(game, &nxt, game->current_player == BLACK);
			del_and_recount(game, row, col);
			if (state == FORBIDDEN_PLAY) continue;
			// Min-Max�����жϼ����Ѿ��ذ�ʱ����naive_strategy����ʱӦ�����ӷ���
			if ((val = score_map[P2I(game->current_player)][row][col] + (int)ceil(1.5 * score_map[P2I(-game->current_player)][row][col])) > res->val) {
				res->val = val;
				SET_POS(res->pos, row, col);
			}
			else if (val == res->val) {
				if (mt_rand() % 100 > 50) {
					SET_POS(res->pos, row, col);
				}
			}
		}
	}
}