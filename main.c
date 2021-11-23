#include <machine.h>
#include <stdint.h>
#include <string.h> //memcpy
#define	_SPR2_
#include <sega_spr.h>
#include <sega_scl.h> 
#include <sega_mth.h>
#include <sega_cdc.h>
#include <sega_sys.h>
#include <sega_per.h>
#include <sega_xpt.h>

#include "cd.h"
#include "devcart.h"
#include "game.h"
#include "release.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "print.h"
#include "vblank.h"

volatile Uint32 frame = 0;

typedef enum {
	STATE_NOTICE = 0,
	STATE_LOGO,
	STATE_INTRO,
	STATE_MENU,
	STATE_CUTSCENE,
	STATE_GAME,
	STATE_SOON,
} GAME_STATE;

int main() {
	frame = 0;

	CdcStat cd_status;
//
//	set_imask(0); // enable interrupts
//
	SCL_Vdp2Init();
//
	CD_Init();
	Sprite_Init();
	Scroll_Init();
	Print_Init();
	Print_Load();
	Sound_Init();
    Game_Init();

    CD_ChangeDir("BG");
    CD_Load("GARFIELD.TLE", (void *)LWRAM);
    Scroll_LoadTile((void *)LWRAM, (volatile void *)SCL_VDP2_VRAM_B0, SCL_RBG0, 0);
    CD_ChangeDir("..");
    
    for (int i = 0; i < 0x4000; i++) {
        ((volatile Uint16 *)SCL_VDP2_VRAM_B1)[i] = (i % 16) * 2;
    }
    
    while (1) {
		// if the cd drive is opened, return to menu (don't do this with devcart
		// because leaving the door open skips the boot animation which is nice)
		if (!DEVCART_LOAD) {
			CDC_GetPeriStat(&cd_status);
			if ((cd_status.status & 0xF) == CDC_ST_OPEN) {
				SYS_EXECDMP();
			}
		}

		//if player hits A+B+C+Start, return to menu
		if (PadData1 == (PAD_A | PAD_B | PAD_C | PAD_S)) {
			if (DEVCART_LOAD) {
				Devcart_Reset();
			}
			else {
				SYS_EXECDMP();
			}
		}

		Sprite_StartDraw();
        
        Game_Run();
       
        Fixed32 r = MTH_FIXED(2);

        if(PadData1){
			SCL_Open(SCL_RBG_TB_A);
			if(PadData1 & PAD_U)
				SCL_Move( 0, r, 0);
			else if(PadData1 & PAD_D)
				SCL_Move( 0,-r, 0);
			if(PadData1 & PAD_R)
				SCL_Move(-r, 0, 0);
			else if(PadData1 & PAD_L)
				SCL_Move( r, 0, 0);
			if(PadData1 & PAD_RB)
				SCL_Move( 0, 0,-r);
			else if(PadData1 & PAD_LB)
				SCL_Move( 0, 0, r);
			if((PadData1 & PAD_Z))
				SCL_Rotate(0,r,0);
			if((PadData1 & PAD_Y))
				SCL_Rotate(0,0,r);
			if((PadData1 & PAD_X))
				SCL_Rotate(r,0,0);
			if((PadData1 & PAD_S)) {
				SCL_MoveTo(0,0,0);
				SCL_RotateTo(0,0,0,SCL_X_AXIS);
			}
			SCL_Close();
		}
        
		Sprite_DrawAll();
		if (DEBUG) {
			Print_Display();
		}
		SPR_2CloseCommand();

		SCL_DisplayFrame();		// wait for vblank int to set flag to 0
	}

	return 0;
}
