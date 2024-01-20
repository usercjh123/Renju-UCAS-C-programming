// 该模块用于输入输出
#pragma once
#include "game.h"

#define MAX_LEN 100

#define INPUT_SUCC 0
#define INPUT_HELP 1
#define INPUT_ERR 2

#define IO_BLACK 0
#define IO_RED 1
#define IO_GREEN 2
#define IO_YELLOW 3
#define IO_BLUE 4

void print_with_color(char const message[], int font_color, int bg_color, int new_line); // 用于颜色输出
void welcome_screen(int* mode, int* level, int* color); // 用于输出开始界面
void display(Game const* game, Pos const* pos, int player); // display用于输出棋盘
void IO_loop(Game const* game, Pos* input_pos);
void save_board();