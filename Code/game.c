#include "game.h"
#include "counting.h"
#include "checker.h"
#include "utils.h"

void initialize_game(Game* game) {
	Direction dir; int p;
	for (int row = 0; row < BOARD_SIZE; ++row) {
		for (int col = 0; col < BOARD_SIZE; ++col) {
			game->board[row][col] = 0;
			for (p = 0; p <= 1; ++p) {
				for_all_dir(dir) {
					game->board_cnt[row][col][p].hline[dir][0].len = 0;
					game->board_cnt[row][col][p].hline[dir][0].blockage = 0;
					game->board_cnt[row][col][p].hline[dir][0].hash = 0;
					game->board_cnt[row][col][p].hline[dir][1].len = 0;
					game->board_cnt[row][col][p].hline[dir][1].blockage = 0;
					game->board_cnt[row][col][p].hline[dir][1].hash = 0;
				}
			}
		}
	}
	game->board_hash = 0;
	game->current_player = BLACK;
	game->recent_play.col = -1;
}

int play_at(Game* game, Pos const* pos)
{
	if (game->board[pos->row][pos->col] != 0) return OCCUPATION;
	add_and_recount(game, pos->row, pos->col, game->current_player);
	int state = check(game, pos, game->current_player == BLACK);
	if (state == FORBIDDEN_PLAY) del_and_recount(game, pos->row, pos->col); // 对于不合法的走子应重置状态
	else {
		game->current_player *= -1;
		game->recent_play = *pos;
	}
	return state;
}