// ��ģ������������������
#pragma once
#include "game.h"

typedef struct Game Game; // �����ǰ����

typedef enum Direction {
	HORZ, VERT, pDIAG, sDIAG
	// pDIAG : ���Խ��ߣ�����--���£�
	// sDIAG : ���Խ��ߣ�����--���ϣ�
} Direction;

// ʵ�����ӹ��ܵĻ�������
void count_all_dir(Lines* res, int const board[BOARD_SIZE][BOARD_SIZE], int r, int c, int player); // ��ĳ�����ڵ��������ϵ����ӽ��м���
int adjacent_blank(Pos* pos, Game const* game, int row, int col, int mv_r, int mv_c);
// ʵ�����������add/removeԼ��ʱ1e-7s����
void add_and_recount(Game* game, int row, int col, int player); // ����һ�����ӣ�����������������
void del_and_recount(Game* game, int row, int col); // �Ƴ�����
