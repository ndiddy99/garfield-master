#ifndef GAME_H
#define GAME_H

#include <sega_mth.h>

typedef enum {
    STATE_AIR = 0,
    STATE_GROUND,
} PIECE_STATE;

typedef struct {
    int x;
    int y;
    Uint8 num;
    Uint8 rotation;
    PIECE_STATE state;
} PIECE;

// reloads all assets & initializes game
void Game_Init();

// advances gameplay by one frame
int Game_Run();

#endif

