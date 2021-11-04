#include <sega_scl.h>

#include "cd.h"
#include "game.h"
#include "scroll.h"
#include "sprite.h"

int blockNum;
SPRITE_INFO blockSpr;
void Game_Init() {
    Uint8 *gameBuf = (Uint8 *)LWRAM;
    
    CD_ChangeDir("GAME");
    
    blockNum = Sprite_Load("BLOCKS.SPR", NULL); // sprites for active blocks

    CD_Load("PLACED.TLE", gameBuf);
    Scroll_LoadTile(gameBuf, (volatile void *)SCL_VDP2_VRAM_A1, SCL_NBG0, 0);
    volatile Uint16 *gameMap = (volatile Uint16 *)MAP_PTR(0);

    for (int i = 0; i < 2048; i++) {
        gameMap[i] = (i & 7) * 2;
    }
   
    CD_ChangeDir("..");
}

int Game_Run() {
    Sprite_Make(blockNum + 1, MTH_FIXED(64), MTH_FIXED(64), &blockSpr);
    Sprite_Draw(&blockSpr);
    return 0;
}
