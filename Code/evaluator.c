#include <assert.h>
#include <math.h>
#include <string.h>
#include "evaluator.h"
#include "counting.h"
#include "game.h"
#include "utils.h"
#include "checker.h"

// 用于评分
#define TYPE_NONE 0
#define TYPE_OPEN_TWO 1
#define TYPE_CLOSE_TWO 2
#define TYPE_OPEN_THREE 3
#define TYPE_CLOSE_THREE 4
#define TYPE_OPEN_FOUR 5
#define TYPE_CLOSE_FOUR 6

// 单子, 活二, 冲二, 活三, 冲三, 活四, 冲四
static int EVAL_SCORE_LIST_SELF[8] = { 1, 23, 13, 211, 89, SCORE_GAME_OVER, SCORE_GAME_OVER }; // 进行最终局面评估使用的分数表
static int EVAL_SCORE_LIST_OP[8] = { 1, 17, 11, 199, 61, SCORE_GAME_OVER, 229 };
static int MOVE_SCORE_LIST[8] = { 1, 40, 10, 1000, 50, 1000, 1000 }; // 进行走子选择时使用的分数表（假设下一步是己方走）
#define NBHR_SCORE 28

#define CHECK_EQUAL(x, y) ((y) == -1 || (x) == (y))

// 影响强度随距离的衰减
#define DECAY1 0.8
#define DECAY2 0.3


// 棋盘移动
static int const mv_row[4] = { 0, 1, 1, -1 };
static int const mv_col[4] = { 1, 0, 1,  1 };

// 工具函数，用于判断棋形
static inline int get_type(int len, int blockage) {
	if (len <= 1) return TYPE_NONE;
	switch (len) {
	case 2:
		if (blockage == 0) return TYPE_OPEN_TWO;
		else if (blockage == 1) return TYPE_CLOSE_TWO;
		return TYPE_NONE;
	case 3:
		if (blockage == 0) return TYPE_OPEN_THREE;
		else if (blockage == 1) return TYPE_CLOSE_THREE; // 两侧都堵住的三没有价值（不算冲三）
		return TYPE_NONE;
	case 4:
		if (blockage == 0) return TYPE_OPEN_FOUR;
		else return TYPE_CLOSE_FOUR; // 两侧都堵住的四也是冲四
	}
	if (len >= 5) {
		if (blockage == 0) return TYPE_OPEN_FOUR;
		else return TYPE_CLOSE_FOUR;
	}
	// 理论上不会出现
	assert(0);
}

// 用于局面评估
// 评估时，下一步是己方走子
int eval_score(Game const* game, int player) {
	Direction dir; int p = 0;
	Lines const* res;
	int score = 0, type;
	int len, blockage;
	// 总子数, 临接子数
	// 0,1 -> white,black
	int tot[2] = { 0, 0 }, nbhr[2] = { 0, 0 };
	// 用于遍历周围子
	int r, c;
	// 0 -> opn; 1 -> self
	int cnt[2][8] = { {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) {
				// 统计临接子数
				++tot[P2I(game->board[row][col])];
				for_all_dir(dir) {
					for (p = -1; p <= 1; p += 2) {
						r = row + p * mv_row[dir]; c = col + p * mv_col[dir];
						if (IN_RANGE(r, BOARD_SIZE) && IN_RANGE(c, BOARD_SIZE) && game->board[r][c] == game->board[row][col])
							++nbhr[P2I(game->board[row][col])];
					}
				}
			}
			else {
				// 分析空点对应的威胁
				for_all_dir(dir) {
					for (p = -1; p <= 1; p += 2) { // 编译器展开
						res = &game->board_cnt[row][col][P2I(p * player)];
						len = res->hline[dir][0].len + res->hline[dir][1].len;
						blockage = res->hline[dir][0].blockage + res->hline[dir][1].blockage;
						type = get_type(len, blockage);
						if (type != TYPE_NONE) cnt[P2I(p)][type] += 1;
						else cnt[P2I(p)][TYPE_NONE] += len;
					}
				}
			}
		}
	}
	// 通常来讲，活四都有两个威胁点，活三大部分有两个威胁点，这里是为了避免对必胜/必负局面的误判
	cnt[1][TYPE_OPEN_FOUR] >>= 1; cnt[0][TYPE_OPEN_FOUR] >>= 1;
	cnt[1][TYPE_OPEN_THREE] >>= 1; cnt[0][TYPE_OPEN_THREE] >>= 1;
	// 己方有至少一个四（因为下一步是己方走）
	if (cnt[1][TYPE_CLOSE_FOUR] + cnt[1][TYPE_OPEN_FOUR] >= 1) return SCORE_GAME_OVER;
	// 这之下都假设己方没有四
	// 对方有至少两个冲四/一个活三一个冲四/两个活三/一个活四
	else if (cnt[0][TYPE_CLOSE_FOUR] + cnt[0][TYPE_OPEN_THREE] >= 2 || cnt[0][TYPE_OPEN_FOUR] >= 1) return -SCORE_GAME_OVER;
	// 己方有至少一个活三（因为下一步是己方走）且对方没有冲四或活四
	else if (cnt[1][TYPE_OPEN_THREE] >= 1 && (cnt[0][TYPE_CLOSE_FOUR] + cnt[0][TYPE_OPEN_FOUR] == 0)) return SCORE_GAME_OVER;
	// 计算线上的棋形构成的评分
	for (p = 0; p < 7; ++p) score += EVAL_SCORE_LIST_SELF[p] * cnt[1][p] - EVAL_SCORE_LIST_OP[p] * cnt[0][p];
	// 第二项对应的是棋子的平均临接子数对应的分值
	return score + (int)ceil(NBHR_SCORE * (1.0 * nbhr[P2I(player)] / tot[P2I(player)] - 1.0 * nbhr[P2I(-player)] / tot[P2I(-player)]));
}

