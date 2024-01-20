#include <assert.h>
#include <math.h>
#include <string.h>
#include "evaluator.h"
#include "counting.h"
#include "game.h"
#include "utils.h"
#include "checker.h"

// ��������
#define TYPE_NONE 0
#define TYPE_OPEN_TWO 1
#define TYPE_CLOSE_TWO 2
#define TYPE_OPEN_THREE 3
#define TYPE_CLOSE_THREE 4
#define TYPE_OPEN_FOUR 5
#define TYPE_CLOSE_FOUR 6

// ����, ���, ���, ����, ����, ����, ����
static int EVAL_SCORE_LIST_SELF[8] = { 1, 23, 13, 211, 89, SCORE_GAME_OVER, SCORE_GAME_OVER }; // �������վ�������ʹ�õķ�����
static int EVAL_SCORE_LIST_OP[8] = { 1, 17, 11, 199, 61, SCORE_GAME_OVER, 229 };
static int MOVE_SCORE_LIST[8] = { 1, 40, 10, 1000, 50, 1000, 1000 }; // ��������ѡ��ʱʹ�õķ�����������һ���Ǽ����ߣ�
#define NBHR_SCORE 28

#define CHECK_EQUAL(x, y) ((y) == -1 || (x) == (y))

// Ӱ��ǿ��������˥��
#define DECAY1 0.8
#define DECAY2 0.3


// �����ƶ�
static int const mv_row[4] = { 0, 1, 1, -1 };
static int const mv_col[4] = { 1, 0, 1,  1 };

// ���ߺ����������ж�����
static inline int get_type(int len, int blockage) {
	if (len <= 1) return TYPE_NONE;
	switch (len) {
	case 2:
		if (blockage == 0) return TYPE_OPEN_TWO;
		else if (blockage == 1) return TYPE_CLOSE_TWO;
		return TYPE_NONE;
	case 3:
		if (blockage == 0) return TYPE_OPEN_THREE;
		else if (blockage == 1) return TYPE_CLOSE_THREE; // ���඼��ס����û�м�ֵ�����������
		return TYPE_NONE;
	case 4:
		if (blockage == 0) return TYPE_OPEN_FOUR;
		else return TYPE_CLOSE_FOUR; // ���඼��ס����Ҳ�ǳ���
	}
	if (len >= 5) {
		if (blockage == 0) return TYPE_OPEN_FOUR;
		else return TYPE_CLOSE_FOUR;
	}
	// �����ϲ������
	assert(0);
}

// ���ھ�������
// ����ʱ����һ���Ǽ�������
int eval_score(Game const* game, int player) {
	Direction dir; int p = 0;
	Lines const* res;
	int score = 0, type;
	int len, blockage;
	// ������, �ٽ�����
	// 0,1 -> white,black
	int tot[2] = { 0, 0 }, nbhr[2] = { 0, 0 };
	// ���ڱ�����Χ��
	int r, c;
	// 0 -> opn; 1 -> self
	int cnt[2][8] = { {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0} };
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			if (game->board[row][col] != 0) {
				// ͳ���ٽ�����
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
				// �����յ��Ӧ����в
				for_all_dir(dir) {
					for (p = -1; p <= 1; p += 2) { // ������չ��
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
	// ͨ�����������Ķ���������в�㣬�����󲿷���������в�㣬������Ϊ�˱���Ա�ʤ/�ظ����������
	cnt[1][TYPE_OPEN_FOUR] >>= 1; cnt[0][TYPE_OPEN_FOUR] >>= 1;
	cnt[1][TYPE_OPEN_THREE] >>= 1; cnt[0][TYPE_OPEN_THREE] >>= 1;
	// ����������һ���ģ���Ϊ��һ���Ǽ����ߣ�
	if (cnt[1][TYPE_CLOSE_FOUR] + cnt[1][TYPE_OPEN_FOUR] >= 1) return SCORE_GAME_OVER;
	// ��֮�¶����輺��û����
	// �Է���������������/һ������һ������/��������/һ������
	else if (cnt[0][TYPE_CLOSE_FOUR] + cnt[0][TYPE_OPEN_THREE] >= 2 || cnt[0][TYPE_OPEN_FOUR] >= 1) return -SCORE_GAME_OVER;
	// ����������һ����������Ϊ��һ���Ǽ����ߣ��ҶԷ�û�г��Ļ����
	else if (cnt[1][TYPE_OPEN_THREE] >= 1 && (cnt[0][TYPE_CLOSE_FOUR] + cnt[0][TYPE_OPEN_FOUR] == 0)) return SCORE_GAME_OVER;
	// �������ϵ����ι��ɵ�����
	for (p = 0; p < 7; ++p) score += EVAL_SCORE_LIST_SELF[p] * cnt[1][p] - EVAL_SCORE_LIST_OP[p] * cnt[0][p];
	// �ڶ����Ӧ�������ӵ�ƽ���ٽ�������Ӧ�ķ�ֵ
	return score + (int)ceil(NBHR_SCORE * (1.0 * nbhr[P2I(player)] / tot[P2I(player)] - 1.0 * nbhr[P2I(-player)] / tot[P2I(-player)]));
}

// ��������
static inline int is_type(HalfLine const hl[2], int len0, int blockage0, int len1, int blockage1) {
	return CHECK_EQUAL(hl[0].len, len0) && CHECK_EQUAL(hl[0].blockage, blockage0) && CHECK_EQUAL(hl[1].len, len1) && CHECK_EQUAL(hl[1].blockage, blockage1);
}

// ��������
void scoring_board(Game const* game, int score_map[2][BOARD_SIZE][BOARD_SIZE])
{
	Direction dir;
	Lines const* res;
	int p, q; // ����Сѭ��
	int len, blockage, type;
	int val;
	// ��ʼ��score_map
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
					for (q = 0; q <= 1; ++q) { // ��������
						if (res->hline[dir][q].len == 0 && res->hline[dir][q].blockage == 0) { // Ϊ���ڵ�ӷ�
							score_map[p][row + I2P(q) * mv_row[dir]][col + I2P(q) * mv_col[dir]] += (int)(val * DECAY1);
						}
					}
					// ������һЩ����
					// �������0xox0��o�ǵ�ǰ���ǵ㣬��ͬ��==> TYPE_OPEN_TWO
					if (type == TYPE_OPEN_TWO && is_type(res->hline[dir], 1, 0, 1, 0)) {
						score_map[p][row + 2 * mv_row[dir]][col + 2 * mv_col[dir]] += (int)(val * DECAY2);
						score_map[p][row - 2 * mv_row[dir]][col - 2 * mv_col[dir]] += (int)(val * DECAY2);
					}
					// ������
					else if (type == TYPE_CLOSE_THREE){
						// -bxxox0+
						if(is_type(res->hline[dir], 2, 1, 1, 0)) score_map[p][row + 2 * mv_row[dir]][col + 2 * mv_col[dir]] += (int)(val * DECAY2);
						// -0xoxxb+
						else if(is_type(res->hline[dir], 1, 0, 2, 1)) score_map[p][row - 2 * mv_row[dir]][col - 2 * mv_col[dir]] += (int)(val * DECAY2);
					}
					// ������
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
