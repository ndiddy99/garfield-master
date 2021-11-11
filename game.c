#include <sega_scl.h>

#include "cd.h"
#include "game.h"
#include "piece.h"
#include "print.h"
#include "rng.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

static int blockStart;
static SPRITE_INFO blockSpr;

#define GAME_ROWS (20)
#define GAME_COLS (10)
static int gameBoard[GAME_ROWS][GAME_COLS];

#define ROW_OFFSET (64)
#define TILE_SIZE (8)
#define BOARD_ROW (4)
#define BOARD_COL (8)
volatile Uint16 *boardVram;

#define SPAWN_X (4)
#define SPAWN_Y (-1)
static PIECE currPiece;

#define MOVE_FRAMES (10)
#define DAS_FRAMES (2)
static int leftTimer;
static int rightTimer;

#define ROTATE_CLOCKWISE (-1)
#define ROTATE_COUNTERCLOCKWISE (1)

// initializes a new piece
static void Game_MakePiece(PIECE *piece) {
    piece->num = RNG_Get();
    piece->rotation = 0;
    piece->x = SPAWN_X;
    piece->y = SPAWN_Y;
}

void Game_Init() {
    // load assets
    Uint8 *gameBuf = (Uint8 *)LWRAM;
    CD_ChangeDir("GAME");
    
    blockStart = Sprite_Load("BLOCKS.SPR", NULL); // sprites for active blocks

    CD_Load("PLACED.TLE", gameBuf);
    Scroll_LoadTile(gameBuf, (volatile void *)SCL_VDP2_VRAM_A1, SCL_NBG0, 0);
    boardVram = (volatile Uint16 *)MAP_PTR(0) + (BOARD_ROW * ROW_OFFSET) + BOARD_COL;
   
    CD_ChangeDir("..");

    // initialize the RNG
    RNG_Init();

    // initialize the board
    for (int y = 0; y < GAME_ROWS; y++) {
        for (int x = 0; x < GAME_COLS; x++) {
            gameBoard[y][x] = 0;
        }
    }

    leftTimer = MOVE_FRAMES;
    rightTimer = MOVE_FRAMES;

    // set the first piece
    Game_MakePiece(&currPiece);
}

// draws a piece
static void Game_DrawPiece(PIECE *piece) {
    int tileNo;

    for (int y = 0; y < PIECE_SIZE; y++) {
        for (int x = 0; x < PIECE_SIZE; x++) {
            tileNo = pieces[piece->num][piece->rotation][y][x];
            if (tileNo != 0) {
                // subtract 1 from the sprite number because the piece arrays have the first
                // block sprite as 1 and 0 as "nothing"
                Sprite_Make(blockStart + tileNo - 1, MTH_IntToFixed((BOARD_COL + piece->x + x) * TILE_SIZE),
                        MTH_IntToFixed((BOARD_ROW + piece->y + y) * TILE_SIZE), &blockSpr);
                Sprite_Draw(&blockSpr);
            }
        }
    }
}

// copies a piece to the board
static void Game_CopyPiece(PIECE *piece) {
    int tile;

    for (int y = 0; y < PIECE_SIZE; y++) {
        for (int x = 0; x < PIECE_SIZE; x++) {
            tile = pieces[piece->num][piece->rotation][y][x];
            if (((piece->x + x) >= 0) && ((piece->y + y) >= 0) && (tile != 0)) {
                gameBoard[piece->y + y][piece->x + x] = tile;
            }
        }
    }
}

static inline int Game_BoardGet(int x, int y) {
    if ((y < 0) || (y >= GAME_ROWS)) {
        return 1;
    }

    if ((x < 0) || (x >= GAME_COLS)) {
        return 1;
    }

    return gameBoard[y][x];
}

