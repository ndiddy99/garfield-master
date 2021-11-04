#ifndef SCROLL_H
#define SCROLL_H

#define MAP_PTR(bg) ((Uint16 *)vram[bg])
#define MAP_PTR32(bg) ((Uint32 *)vram[bg])
extern Uint32 vram[];

//number of tiles between A0 and A1
#define SCROLL_A1_OFFSET (0x1000)
//number of tiles between A0 and B1
#define SCROLL_B1_OFFSET (0x3000)

#define SCROLL_HEADER16 (72)
#define SCROLL_HEADER256 (1032)

void Scroll_Init(void);
// Loads a tile file into VRAM.
// src: where in RAM the tile file is
// dest: where in VRAM to load the tile file (or NULL for "don't load into vram")
// object: what screen to load the palette to
// palno: the color to load the palette from the file at
int Scroll_LoadTile(void *src, volatile void *dest, Uint32 object, Uint16 palno);
// Returns a pointer to the start of the tile file's graphics
char *Scroll_TilePtr(void *buff, int *size);
// Returns a pointer to the start of the map file's graphics
char *Scroll_MapPtr(void *buff, int *xsize, int *ysize);

void Scroll_Scale(int num, Fixed32 scale);
void Scroll_Set(int num, Fixed32 x, Fixed32 y);
void Scroll_Move(int num, Fixed32 x, Fixed32 y);
//zeroes out all tilemap vram
void Scroll_ClearMaps(void);
// zeroes out the entire vram
void Scroll_ClearVram(void);
//sets bg #num's character size
void Scroll_CharSize(int num, Uint8 size);
//enables/disable bg #num
void Scroll_Enable(int num, Uint8 state);
//sets bg #num's map size (either 1 word or 2 word)
void Scroll_MapSize(int num, Uint8 size);

#endif
