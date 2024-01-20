// ��ģ�������������
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

void print_with_color(char const message[], int font_color, int bg_color, int new_line); // ������ɫ���
void welcome_screen(int* mode, int* level, int* color); // ���������ʼ����
void display(Game const* game, Pos const* pos, int player); // display�����������
void IO_loop(Game const* game, Pos* input_pos);
void save_board();