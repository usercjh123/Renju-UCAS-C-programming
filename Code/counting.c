#include <assert.h>
#include "counting.h"
#include "game.h"
#include "zobrist.h"
#include "utils.h"

// 用于在棋盘上移动
static int const mv_row[4] = {0, 1, 1, -1};
static int const mv_col[4] = {1, 0, 1,  1};

//// 工具函数：用于计算距离边界的距离（已用宏替代）
//static inline int max_step(int row, int col, int mv_r, int mv_c)
//{
//	int dr = BOARD_SIZE, dc = BOARD_SIZE;
//	if (mv_r != 0) dr = (mv_r == 1) ? BOARD_SIZE - row - 1 : row;
//	if (mv_c != 0) dc = (mv_c == 1) ? BOARD_SIZE - col - 1 : col;
//	return MIN(dr, dc);
//}

// 工具函数：数某条射线上的棋子
static void count_half_line(HalfLine* res, int const board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player, int mv_r, int mv_c)
{
	int d = MAX_STEP(r, c, mv_r, mv_c);
	int i = 1;
	res->hash = 0;
	for (; i <= d; ++i) {
		r += mv_r; c += mv_c;
		if (board[r][c] != player) { // 遇到空格或对方棋子
			res->blockage = (board[r][c] == 0) ? 0 : 1; res->len = i - 1;
			return;
		}
		else zobrist_change(&(res->hash), r, c, player); // 此处的else没有用，只是用来提醒一下这里已经有board[r][c] == player
	}
	res->blockage = 1; res->len = d; // 遇到边界
}

// 工具函数：数某条直线上的棋子
static void count_dir(Lines* res, int const board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player, Direction dir) {
	HalfLine hl_res;
	for (int p = -1; p <= 1; p += 2) { // 遍历两个相反方向（编译器展开循环）
		count_half_line(&hl_res, board, r, c, player, p * mv_row[dir], p * mv_col[dir]);
		res->hline[dir][P2I(p)] = hl_res;
	}
}

void count_all_dir(Lines* res, int const board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player)
{
	Direction dir;
	for_all_dir(dir) {
		count_dir(res, board, r, c, player, dir); // 初始化全部放在count_dir中
	}
}
// 获取某条射线上最临近的空点
int adjacent_blank(Pos* pos, Game const* game, int row, int col, int mv_r, int mv_c)
{
	int i = 1, d = MAX_STEP(row, col, mv_r, mv_c);
	int r = row, c = col;
	for (; i <= d; ++i) {
		r += mv_r; c += mv_c;
		if (game->board[r][c] == 0) {
			pos->row = r; pos->col = c;
			return 1;
		}
	}
	return 0;
}

static inline void count_all_player(Game* game, int row, int col, Direction dir) {
	count_dir(&(game->board_cnt[row][col][P2I(BLACK)]), game->board, row, col, BLACK, dir);
	count_dir(&(game->board_cnt[row][col][P2I(WHITE)]), game->board, row, col, WHITE, dir);
}

static inline void recounting_from(Game* game, int row, int col) {
	Direction dir; Pos pos;
	int p;
	for_all_dir(dir) {
		for (p = -1; p <= 1; p += 2) { // 遍历两个相反方向
			if (adjacent_blank(&pos, game, row, col, p * mv_row[dir], p * mv_col[dir])) {
				count_all_player(game, pos.row, pos.col, dir);
			}
		}
	}
}

void add_and_recount(Game* game, int row, int col, int player) {
// 思路：增加棋子最多会影响其8个方向上的第一个空点，因此找到这8个点并重新数子即可（这并非效率最高的实现方式）
	assert(game->board[row][col] == 0);
	game->board[row][col] = player;
	zobrist_change(&(game->board_hash), row, col, player);
	recounting_from(game, row, col);
}

void del_and_recount(Game* game, int row, int col) {
	assert(game->board[row][col] != 0);
	zobrist_change(&(game->board_hash), row, col, game->board[row][col]);
	game->board[row][col] = 0;
	recounting_from(game, row, col);
	Direction dir;
	for_all_dir(dir) count_all_player(game, row, col, dir);
}