// 辅助函数
static inline int is_type(HalfLine const hl[2], int len0, int blockage0, int len1, int blockage1) {
	return CHECK_EQUAL(hl[0].len, len0) && CHECK_EQUAL(hl[0].blockage, blockage0) && CHECK_EQUAL(hl[1].len, len1) && CHECK_EQUAL(hl[1].blockage, blockage1);
}

// 用于走子
void scoring_board(Game const* game, int score_map[2][BOARD_SIZE][BOARD_SIZE])
{
	Direction dir;
	Lines const* res;
	int p, q; // 用于小循环
	int len, blockage, type;
	int val;
	// 初始化score_map
	memset(score_map, 0, sizeof(int) * 2 * BOARD_SIZE * BOARD_SIZE);
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) continue;
			for (p = 0; p <= 1; ++p) {
				res = &game->board_cnt[row][col][p];
				for_all_dir(dir) {
					len = res->hline[dir][0].len + res->hline[dir][1].len;
					blockage = res->hline[dir][0].blockage + res->hline[dir][1].blockage;
					type = get_type(len, blockage);
					if (type == TYPE_NONE) {
						score_map[p][row][col] += len;
						continue;
					}
					score_map[p][row][col] += (val = MOVE_SCORE_LIST[type]);
					for (q = 0; q <= 1; ++q) { // 两条射线
						if (res->hline[dir][q].len == 0 && res->hline[dir][q].blockage == 0) { // 为近邻点加分
							score_map[p][row + I2P(q) * mv_row[dir]][col + I2P(q) * mv_col[dir]] += (int)(val * DECAY1);
						}
					}
					// 下面是一些特判
					// 跳活二：0xox0（o是当前考虑点，下同）==> TYPE_OPEN_TWO
					if (type == TYPE_OPEN_TWO && is_type(res->hline[dir], 1, 0, 1, 0)) {
						score_map[p][row + 2 * mv_row[dir]][col + 2 * mv_col[dir]] += (int)(val * DECAY2);
						score_map[p][row - 2 * mv_row[dir]][col - 2 * mv_col[dir]] += (int)(val * DECAY2);
					}
					// 跳冲三
					else if (type == TYPE_CLOSE_THREE){
						// -bxxox0+
						if(is_type(res->hline[dir], 2, 1, 1, 0)) score_map[p][row + 2 * mv_row[dir]][col + 2 * mv_col[dir]] += (int)(val * DECAY2);
						// -0xoxxb+
						else if(is_type(res->hline[dir], 1, 0, 2, 1)) score_map[p][row - 2 * mv_row[dir]][col - 2 * mv_col[dir]] += (int)(val * DECAY2);
					}
					// 跳活三
					else if (type == TYPE_OPEN_THREE){
						// -0xxox0+
						if(is_type(res->hline[dir], 2, 0, 1, 0)) score_map[p][row + 2 * mv_row[dir]][col + 2 * mv_col[dir]] += (int)(val * DECAY2);
						// -0xoxx0+
						else if(is_type(res->hline[dir], 1, 0, 2, 0)) score_map[p][row - 2 * mv_row[dir]][col - 2 * mv_col[dir]] += (int)(val * DECAY2);
					}
				}
			}
		}
	}
}
