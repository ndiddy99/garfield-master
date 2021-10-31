#ifndef SPRITE_H
#define SPRITE_H

#include <sega_def.h>
#include <sega_mth.h>

#define MIRROR_HORIZ (1 << 4)
#define MIRROR_VERT (1 << 5)

struct SpriteInfo;
typedef struct SpriteInfo SPRITE_INFO;

typedef void (*IterateFunc)(SPRITE_INFO *);

#define SPRITE_DATA_SIZE (12)

typedef struct SpriteInfo {
	Uint16 display;
	Uint16 charNum; //tile number
	Uint16 index; //where the sprite is in the sprites array
	Fixed32 x;
	Fixed32 y;
	Fixed32 xSize;
	Fixed32 ySize;
	Fixed32 scale;
	Fixed32 angle;
	Uint16 mirror;
	SPRITE_INFO *prev; // for iterating through a certain type of sprite
	SPRITE_INFO *next;
	Uint8 data[SPRITE_DATA_SIZE] __attribute__((aligned(4)));
	IterateFunc iterate;
} SPRITE_INFO;

#define SPRITE_LIST_SIZE (80)
extern SPRITE_INFO sprites[];

//sets up initial sprite display
void Sprite_Init(void);
//erases framebuffer (run at start of draw command)
void Sprite_Erase(Sint16 x, Sint16 y);
// clears vdp1 memory
void Sprite_Clear(void);
// loads a sprite off the disc, returns the tile number of the first loaded sprite
// count: optional parameter, if it's not null gets set to # of sprites loaded
int Sprite_Load(char *filename, int *count);
//gets vdp1 ready for draw commands
void Sprite_StartDraw(void);
//automatically picks the simplest SBL function for drawing the sprite depending
//on required features
//needs command to be opened before calling
void Sprite_Draw(SPRITE_INFO *info);
//inits the SPRITE_INFO pointer given
void Sprite_Make(int tile_num, Fixed32 x, Fixed32 y, SPRITE_INFO *ptr);
//draws all sprites in the sprite list
void Sprite_DrawAll(void);
//gets a pointer to the next sprite in the list
SPRITE_INFO *Sprite_Next(void);
// adds the sprite to a doubly linked list
void Sprite_ListAdd(SPRITE_INFO **head, SPRITE_INFO *sprite);
// removes the sprite from a doubly linked list
void Sprite_ListRemove(SPRITE_INFO **head, SPRITE_INFO *sprite);
//deletes the given sprite from the sprite list
void Sprite_Delete(SPRITE_INFO *sprite);
//deletes all sprites from the list
void Sprite_DeleteAll(void);

#endif
