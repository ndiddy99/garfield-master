#include <sega_scl.h>

#include "cd.h"
#include "print.h"
#include "scroll.h"
#include "sound.h"
#include "vblank.h"

static volatile Uint8 *chrVram = (Uint8 *)SCL_VDP2_VRAM_B0;
static volatile Uint16 *mapVram = (Uint16 *)SCL_VDP2_VRAM_B1;

static int rank;
static int frames;

static char *ranks[] = {
    "NERMAL",
    "ODIE",
    "IRMA",
    "LYMAN",
    "JON",
    "LIZ",
    "ARLENE",
    "POOKY",
    "GARFIELD",
    "GOD"
};

#define RANKFONT_WIDTH (16)
#define MAP_WIDTH (32)

#define ASCII_A (65)
#define TILE_A (32)
#define TILE_SPACE (0)
#define TILE_COLON (11)
static void Rank_Print(char *string, int x, int y) {
    int xOffset = 0;
    int tileNo;
    while (*string != '\0') {
        switch (*string) {
            case ':':
                tileNo = TILE_COLON;
                break;

            case ' ':
                tileNo = TILE_SPACE;
                break;

            default:
                tileNo = *string - ASCII_A + TILE_A;
                break;
        }
        mapVram[(y * MAP_WIDTH) + x + xOffset] = tileNo * 2;
        xOffset++;
        string++;
    }
}

void Rank_Setup(int ranking) {
    rank = ranking;
}

void Rank_Init() {
    // clear out previous scroll data
    for (int i = 0; i < 0x40000; i++) {
        ((Uint8 *)SCL_VDP2_VRAM)[i] = 0;
    }

    for (int i = 0; i < 64 * 64; i++) {
        mapVram[i] = 0;
    }

    Print_Init();

    // reset scroll pos
    SCL_Open(SCL_RBG_TB_A);
    SCL_MoveTo(0, 0, 0);
    SCL_Close();

    CD_Load("RANKFONT.TLE", (void *)LWRAM);
    Scroll_LoadTile((void *)LWRAM, chrVram, SCL_RBG0, 0);
    
    Rank_Print("YOUR RANK:", 4, 4);
    frames = 0;
}

int Rank_Run() {
    frames++;
    if ((frames >= 600) || (PadData1E)) {
        return 1;
    }

    else if (frames == 200) {
        Rank_Print(ranks[rank], 4, 5);
        Sound_Play(SOUND_FALL);
        if (rank >= 9) {
            Sound_CDDA(GAMEOVER3_TRACK, 0);
        }
        else if (rank >= 5) {
            Sound_CDDA(GAMEOVER2_TRACK, 0);
        }
        else {
            Sound_CDDA(GAMEOVER1_TRACK, 0);
        }
    }

    return 0;
}
