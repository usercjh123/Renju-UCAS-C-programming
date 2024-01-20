// �ṩһЩ���õĺ궨��
#pragma once

#define INF 0xffffff
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define IN_RANGE(x, d) ((x) >= 0 && (x) < (d))
#define SET_POS(pos, r, c) (pos).row = (r);(pos).col = (c);
// �������������ڼ���㵽�߽�ľ���
#define DIST(x, d, mv) ((mv) == 0 ? (d) : ((mv) == 1 ? (d) - (x) - 1 : (x)))
#define MAX_STEP(row, col, mv_r, mv_c) MIN(DIST((row), BOARD_SIZE, (mv_r)), DIST((col), BOARD_SIZE, (mv_c)))
#define P2I(p) (((p) + 1) >> 1) // -1 -> 0; 1 -> 1
#define I2P(i) (((i) << 1) - 1) // 0 -> -1; 1 -> 1
#define for_all_dir(dir) for((dir) = HORZ ; (dir) <= sDIAG ; ++(dir)) // ���ڱ������з���