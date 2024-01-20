// ��ģ������Ѱ�ұ�ʤ���ԣ���VCT��VCF��ͳ��VCX��
#pragma once
#include "game.h"

// ���ڶ��߳�����ʱ���ݲ���
typedef struct AttackInfo {
	// ����״̬; �������; ����λ��
	Game* game; int depth; Pos attack_pos;
	// ��в����
	int threat_type;
	// ����״̬״̬
	int state;
	int max_depth;
} AttackInfo;

int attack(Game* game, int depth, Pos* attack_pos, int const max_depth);
// ���߳�����
int multithreading_attack(Game* game, Pos* attack_pos, int const level, int const output);