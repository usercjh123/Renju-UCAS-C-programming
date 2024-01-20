#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "game.h"
#include "init.h"
#include "IO.h"
#include "record.h"
#include "AI.h"

int main() {
	int mode, level, color;
	welcome_screen(&mode, &level, &color);
	// 创建并初始化game
	Game game;
	init_all(&game);
	int game_state; // 判断游戏终止
	Pos input_pos; // 用于输入走子位置
	int rnd = 0;
	while(1) {
		// 人机对战
		if (mode == 2){
			if (game.current_player == color) {
				IO_loop(&game, &input_pos);
				game_state = play_at(&game, &input_pos);
			}
			else {
				if(rnd != 0) printf("AI正在决策\n");
				input_pos = AI_play(&game, level, 7, 7, 1, rnd);
				printf("AI下在：\033[1;31;40m%c%d\033[0m\n", 'A' + input_pos.col, 15 - input_pos.row);
				game_state = play_at(&game, &input_pos);
			}
		}
		else {
			IO_loop(&game, &input_pos);
			game_state = play_at(&game, &input_pos);
		}
		// 对异常的状态进行处理
		switch (game_state)
		{
		case FORBIDDEN_PLAY:
			print_with_color("该位置是禁手位！", IO_RED, IO_BLACK, 1);
			// 在禁手状态时，本身就需要判断对方胜利
			if (-game.current_player == BLACK) print_with_color("游戏结束 : 黑方胜利！\n", IO_GREEN, IO_BLACK, 0);
			else print_with_color("游戏结束 : 白方胜利！\n", IO_GREEN, IO_BLACK, 0);
			Sleep(1000);
			goto game_over;
		case GAME_OVER:
			display(&game, &input_pos, 0);
			record(&input_pos);
			// 在game_over状态时，玩家信息已经被取负
			if (-game.current_player == BLACK) print_with_color("游戏结束 : 黑方胜利！\n", IO_GREEN, IO_BLACK, 0);
			else print_with_color("游戏结束 : 白方胜利！\n", IO_GREEN, IO_BLACK, 0);
			Sleep(1000);
			goto game_over;
		}
		++rnd;
		record(&input_pos);
	}
game_over:;
	save_board();
	return 0;
}
