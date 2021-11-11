#ifndef GAME_H
#define GAME_H

#include <sega_mth.h>

typedef struct {
    int x;
    int y;
    Uint8 num;
    Uint8 rotation;
} PIECE;

// reloads all assets & initializes game
void Game_Init();

// advances gameplay by one frame
int Game_Run();

#endif

