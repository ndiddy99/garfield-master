#include <sega_def.h>
#include <sega_mth.h>
#include <machine.h>
#define _SPR2_
#include <sega_spr.h>
#include <sega_scl.h>
#include <string.h>

#include "cd.h"
#include "scroll.h"
#include "sprite.h"
#include "vblank.h"

int numSprites = 0;

int tileCount = 0;
int palCnt = 0;
SPRITE_INFO sprites[SPRITE_LIST_SIZE];
//normalize diagonal speed
#define DIAGONAL_MULTIPLIER (MTH_FIXED(0.8))

#define CommandMax    300
#define GourTblMax    300
#define LookupTblMax  100
#define CharMax       256 //CHANGE WHEN YOU INCREASE TILES BEYOND THIS POINT
#define DrawPrtyMax   256
SPR_2DefineWork(work2D, CommandMax, GourTblMax, LookupTblMax, CharMax, DrawPrtyMax)
#define ENDCODE_DISABLE (1 << 7)

static Uint8 spriteBuf[65536];
static Uint16 red[64];

void Sprite_Init() {
	Sprite_DeleteAll();

	SCL_Vdp2Init();
	SPR_2Initial(&work2D);
	SPR_2SetTvMode(SPR_TV_NORMAL, SPR_TV_320X224, OFF);
	SCL_SetColRamMode(SCL_CRM15_2048);
    SCL_SetSpriteMode(SCL_TYPE5, SCL_MIX, SCL_SP_WINDOW);
	SCL_AllocColRam(SCL_SPR, 256, OFF);

	SetVblank(); //setup vblank routine
	set_imask(0);
	
	SPR_2FrameChgIntr(1); //wait until next frame to set color mode
	SPR_2FrameEraseData(RGB16_COLOR(0, 0, 0)); //zero out frame
	SCL_DisplayFrame();
}

void Sprite_Clear() {
	tileCount = 0;
	palCnt = 0;
	SPR_2ClrAllChar();
}

static int Sprite_LoadPal(Uint8 *buffer, int *count) {
	Sint32 numPals;
	memcpy(&numPals, buffer, sizeof(numPals));
	buffer += sizeof(numPals);

	// load all the palettes
	for (int i = 0; i < numPals; i++) {
		SCL_SetColRam(SCL_SPR, i + palCnt, 16, buffer);
		buffer += 16 * sizeof(Sint32); // move to next palette
	}

	// first 4 bytes after palettes is the number of sprites
	Sint32 numSprites;
	memcpy(&numSprites, buffer, sizeof(numSprites));
	buffer += sizeof(numSprites);

	// load all the sprites
	Sint32 spriteX;
	Sint32 spriteY;
	Sint32 spritePal;
	for (int i = 0; i < numSprites; i++) {
		memcpy(&spriteX, buffer, sizeof(spriteX));
		buffer += sizeof(spriteX);
		memcpy(&spriteY, buffer, sizeof(spriteY));
        buffer += sizeof(spriteY);
		memcpy(&spritePal, buffer, sizeof(spritePal));
		spritePal = (spritePal * 16) + palCnt;
		buffer += sizeof(spritePal);
		SPR_2SetChar((Uint16)(i + tileCount), COLOR_0, (Uint16)(spritePal),
		  (Uint16)spriteX, (Uint16)spriteY, buffer);
		buffer += ((spriteX / 2) * spriteY);
	}
	int sprite_tilebak = tileCount;
	tileCount += numSprites;
	palCnt += (numPals * 16);
	if (count) {
		*count = numSprites;
	}
	return sprite_tilebak;
}

static int Sprite_LoadRGB(Uint8 *buffer, int *count) {
	// first 4 bytes is the number of sprites
	Sint32 numSprites;
	memcpy(&numSprites, buffer, sizeof(numSprites));
	buffer += sizeof(numSprites);

	// load all the sprites
	Sint32 spriteX;
	Sint32 spriteY;
	for (int i = 0; i < numSprites; i++) {
		memcpy(&spriteX, buffer, sizeof(spriteX));
		buffer += sizeof(spriteX);
		memcpy(&spriteY, buffer, sizeof(spriteY));
		buffer += sizeof(spriteY);
		SPR_2SetChar((Uint16)(i + tileCount), COLOR_5, 0,
		  (Uint16)spriteX, (Uint16)spriteY, buffer);
		buffer += (spriteX * spriteY * 2);
	}
	int sprite_tilebak = tileCount;
	tileCount += numSprites;
	if (count) {
		*count = numSprites;
	}
	return sprite_tilebak;
}

int Sprite_Load(char *filename, int *count) {
	CD_Load(filename, spriteBuf);
    Sint32 type;

    for (int i = 0; i < 64; i++) {
        red[i] = RGB16_COLOR(31, 31, 31);
    }

    memcpy(&type, spriteBuf, sizeof(type));
    if (type == 0) {
        return Sprite_LoadPal(spriteBuf + sizeof(type), count);
    }
    else {
        return Sprite_LoadRGB(spriteBuf + sizeof(type), count);
    }
}


void Sprite_StartDraw(void) {
	XyInt xy;

	SPR_2OpenCommand(SPR_2DRAW_PRTY_OFF);
	xy.x = 320;
	xy.y = 240;
	SPR_2SysClip(0, &xy);
	// sprite_erase(xy.x, xy.y);
}

void Sprite_Draw(SPRITE_INFO *info) {
	XyInt xy[4];
	Fixed32 xOffset, yOffset, sin, cos, scaledX, scaledY;

	if (info->scale == MTH_FIXED(1) && info->angle == 0) {
		xy[0].x = (Sint16)MTH_FixedToInt(info->x);
		xy[0].y = (Sint16)MTH_FixedToInt(info->y);
		SPR_2NormSpr(0, info->mirror, COLOR_5 | ENDCODE_DISABLE, 0, info->charNum, xy, NO_GOUR); // rgb normal sprite
	}
	
	else if (info->angle == 0){	
		xy[0].x = (Sint16)MTH_FixedToInt(info->x);
		xy[0].y = (Sint16)MTH_FixedToInt(info->y);
		//the way scale works is by giving the x/y coordinates of the top left and
		//bottom right corner of the sprite
		xy[1].x = (Sint16)(MTH_FixedToInt(MTH_Mul(info->xSize, info->scale) + info->x));
		xy[1].y = (Sint16)(MTH_FixedToInt(MTH_Mul(info->ySize, info->scale) + info->y));
		SPR_2ScaleSpr(0, info->mirror, COLOR_5 | ENDCODE_DISABLE, 0, info->charNum, xy, NO_GOUR); // rgb scaled sprite
	}
	
	else {
		//offset of top left sprite corner from the origin
		xOffset = -(MTH_Mul(info->xSize >> 1, info->scale));
		yOffset = -(MTH_Mul(info->ySize >> 1, info->scale));
		sin = MTH_Sin(info->angle);
		cos = MTH_Cos(info->angle);
		scaledX = info->x + MTH_Mul(info->xSize >> 1, info->scale);
		scaledY = info->y + MTH_Mul(info->ySize >> 1, info->scale);
		//formula from
		//https://gamedev.stackexchange.com/questions/86755/
		for (int i = 0; i < 4; i++) {
			if (i == 1) xOffset = -xOffset; //upper right
			if (i == 2) yOffset = -yOffset; //lower right
			if (i == 3) xOffset = -xOffset; //lower left
			xy[i].x = (Sint16)MTH_FixedToInt(MTH_Mul(xOffset, cos) - 
				MTH_Mul(yOffset, sin) + scaledX);
			xy[i].y = (Sint16)MTH_FixedToInt(MTH_Mul(xOffset, sin) +
				MTH_Mul(yOffset, cos) + scaledY);
		}
		SPR_2DistSpr(0, info->mirror, COLOR_5 | ENDCODE_DISABLE, 0, info->charNum, xy, NO_GOUR); // rgb distorted sprite
	}
}

void Sprite_Make(int tileNum, Fixed32 x, Fixed32 y, SPRITE_INFO *ptr) {
	ptr->display = 1;
	ptr->charNum = tileNum;
	ptr->x = x;
	ptr->y = y;
	ptr->xSize = 0;
	ptr->ySize = 0;
	ptr->mirror = 0;
	ptr->scale = MTH_FIXED(1);
	ptr->angle = 0;
	ptr->prev = NULL;
	ptr->next = NULL;
	ptr->iterate = NULL;
}

void Sprite_DrawAll() {
	for (int i = 0; i < SPRITE_LIST_SIZE; i++) {
		if (sprites[i].display) {
			if (sprites[i].iterate != NULL) {
				sprites[i].iterate(&sprites[i]);
			}
			//check again because iterate function may have deleted sprite
			if (sprites[i].display) {
				Sprite_Draw(&sprites[i]);
			}
		}
	}
}

SPRITE_INFO *Sprite_Next() {
	int i;
	for (i = 0; i < SPRITE_LIST_SIZE; i++) {
		if (!sprites[i].display) {
			numSprites++;
			sprites[i].index = i;
			sprites[i].iterate = NULL;
			return &sprites[i];
		}
	}
	return NULL;
}

void Sprite_ListAdd(SPRITE_INFO **head, SPRITE_INFO *sprite) {
	sprite->next = *head;
	sprite->prev = NULL;
	if (*head != NULL) {
		(*head)->prev = sprite;
	}
	*head = sprite;
}

void Sprite_ListRemove(SPRITE_INFO **head, SPRITE_INFO *sprite) {
	if (*head == sprite) {
		*head = sprite->next;
	}

	if (sprite->next != NULL) {
		sprite->next->prev = sprite->prev;
	}

	if (sprite->prev != NULL) {
		sprite->prev->next = sprite->next;
	}
}

void Sprite_Delete(SPRITE_INFO *sprite) {
	sprite->display = 0;
	sprite->iterate = NULL;
	numSprites--;
}

void Sprite_DeleteAll() {
	for (int i = 0; i < SPRITE_LIST_SIZE; i++) {
		sprites[i].display = 0;
	}
	numSprites = 0;
}

