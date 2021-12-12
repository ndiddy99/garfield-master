#include <sega_dma.h>
#include <sega_scl.h>
#include <string.h>

#include "cd.h"
#include "hwram.h"
#include "print.h"
#include "scroll.h"

#define ASCII_NUMBER_BASE (48)

#define BG_COUNT (7)
static Uint8 *bgAddrs[BG_COUNT];
static int bgLengths[BG_COUNT];

static SclRgb black;
static SclRgb normal;

static int currBG;

static Uint8 *chrVram = (Uint8 *)SCL_VDP2_VRAM_B0;
static Uint16 *mapVram = (Uint16 *)SCL_VDP2_VRAM_B1;

typedef enum {
    STATE_NONE = 0,
    STATE_BUFFER,
    STATE_FADEOUT,
    STATE_COPY,
} BG_STATE;

static int bgState;

// number of bytes to copy per frame
#define BUFFER_BYTES (128)
#define DMA_BYTES (512)
static int copyCursor;
static int copyOffset;

#define FADE_FRAMES (30)
static int frames;


void BG_Init() {
    // reset position
    SCL_Open(SCL_RBG0);
    SCL_MoveTo(0, 0, 0);
    SCL_RotateTo(0, 0, 0, SCL_X_AXIS);
    SCL_Close();

    black.red = -255; black.green = -255; black.blue = -255;
    normal.red = 0; normal.green = 0; normal.blue = 0;

    SCL_SetColOffset(SCL_OFFSET_A, SCL_RBG0 | SCL_NBG2, -255, -255, -255);
    
    // load all the backgrounds into LWRAM
    char filename[] = "n.TLE";
    Uint8 *cursor = (Uint8 *)LWRAM;
    CD_ChangeDir("BG");
    for (int i = 0; i < BG_COUNT; i++) {
        filename[0] = ASCII_NUMBER_BASE + i;
        int size = CD_Load(filename, cursor);
        bgAddrs[i] = cursor;
        bgLengths[i] = size;
        cursor += size;
    }
    CD_ChangeDir("..");

    // copy the first background to the screen
    Scroll_LoadTile(bgAddrs[0], chrVram, SCL_RBG0, 0);

    // set up the tilemap
    int counter = 0;
    for (int y = 0; y < (224 / 16); y++) {
        for (int x = 0; x < (320 / 16); x++) {
            mapVram[(y * 32) + x] = counter * 2;
            counter++;
        }
    }
    
    // fade in bg
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &black, &normal);

    // initialize the state
    bgState = STATE_BUFFER;
    copyCursor = 0;
    currBG = 1;
}

void BG_Run() {
    switch (bgState) {
        case STATE_BUFFER:
            //Print_String("BUFF", 0, 0);
            memcpy(&HWRAM_Buffer[copyCursor], bgAddrs[currBG] + copyCursor, BUFFER_BYTES);
            copyCursor += BUFFER_BYTES;
            if (copyCursor >= bgLengths[currBG]) {
                copyCursor = 0;
                bgState = STATE_NONE;
            }
            break;

        case STATE_FADEOUT:
            //Print_String("FADE", 0, 0);
            frames++;
            if (frames >= FADE_FRAMES) {
                copyCursor = 0;
                // start of the tile data
                copyOffset = (Uint8 *)Scroll_TilePtr(HWRAM_Buffer, NULL) - HWRAM_Buffer;
                bgState = STATE_COPY;
            }
            break;

        case STATE_COPY:
            //Print_String("COPY", 0, 0);
            DMA_ScuMemCopy(chrVram + copyCursor, &HWRAM_Buffer[copyCursor + copyOffset], DMA_BYTES);
            copyCursor += DMA_BYTES;
            if (copyCursor >= bgLengths[currBG]) {
                // load palette
                Scroll_LoadTile(HWRAM_Buffer, NULL, SCL_RBG0, 0);
                // fade in bg
                SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &black, &normal);
                currBG++;
                copyCursor = 0;
                if (currBG < BG_COUNT) {
                    bgState = STATE_BUFFER;
                }
                else {
                    bgState = STATE_NONE;
                }
            }
            break;

        case STATE_NONE:
            //Print_String("NONE", 0, 0);
            break;
    }
}

void BG_Next() {
    frames = 0;
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, FADE_FRAMES, &normal, &black);
    bgState = STATE_FADEOUT;
}