// returns 1 if a piece can fit on the board
static int Game_CheckPiece(PIECE *piece) {
    for (int y = 0; y < PIECE_SIZE; y++) {
        for (int x = 0; x < PIECE_SIZE; x++) {
            if (Game_BoardGet(piece->x + x, piece->y + y) &&
                    pieces[piece->num][piece->rotation][y][x]) {
                return 0;
            }
        }
    }

    return 1;
}

// checks the center column rule for kicks
static int Game_CanKick(PIECE *piece) {
    if ((piece->num == PIECE_L) || (piece->num == PIECE_J) || (piece->num == PIECE_T)) {
        for (int y = 0; y < PIECE_SIZE; y++) {
            for (int x = 0; x < PIECE_SIZE; x++) {
                if (Game_BoardGet(piece->x + x, piece->y + y) &&
                        pieces[piece->num][piece->rotation][y][x]) {
                    if (x == 1) {
                        return 0;
                    }
                    else {
                        return 1;
                    }
                }
            }
        }
    }

    return 1;
}

static int Game_CanMoveLeft() {
    if (PadData1E & PAD_L) {
        leftTimer = MOVE_FRAMES;
        return 1;
    }

    else if (PadData1 & PAD_L) {
        if (leftTimer == 0) {
            leftTimer = DAS_FRAMES;
            return 1;
        }
        else {
            leftTimer--;
        }
    }
    return 0;
}

static int Game_CanMoveRight() {
    if (PadData1E & PAD_R) {
        rightTimer = MOVE_FRAMES;
        return 1;
    }

    else if (PadData1 & PAD_R) {
        if (rightTimer == 0) {
            rightTimer = DAS_FRAMES;
            return 1;
        }
        else {
            rightTimer--;
        }
    }
    return 0;
}

static void Game_Rotate(PIECE *piece, int rotation) {
    Uint8 originalRotation = piece->rotation;
    piece->rotation += rotation;
    piece->rotation %= PIECE_ROTATIONS;
    if (Game_CheckPiece(piece)) {
        return;
    }

    if (Game_CanKick(piece)) {
        // try going to the left
        piece->x++;
        if (Game_CheckPiece(piece)) {
            return;
        }

        // try going to the left
        piece->x -= 2;
        if (Game_CheckPiece(piece)) {
            return;
        }

        // move piece back to where it originally was
        piece->x++;
    }
    
    // rotate back
    piece->rotation = originalRotation;
}

int Game_Run() {
    if (PadData1E & PAD_A) {
        Game_CopyPiece(&currPiece);
        Game_MakePiece(&currPiece);
    }

    // clockwise rotation
    if (PadData1E & PAD_C) {
        Game_Rotate(&currPiece, ROTATE_CLOCKWISE);
    }

    // counterclockwise rotation
    if (PadData1E & PAD_B) {
        Game_Rotate(&currPiece, ROTATE_COUNTERCLOCKWISE);
    }
    currPiece.rotation %= PIECE_ROTATIONS;

    if (PadData1E & PAD_D) {
        currPiece.y++;
        if (!Game_CheckPiece(&currPiece)) {
            currPiece.y--;
        }
    }

    if (Game_CanMoveLeft()) {
        currPiece.x--;
        if (!Game_CheckPiece(&currPiece)) {
            currPiece.x++;
        }
    }

    if (Game_CanMoveRight()) {
        currPiece.x++;
        if (!Game_CheckPiece(&currPiece)) {
            currPiece.x--;
        }
    }

    Print_Num(currPiece.x, 2, 0);
    Print_Num(currPiece.y, 3, 0);
    Print_Num(currPiece.rotation, 4, 0);
        

    // Game_Draw(drawPiece);
    Game_DrawPiece(&currPiece);

    // copy the board to VRAM
    for (int y = 0; y < GAME_ROWS; y++) {
        for (int x = 0; x < GAME_COLS; x++) {
            boardVram[(y * ROW_OFFSET) + x] = (gameBoard[y][x] * 2);
        }
    }
    return 0;
}
