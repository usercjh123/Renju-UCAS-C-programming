// ��ģ�������ж�ʤ�������
#pragma once
#include "game.h"

// ����ֵ��NORMAL_PLAY������в��, FORBIDDEN_PLAY�����֣�, ONE/TWO_THREAT_PLAY������в��, GAME_OVER��playerʤ����
int check(Game const* game, Pos const* pos, int forb);