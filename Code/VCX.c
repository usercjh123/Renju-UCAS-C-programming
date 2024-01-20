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

// 一些数组最大大小
#define MAX_N_THREAT 15
#define MAX_N_DEFENCE 36
#define MAX_N_ATTACK_POS 36

// 搜索成功与失败返回值
#define SUCCESS 1
#define FAIL 0

// 等级与搜索深度对照表
static int level2depth[3] = { 8, 10, 12 };

static int const mv_row[4] = { 0, 1, 1, -1 };
static int const mv_col[4] = { 1, 0, 1,  1 };

static Game new_game[MAX_N_THREAD]; // 多线程搜索内存

// 工具函数，判断某方向是否构成威胁
static inline int is_dir_threat(Game const* game, Pos const* pos, Direction dir, int player) {
	Lines const* res;
	int len, blockage;
	res = &game->board_cnt[pos->row][pos->col][P2I(player)];
	len = res->hline[dir][0].len + res->hline[dir][1].len;
	blockage = res->hline[dir][0].blockage | res->hline[dir][1].blockage;
	if ((len == 3 && blockage == 0) || len == 4) return 1;
	return 0;
}

// defend的辅助函数，用于检测一个威胁点是否仍然构成威胁
static inline int is_threat(Game const* game, Pos const* pos) {
	if (game->board[pos->row][pos->col] != 0) return 0;
	Direction dir;
	for_all_dir(dir) {
		if (is_dir_threat(game, pos, dir, -game->current_player)) return 1;
	}
	return 0;
}

// 检查所有威胁点是否还构成威胁
static inline int check_defence(Game const* game, Pos const threat_pos[MAX_N_THREAT], int n_threat_pos) {
	for (int i = 0; i < n_threat_pos; ++i) {
		if (is_threat(game, &threat_pos[i])) return 0;
	}
	return 1;
}

// 检查进攻步是否造成威胁，并返回威胁类型和威胁点
// 这是理解VCX搜索的核心
static inline void find_threat_pos(Game const* game, Pos const* pos, Pos threat_pos[MAX_N_THREAT], int* n_threat_pos) {
	/*
	如果某一步构成威胁，threat_pos中将存放这一步的“威胁点”
	例如，下面的棋形(w->白, b->黑, 0->空)中，黑棋构成了活三，也就是再走一步就形成活四
	w0bbb00w
	12345678
	但只有再6位置再走一步才能形成活四，导致黑棋胜利，因此6位置就是这个棋形的“威胁点”，threat_pos将存放6位置
	这时白棋如果希望防御，只需要使得6位置不再构成威胁点（即在6位置再下一步不会形成活四），因此可以走2,6,7位置（可以不必下在6位置，这给了白棋更多的防御形式）
	再如，在下面的黑棋活三棋形中，威胁点是3与7
	w00bbb00w
	123456789
	白棋如果想防御需要使得两个威胁点同时消失，而如果下在8或2位置只能使得一个位置的威胁点消失，因此需要下在3或7位置
	再如，在下面的黑棋冲四棋形中，威胁点是6（黑棋下一步形成五），白棋为了使得威胁点消失，只能下在6位置
	wbbbb00w
	12345678
	*/
	*n_threat_pos = 0;
	// 下面的代码对checker.c中的check_three_four函数进行了简单的修改
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

// 如果能够成功防御，返回1，否则返回0
static int defend(Game* game, int depth, Pos const threat_pos[MAX_N_THREAT], int const n_threat_pos, int const threat_type, int const max_depth) {
	int state;
	Pos attack_pos, nxt;
	Pos defence[MAX_N_DEFENCE]; int n_defence = 0;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) continue;
			add_and_recount(game, row, col, game->current_player);
			// 跳过禁手点
			nxt.row = row; nxt.col = col;
			state = check(game, &nxt, game->current_player == BLACK);
			if (state == FORBIDDEN_PLAY) goto next;
			// 如果某步导致自己获胜，也是成功防守
			// 由于进攻方已经构建了一步威胁，后手的防守方若想通过进攻来防守，只能通过构建一个单步威胁才可能达成目标
			// 为了简化搜索，我们认为在防守方构成了一个单步威胁且进攻方仅仅构成了一个两步威胁时，进攻方进攻失败（因为此时进攻方不得不停下来防守）
			// 在比赛中，貌似发现这里有个bug，即在对方连五时这里判断不出来？？？忘记保存棋谱了，不知道这里为什么有bug
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

// 利用结构体传递多参数
static void attack_at(void* arg) {
	AttackInfo* info = (AttackInfo*)arg;
	Pos threat_pos[MAX_N_THREAT]; int n_threat_pos;
	find_threat_pos(info->game, &(info->attack_pos), threat_pos, &n_threat_pos); // 该点构成三或四，且不是叶子节点，利用find_threat_pos函数找出威胁点
	info->game->current_player *= -1; // 设置下一层的状态，进入下一层搜索(defend)
	info->state = defend(info->game, (info->depth) + 1, threat_pos, n_threat_pos, info->threat_type, info->max_depth);
	info->game->current_player *= -1; // 返回本层
}

// branching-factor 最大约为10，有很多0
// 如果能够成功进攻，返回1，否则返回0
int attack(Game* game, int depth, Pos* attack_pos, int const max_depth) {
	//printf("%d\n", depth);
	Pos nxt; int state;
	AttackInfo info;
	int score_map[2][BOARD_SIZE][BOARD_SIZE]; scoring_board(game, score_map);
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			// 在层数较大时，不扩展全部进攻节点（限制搜索空间）
			if (game->board[row][col] != 0 || (depth > 0 && score_map[P2I(game->current_player)][row][col] < 30)) continue;
			// 跳过无威胁点或禁手点
			add_and_recount(game, row, col, game->current_player);
			nxt.row = row; nxt.col = col;
			state = check(game, &nxt, game->current_player == BLACK);
			if (state == FORBIDDEN_PLAY || state == NORMAL_PLAY) goto next; // 搜索下一个进攻点
			if (state == GAME_OVER) { // 进攻方胜利
				del_and_recount(game, row, col);
				*attack_pos = nxt; // 记录必胜走法
				return SUCCESS;
			}
			if (depth == max_depth) goto next; // 叶子节点，不继续遍历
			info.game = game;
			SET_POS(info.attack_pos, row, col); 
			info.depth = depth; info.threat_type = state; info.max_depth = max_depth;
			attack_at((void*)&info);
			if (!(info.state)) { // 如果该进攻步无法成功防御，则进攻成功
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
	// 与attack函数思路相同，这里仅仅记录下了可能的攻击位置
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
	if(output) printf("VCX搜索评估进攻位置：");
	do {
		// 这里曾经栈溢出过，不确定现在是否稳定
		int n_thread;
		for (n_thread = 0; n_thread < MAX_N_THREAD && i < n; ++n_thread) {
			new_game[n_thread] = *game; // 将状态信息复制一遍，防止线程冲突
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