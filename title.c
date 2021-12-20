#include <sega_scl.h>
#include "cd.h"
#include "print.h"
#include "scroll.h"
#include "sound.h"
#include "vblank.h"

static Uint8 *logoGfx;
static Uint8 *bobGfx;
static Uint8 *titleGfx;

typedef enum {
    STATE_LOGO_FADEIN = 0,
    STATE_LOGO_SHOW,
    STATE_LOGO_FADEOUT,
    STATE_BOB_FADEIN,
    STATE_BOB_SHOW,
    STATE_BOB_FADEOUT,
    STATE_TITLE_FADEIN,
    STATE_TITLE_SHOW,
    STATE_TITLE_FADEOUT
} TITLE_titleState;
static int titleState;
static int frames;

#define FADE_FRAMES (30)
#define SHOW_FRAMES (120)

static SclRgb black;
static SclRgb normal;

void Title_Init() {
    black.red = -255; black.green = -255; black.blue = -255;
    normal.red = 0; normal.green = 0; normal.blue = 0;

    SCL_SetColOffset(SCL_OFFSET_A, SCL_NBG0, -255, -255, -255);

    CD_ChangeDir("TITLE");
    Uint8 *cursor = (Uint8 *)LWRAM;
    logoGfx = cursor;
    cursor += CD_Load("LOGO.TLE", cursor);
    bobGfx = cursor;
    cursor += CD_Load("BOB.TLE", cursor);
    titleGfx = cursor;
    cursor += CD_Load("TITLE.TLE", cursor);
    CD_ChangeDir("..");
    Sound_CDDA(TITLE_TRACK, 1);

    // set up map
    int counter = 1;
    for (int y = 0; y < (224 / 8); y++) {
        for (int x = 0; x < (320 / 8); x++) {
            MAP_PTR(0)[y * 64 + x] = counter * 2;
            counter++;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        ((volatile Uint8 *)SCL_VDP2_VRAM_A1)[i] = 0;
    }
    Scroll_LoadTile(logoGfx, (volatile void *)SCL_VDP2_VRAM_A1 + 64, SCL_NBG0, 0);

    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &black, &normal);
    titleState = STATE_LOGO_FADEIN;
    frames = 0;
}

int Title_Run() {
    switch (titleState) {
        case STATE_LOGO_FADEIN:
            frames++;
            if (frames >= SHOW_FRAMES) {
                frames = 0;
                titleState = STATE_LOGO_SHOW;
            }
            break;

        case STATE_LOGO_SHOW:
            frames++;
            if (frames >= SHOW_FRAMES) {
                frames = 0;
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &normal, &black);
                titleState = STATE_LOGO_FADEOUT;
            }
            break;

        case STATE_LOGO_FADEOUT:
            frames++;
            if (frames >= SHOW_FRAMES) {
                frames = 0;
                Scroll_LoadTile(bobGfx, (volatile void *)SCL_VDP2_VRAM_A1 + 64, SCL_NBG0, 0);
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &black, &normal);
                titleState = STATE_BOB_FADEIN;
            }
            break;
        
        case STATE_BOB_FADEIN:
            frames++;
            if (frames >= SHOW_FRAMES) {
                frames = 0;
                titleState = STATE_BOB_SHOW;
            }
            break;

        case STATE_BOB_SHOW:
            frames++;
            if (frames >= SHOW_FRAMES) {
                frames = 0;
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &normal, &black);
                titleState = STATE_BOB_FADEOUT;
            }
            break;

        case STATE_BOB_FADEOUT:
            frames++;
            if (frames >= SHOW_FRAMES) {
                frames = 0;
                Scroll_LoadTile(titleGfx, (volatile void *)SCL_VDP2_VRAM_A1 + 64, SCL_NBG0, 0);
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &black, &normal);
                titleState = STATE_TITLE_FADEIN;
            }
            break;
        
        case STATE_TITLE_FADEIN:
            frames++;
            if (frames > FADE_FRAMES) {
                frames = 0;
                titleState = STATE_TITLE_SHOW;
            }
            break;

        case STATE_TITLE_SHOW:
            if (PadData1) {
                frames = 0;
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &normal, &black);
                titleState = STATE_TITLE_FADEOUT;
            }
            break;

        case STATE_TITLE_FADEOUT:
            frames++;
            if (frames >= SHOW_FRAMES) {
                return 1;
            }
            break;
    }
    
    return 0;
}

