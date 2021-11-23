#include <string.h>
#include <sega_def.h>
#include <sega_dma.h>
#include <sega_mth.h>
#include <sega_scl.h>
#define	_SPR2_
#include <sega_spr.h>

#include "cd.h"
#include "print.h"
#include "scroll.h"
#include "sprite.h"

// where in VRAM each tilemap is
Uint32 vram[] = {
	SCL_VDP2_VRAM_A0, 
	SCL_VDP2_VRAM_A0 + 0x10000, 
	SCL_VDP2_VRAM_A0, 
	SCL_VDP2_VRAM_A0 + 0x10000,
    SCL_VDP2_VRAM_B1,
};

Uint32 charOffsets[] = {
	SCL_VDP2_VRAM_A1 - SCL_VDP2_VRAM, //NBG0
	SCL_VDP2_VRAM_B1 - SCL_VDP2_VRAM, //NBG1
	SCL_VDP2_VRAM_B1 - SCL_VDP2_VRAM, //NBG2
	SCL_VDP2_VRAM_B1 - SCL_VDP2_VRAM, //NBG3
};

/*
 * 0: NBG0 Pattern Name
 * 1: NBG1 Pattern Name
 * 2: NBG2 Pattern Name
 * 3: NBG3 Pattern Name
 * 4: NBG0 Character Pattern
 * 5: NBG1 Character Pattern
 * 6: NBG2 Character Pattern
 * 7: NBG3 Character Pattern
 * C: NBG0 Vertical Scroll Table
 * D: NBG1 Vertical Scroll Table
 * E: CPU Read/Write
 * F: No Access
 */

/*
 Data Type			# Accesses required
 Pattern name data          1
 16-color tiles		  		1
 256-color tiles	  		2
 2048-color tiles	  		4
 32K-color tiles	  		4
 16M-color tiles	  		8
 Vertical scroll data 		1
 */

// There's also numerous read restrictions, see SOA technical bulletin #6 for more information
//nbg 0/1/2 tilemaps in A0
//nbg 0/1/2 graphics in A1
Uint16	cycleTb[] = {
	0x012e,0xeeee,
	0x4455,0xee66,
	0xffff,0xffff,
	0xffff,0xffff
};

SclConfig scfg[5];

void Scroll_Init(void) {
	int i;
	Uint16 BackCol;
	SclVramConfig vramCfg;

	//wipe out vram
    for (int i = 0; i < 0x80000; i++) {
        ((volatile char *)SCL_VDP2_VRAM)[i] = 0;
    }
    SCL_AllocColRam(SCL_RBG0, 256, OFF);
	SCL_AllocColRam(SCL_NBG0, 256, OFF);
	SCL_AllocColRam(SCL_NBG1 | SCL_NBG2 | SCL_NBG3, 64, OFF);

	BackCol = 0x0000; //set the background color to black
	SCL_SetBack(SCL_VDP2_VRAM+0x80000-2,1,&BackCol);

	//setup VRAM configuration
	SCL_InitVramConfigTb(&vramCfg);
		vramCfg.vramModeA = ON; //separate VRAM A into A0 & A1
		vramCfg.vramModeB = ON; //separate VRAM B into B0 & B1
        vramCfg.vramB0 = SCL_RBG0_CHAR;
        vramCfg.vramB1 = SCL_RBG0_PN;
        vramCfg.colram = SCL_RBG0_K;
	SCL_SetVramConfig(&vramCfg);
    
    // NBG0
	SCL_InitConfigTb(&scfg[0]);
		scfg[0].dispenbl      = ON;
		scfg[0].charsize      = SCL_CHAR_SIZE_1X1;
		scfg[0].pnamesize     = SCL_PN1WORD;
		scfg[0].flip          = SCL_PN_10BIT;
		scfg[0].platesize     = SCL_PL_SIZE_2X1; //they meant "plane size"
		scfg[0].coltype       = SCL_COL_TYPE_256;
		scfg[0].datatype      = SCL_CELL;
		scfg[0].patnamecontrl = 0x0004; //vram A1 offset
		scfg[0].plate_addr[0] = vram[0];
		scfg[0].plate_addr[1] = vram[0] + 0x800;
	SCL_SetConfig(SCL_NBG0, &scfg[0]);
    
    // NBG1
	memcpy((void *)&scfg[1], (void *)&scfg[0], sizeof(SclConfig));
	scfg[1].dispenbl = ON;
	scfg[1].platesize = SCL_PL_SIZE_1X1;
	scfg[1].coltype = SCL_COL_TYPE_16;
	for(i=0;i<4;i++)   scfg[1].plate_addr[i] = vram[1];
	SCL_SetConfig(SCL_NBG1, &scfg[1]);

    // NBG2
	memcpy((void *)&scfg[2], (void *)&scfg[1], sizeof(SclConfig));
	scfg[2].dispenbl = OFF;
	for(i=0;i<4;i++)   scfg[2].plate_addr[i] = vram[2];
	SCL_SetConfig(SCL_NBG2, &scfg[2]);

    // NBG3
	memcpy((void *)&scfg[3], (void *)&scfg[2], sizeof(SclConfig));
	scfg[3].dispenbl = OFF;
	for(i=0;i<4;i++)   scfg[3].plate_addr[i] = vram[3];
	SCL_SetConfig(SCL_NBG3, &scfg[3]);

    // RBG0
    SCL_InitRotateTable(SCL_VDP2_VRAM_B1 + 0x10000, 1, SCL_RBG0, SCL_NON);
    SCL_InitConfigTb(&scfg[4]);
    scfg[4].dispenbl = ON;
    scfg[4].charsize = SCL_CHAR_SIZE_1X1;
    scfg[4].pnamesize = SCL_PN1WORD;
    scfg[4].flip = SCL_PN_10BIT;
    scfg[4].platesize = SCL_PL_SIZE_1X1;
    scfg[4].coltype = SCL_COL_TYPE_256;
    scfg[4].datatype = SCL_CELL;
    scfg[4].patnamecontrl = 0x0008;
    for (i = 0; i < 16; i++) scfg[4].plate_addr[i] = SCL_VDP2_VRAM_B1;
    SCL_SetConfig(SCL_RBG0, &scfg[4]);
	
	//setup vram access pattern
	SCL_SetCycleTable(cycleTb);
	 
	SCL_Open(SCL_NBG0);
		SCL_MoveTo(FIXED(0), FIXED(0), 0); //home position
	SCL_Close();
	SCL_Open(SCL_NBG1);
		SCL_MoveTo(FIXED(0), FIXED(0), 0);
	SCL_Close();
    SCL_Open(SCL_RBG_TB_A);
        SCL_MoveTo(FIXED(0), FIXED(0), 0);
    SCL_Close();

	Scroll_Scale(0, FIXED(1));
	Scroll_Scale(1, FIXED(1));

	SCL_SetPriority(SCL_SPR, 7);
	SCL_SetPriority(SCL_NBG0, 6);
	SCL_SetPriority(SCL_NBG1, 5);
	SCL_SetPriority(SCL_RBG0, 4);
}

