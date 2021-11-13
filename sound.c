#include <sega_cdc.h>
#include <sega_xpt.h>
#include <string.h>

#include "cd.h"
#include "sound.h"
#include "pcmsys.h"
#include "print.h"
#include "release.h"

static void sound_external_audio_enable(Uint8 vol_l, Uint8 vol_r) {
    volatile Uint16 *slot_ptr;

    //max sound volume is 7
    if (vol_l > 7) {
        vol_l = 7;
    }
    if (vol_r > 7) {
        vol_r = 7;
    }

    // Setup SCSP Slot 16 and Slot 17 for playing
    slot_ptr = (volatile Uint16 *)(0x25B00000 + (0x20 * 16));
    slot_ptr[0] = 0x1000;
    slot_ptr[1] = 0x0000; 
    slot_ptr[2] = 0x0000; 
    slot_ptr[3] = 0x0000; 
    slot_ptr[4] = 0x0000; 
    slot_ptr[5] = 0x0000; 
    slot_ptr[6] = 0x00FF; 
    slot_ptr[7] = 0x0000; 
    slot_ptr[8] = 0x0000; 
    slot_ptr[9] = 0x0000; 
    slot_ptr[10] = 0x0000; 
    slot_ptr[11] = 0x001F | (vol_l << 5);
    slot_ptr[12] = 0x0000; 
    slot_ptr[13] = 0x0000; 
    slot_ptr[14] = 0x0000; 
    slot_ptr[15] = 0x0000; 

    slot_ptr = (volatile Uint16 *)(0x25B00000 + (0x20 * 17));
    slot_ptr[0] = 0x1000;
    slot_ptr[1] = 0x0000; 
    slot_ptr[2] = 0x0000; 
    slot_ptr[3] = 0x0000; 
    slot_ptr[4] = 0x0000; 
    slot_ptr[5] = 0x0000; 
    slot_ptr[6] = 0x00FF; 
    slot_ptr[7] = 0x0000; 
    slot_ptr[8] = 0x0000; 
    slot_ptr[9] = 0x0000; 
    slot_ptr[10] = 0x0000; 
    slot_ptr[11] = 0x000F | (vol_r << 5);
    slot_ptr[12] = 0x0000; 
    slot_ptr[13] = 0x0000; 
    slot_ptr[14] = 0x0000; 
    slot_ptr[15] = 0x0000;

    *((volatile Uint16 *)(0x25B00400)) = 0x020F;
}

//must be called after CD_Init
void sound_init() {
    if (DEVCART_LOAD == 0) {
        sound_external_audio_enable(5, 5);
    }
    load_drv();
    CD_ChangeDir("SFX");
    load_8bit_pcm("I.RAW", 11025);
    load_8bit_pcm("Z.RAW", 11025);
    load_8bit_pcm("S.RAW", 11025);
    load_8bit_pcm("J.RAW", 11025);
    load_8bit_pcm("L.RAW", 11025);
    load_8bit_pcm("O.RAW", 11025);
    load_8bit_pcm("T.RAW", 11025);
    CD_ChangeDir("..");
}


void sound_cdda(int track, int loop) {
    if (DEVCART_LOAD) {
        return;
    }
    CdcPly ply;
    CDC_PLY_STYPE(&ply) = CDC_PTYPE_TNO; //track number
	CDC_PLY_STNO(&ply)  = track;
	CDC_PLY_SIDX(&ply) = 1;
	CDC_PLY_ETYPE(&ply) = CDC_PTYPE_TNO;
	CDC_PLY_ETNO(&ply)  = track;
	CDC_PLY_EIDX(&ply) = 99;
    if (loop) {
        CDC_PLY_PMODE(&ply) = CDC_PM_DFL | 0xf; //0xf = infinite repetitions
    }
    else {
        CDC_PLY_PMODE(&ply) = CDC_PM_DFL;
    }
	
    CDC_CdPlay(&ply);
}

void sound_play(short num) {
    pcm_play(num, PCM_SEMI, 6);
}

