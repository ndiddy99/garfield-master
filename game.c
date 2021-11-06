#include <sega_scl.h>

#include "block.h"
#include "cd.h"
#include "game.h"
#include "rng.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

static int blockStart;
static SPRITE_INFO blockSpr;

void Game_Init() {
    // load assets
    Uint8 *gameBuf = (Uint8 *)LWRAM;
    CD_ChangeDir("GAME");
    
    blockStart = Sprite_Load("BLOCKS.SPR", NULL); // sprites for active blocks

    CD_Load("PLACED.TLE", gameBuf);
    Scroll_LoadTile(gameBuf, (volatile void *)SCL_VDP2_VRAM_A1, SCL_NBG0, 0);
    volatile Uint16 *gameMap = (volatile Uint16 *)MAP_PTR(0);
    for (int i = 0; i < 2048; i++) {
        gameMap[i] = (i % 9) * 2;
    }
   
    CD_ChangeDir("..");

    // initialize the RNG
    RNG_Init();
}

static void Game_Draw(int block) {
    int tileNo;

    for (int y = 0; y < BLOCK_SIZE; y++) {
        for (int x = 0; x < BLOCK_SIZE; x++) {
            tileNo = blocks[block][0][y][x];
            if (tileNo != 0) {
                Sprite_Make(blockStart + tileNo, MTH_IntToFixed(64 + (x * 8)), MTH_IntToFixed(64 + (y * 8)), &blockSpr);
                Sprite_Draw(&blockSpr);
            }
        }
    }
}

int Game_Run() {
    static int drawPiece;
    if (PadData1E & PAD_A) {
        drawPiece = RNG_Get();
    }

    Game_Draw(drawPiece);
    return 0;
}
