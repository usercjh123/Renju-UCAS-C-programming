// 该模块用于寻找必胜策略，即VCT和VCF（统称VCX）
#pragma once
#include "game.h"

// 用于多线程搜索时传递参数
typedef struct AttackInfo {
	// 棋盘状态; 搜索深度; 攻击位置
	Game* game; int depth; Pos attack_pos;
	// 威胁类型
	int threat_type;
	// 防守状态状态
	int state;
	int max_depth;
} AttackInfo;

int attack(Game* game, int depth, Pos* attack_pos, int const max_depth);
// 多线程搜索
int multithreading_attack(Game* game, Pos* attack_pos, int const level, int const output);