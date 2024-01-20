#include <time.h>
#include "init.h"
#include "game.h"
#include "mt19937.h"

void init_all(Game* game)
{
	initialize_game(game);
	mt_initialize((uint64)time(0));
	zobrist_initialize();
}
