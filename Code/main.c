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
	// ��������ʼ��game
	Game game;
	init_all(&game);
	int game_state; // �ж���Ϸ��ֹ
	Pos input_pos; // ������������λ��
	int rnd = 0;
	while(1) {
		// �˻���ս
		if (mode == 2){
			if (game.current_player == color) {
				IO_loop(&game, &input_pos);
				game_state = play_at(&game, &input_pos);
			}
			else {
				if(rnd != 0) printf("AI���ھ���\n");
				input_pos = AI_play(&game, level, 7, 7, 1, rnd);
				printf("AI���ڣ�\033[1;31;40m%c%d\033[0m\n", 'A' + input_pos.col, 15 - input_pos.row);
				game_state = play_at(&game, &input_pos);
			}
		}
		else {
			IO_loop(&game, &input_pos);
			game_state = play_at(&game, &input_pos);
		}
		// ���쳣��״̬���д���
		switch (game_state)
		{
		case FORBIDDEN_PLAY:
			print_with_color("��λ���ǽ���λ��", IO_RED, IO_BLACK, 1);
			// �ڽ���״̬ʱ���������Ҫ�ж϶Է�ʤ��
			if (-game.current_player == BLACK) print_with_color("��Ϸ���� : �ڷ�ʤ����\n", IO_GREEN, IO_BLACK, 0);
			else print_with_color("��Ϸ���� : �׷�ʤ����\n", IO_GREEN, IO_BLACK, 0);
			Sleep(1000);
			goto game_over;
		case GAME_OVER:
			display(&game, &input_pos, 0);
			record(&input_pos);
			// ��game_over״̬ʱ�������Ϣ�Ѿ���ȡ��
			if (-game.current_player == BLACK) print_with_color("��Ϸ���� : �ڷ�ʤ����\n", IO_GREEN, IO_BLACK, 0);
			else print_with_color("��Ϸ���� : �׷�ʤ����\n", IO_GREEN, IO_BLACK, 0);
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