int Scroll_LoadTile(void *src, volatile void *dest, Uint32 object, Uint16 palno) {
	Uint32 palLen;
	memcpy(&palLen, src, sizeof(palLen));
	src += 4;

    Uint32 palSize;
    memcpy(&palSize, src, sizeof(palSize));
    src += 4;

	SCL_SetColRam(object, palno, palLen, src);
	src += (palLen * palSize * 2);

	Uint32 image_size;
	memcpy(&image_size, src, sizeof(image_size));
	src += sizeof(Uint32);
	if (dest) {
		for (int i = 0; i < image_size; i++) {
			((volatile char *)dest)[i] = ((char *)src)[i];
		}
	}
	if (palLen == 16) {
		return image_size / 128;
	}
	else {
		return image_size / 256;
	}
}

char *Scroll_TilePtr(void *buff, int *size) {
	Sint32 palLen;
	memcpy(&palLen, buff, sizeof(palLen));
	buff += (palLen + 1) * sizeof(palLen);
	if (size) {
		memcpy(size, buff, sizeof(*size));
	}
	buff += sizeof(size);
	return (char *)buff;
}

char *Scroll_MapPtr(void *buff, int *xsize, int *ysize) {
	if (xsize) {
		memcpy(xsize, buff, sizeof(int));
	}
	buff += sizeof(int);
	if (ysize) {
		memcpy(ysize, buff, sizeof(int));
	}
	buff += sizeof(int);
	return (char *)buff;
}

#define ZOOM_HALF_NBG0 (0x1)
#define ZOOM_QUARTER_NBG0 (0x2)
#define ZOOM_HALF_NBG1 (0x100)
#define ZOOM_QUARTER_NBG1 (0x200)
#define LOW_BYTE (0xFF)
#define HIGH_BYTE (0xFF00)

void Scroll_Scale(int num, Fixed32 scale) {
	SCL_Open(1 << (num + 2));
		SCL_Scale(scale, scale);
	SCL_Close();
	//reset the configuration byte for the given background
	Scl_n_reg.zoomenbl &= (num == 0 ? HIGH_BYTE : LOW_BYTE);
	if (scale >= FIXED(1)) {
		Scl_n_reg.zoomenbl &= (num == 0 ? ~(ZOOM_HALF_NBG0 | ZOOM_QUARTER_NBG0)
										: ~(ZOOM_HALF_NBG1 | ZOOM_QUARTER_NBG1));
		return;
	}
	else if (scale < FIXED(1) && scale >= FIXED(0.5)) {
		Scl_n_reg.zoomenbl |= (num == 0 ? ZOOM_HALF_NBG0 : ZOOM_HALF_NBG1);
	}
	else {
		Scl_n_reg.zoomenbl |= (num == 0 ? ZOOM_QUARTER_NBG0 : ZOOM_QUARTER_NBG1);
	}
}

void Scroll_Set(int num, Fixed32 x, Fixed32 y) {
	SCL_Open(1 << (num + 2));
	SCL_MoveTo(x, y, 0);
	SCL_Close();
}

void Scroll_Move(int num, Fixed32 x, Fixed32 y) {
	SCL_Open(1 << (num + 2));
	SCL_Move(x, y, 0);
	SCL_Close();
}

void Scroll_ClearMaps(void) {
	memset(MAP_PTR(0), 0, 0x2000);
	memset(MAP_PTR(1), 0, 0x2000);
	memset(MAP_PTR(2), 0, 0x2000);
	memset(MAP_PTR(3), 0, 0x2000);
}

void Scroll_ClearVram(void) {
	for (int i = 0; i < 0x80000; i++) {
		((volatile Uint8 *)SCL_VDP2_VRAM)[i] = 0;
	}
}

void Scroll_CharSize(int num, Uint8 size) {
	scfg[num].charsize = size;
	SCL_SetConfig(1 << (num + 2), &scfg[num]);
}

void Scroll_Enable(int num, Uint8 state) {
	scfg[num].dispenbl = state;
	SCL_SetConfig(1 << (num + 2), &scfg[num]);
}

void Scroll_MapSize(int num, Uint8 size) {
	scfg[num].pnamesize = size;
	SCL_SetConfig(1 << (num + 2), &scfg[num]);
}
