// ��ģ����������
#pragma once
#include "game.h"

#define SCORE_GAME_OVER 10000

int eval_score(Game const* game, int player); // ��ȡ��������
void scoring_board(Game const* game, int score_map[2][BOARD_SIZE][BOARD_SIZE]); // Ϊ�����ϵ����пյ�����ʽ���