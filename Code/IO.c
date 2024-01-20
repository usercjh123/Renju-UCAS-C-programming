#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <windows.h>
#include "IO.h"
#include "record.h"
#include "game.h"

static int getline(char str[], int max_len, int ignore_line_break) {
	char c; int i;
	for (i = 0; i < max_len - 1 && (c = getchar()) != EOF && c != '\n'; ++i) str[i] = c;
	if (!ignore_line_break && c == '\n') {
		str[i] = c;
		++i;
	}
	str[i] = '\0';
	return i;
}

static void generate_prompt(Game const* game) {
	printf("当前的玩家是");
	if (game->current_player == BLACK) print_with_color("黑方. ", IO_RED, IO_BLACK, 0);
	else print_with_color("白方. ", IO_RED, IO_BLACK, 0);
	printf("请输入走子位置（输入'?'查看输入规则）: ");
}

static int get_input(Pos* pos) { // 输入必须为问号或“字母+数字”形式
	char str[MAX_LEN]; getline(str, MAX_LEN, 0);
	int i = 0;
	int col = 0, row = 0;
	for (; str[i] != '\0' && str[i] == ' '; ++i);
	if (str[i] == '?') return INPUT_HELP;
	if (isalpha(str[i])) {
		col = tolower(str[i]) - 'a';
		if (col >= 15) return INPUT_ERR;
		for (++i; str[i] != '\0' && isdigit(str[i]); ++i) {
			row = row * 10 + str[i] - '0';
		}
		if (row == 0 || row > 15) return INPUT_ERR;
		pos->col = col;
		pos->row = 15 - row;
		return INPUT_SUCC;
	}
	else return INPUT_ERR;
}

void display(Game const* game, Pos const* pos, int player)
{
	// 后两个参数是为了在不改变棋盘的情况下新增棋子
	for (int row = 0; row < BOARD_SIZE; ++row) {
		printf("%2d ", 15 - row);
		for (int col = 0; col < BOARD_SIZE; ++col) {
			// 棋子输出
			if (player != 0 && row == pos->row && col == pos->col) {
				printf(player == WHITE ? "△": "▲");
			}
			else if (game->board[row][col] != 0) {
				if (player == 0 && row == game->recent_play.row && col == game->recent_play.col) {
					printf(game->current_player == WHITE ? "▲" : "△");
				}
				else printf(game->board[row][col] == BLACK ? "●" : "○");
			}
			// 空白格输出
			else {
				if (row == 0) {
					if (col == 0) printf("┌─");
					else if (col == BOARD_SIZE - 1) printf("┐");
					else printf("┬─");
				}
				else if (row == BOARD_SIZE - 1) {
					if (col == 0) printf("└─");
					else if (col == BOARD_SIZE - 1) printf("┘");
					else printf("┴─");
				}
				else {
					if (col == 0) printf("├─");
					else if (col == BOARD_SIZE - 1) printf("┤");
					else printf("┼─");
				}
			}
		}
		printf("\n");
	}
	printf("   ");
	for (int col = 0; col < BOARD_SIZE; ++col) {
		printf("%c ", 'A' + col);
	}
	printf("\n");
}



// 工具函数
int valid_char(char c) { return c == '?' || isdigit(c) || isalpha(c); }



void print_with_color(char const message[], int font_color, int bg_color, int new_line)
{
	printf("\033[1;%d;%dm%s\033[0m", 30 + font_color, 40 + bg_color, message);
	if (new_line) printf("\n");
}

void IO_loop(Game const* game, Pos* input_pos)
{
	int input_state, undo = 1;
	char line[10];
	do {
		display(game, input_pos, 0);
		while ((generate_prompt(game), (input_state = get_input(input_pos)) != INPUT_SUCC)) {
			if (input_state == INPUT_HELP) printf("输入必须是\"字母（大小写均可）+ 数字\"（如：C3、a6），字符前的空格会被忽略，在字母和数字之间不能有空格！\n");
			else print_with_color("输入错误！", IO_RED, IO_BLACK, 1);
		}
		if (game->board[input_pos->row][input_pos->col] != 0) {
			print_with_color("该位置已有棋子！", IO_RED, IO_BLACK, 1);
			continue;
		}
		display(game, input_pos, game->current_player);
		printf("是否悔棋(Y,y/任意字符)：");
		getline(line, 10, 0);
		if (line[0] == 'Y' || line[0] == 'y') undo = 1;
		else undo = 0;
	} while (undo);
	
}

void welcome_screen(int* mode, int* level, int* color) {
	printf("\033[0m\n");
	print_with_color("                                       五子棋对战", IO_RED, IO_BLACK, 1);
	printf("----------------------------------------------------------------------------------------------------\n");
	printf("本程序不需要使用旧版控制台！\n如在本界面出现显示问题，请取消\"使用旧版控制台\"选项！\n");
	printf("\n重要提醒：\n输入棋盘位置坐标时，采用\"字母（大小写均可）+ 数字\"（如：C3、a6）的形式，在字母和数字之间不能有空格！\n");
	printf("----------------------------------------------------------------------------------------------------\n");
	printf("游戏模式：\n");
	printf("1. 人人对战\n2. 人机对战\n");
	printf("-------------------------------------------------\n");
	*mode = 0;
	while (*mode != 1 && *mode != 2) {
		printf("请选择游戏模式：");
		scanf("%d", mode);
		getchar(); // 这是个大坑！scanf不读入\n，因此需要用getchar把缓冲区的\n读出
		if (*mode != 1 && *mode != 2) printf("输入错误！请重新选择！\n");
	}
	if (*mode == 1) {
		printf("你选择人人对战模式！按任意键继续...\n");
		_getch();
		Sleep(500);
		system("cls");
		fflush(stdin);
		return;
	}
	printf("-------------------------------------------------\n");
	printf("AI强度：\n");
	printf("1. 较弱\n2. 中等\n3. 较强\n");
	*level = 0;
	while (*level != 1 && *level != 2 && *level != 3) {
		printf("请选择AI强度：");
		scanf("%d", level);
		getchar();
		if (*level != 1 && *level != 2 && *level != 3) printf("输入错误！请重新选择！\n");
	}
	printf("-------------------------------------------------\n");
	*color = 0;
	while (*color != 1 && *color != 2) {
		printf("请选择玩家所执棋子（1. 黑子先手 2. 白子后手）：");
		scanf("%d", color);
		getchar();
		if (*color != 1 && *color != 2) printf("输入错误！请重新选择！\n");
	}
	printf("你选择人机对战模式！\nAI强度：");
	switch (*level)
	{
	case 1:
		printf("较弱\n");
		break;
	case 2:
		printf("中等\n");
		break;
	case 3:
		printf("较强\n");
		break;
	}
	*level -= 1;
	printf("玩家执");
	switch (*color) {
	case 1:
		printf("黑子\n");
		break;
	case 2:
		printf("白子\n");
		break;
	}
	*color = (*color == 1) ? 1 : -1;
	printf("按任意键继续...\n");
	_getch();
	Sleep(500);
	system("cls");
	return;
}

void save_board() {
	while (1) {
		char line[10];
		printf("是否保存局面（Y,y/任意字符）：");
		getline(line, 10, 0);
		if (line[0] != 'Y' && line[0] != 'y') break;
		printf("请输入保存文件名（保存在当前目录，不需要加扩展名）：\n");
		char filename[100], filedir[100] = "./";
		getline(filename, 100, 1);
		strcat(filedir, filename);
		strcat(filedir, ".sav");
		if (save(filedir) == 0) {
			print_with_color("保存成功！", IO_GREEN, IO_BLACK, 1);
			break;
		}
		else print_with_color("保存失败！", IO_RED, IO_BLACK, 1);
	}
	
	
	
}