#include <sega_cdc.h>
#include <sega_xpt.h>
#include <string.h>

#include "cd.h"
#include "sound.h"
#include "print.h"
#include "release.h"

typedef enum {
    STATE_HALT = 0,
    STATE_PLAY
} DRIVER_STATE;

typedef struct {
    Uint32 state; // what state the driver is in
    Uint8 *track_addr; // where the song to play is
    Uint32 ack; // if the 68k has acknowledged the saturn
} DRIVER_COMMS;

#define SMPC_REG_IREG(i)        (*((volatile unsigned char *)0x20100001+((i) * 2)))
#define SMPC_REG_COMREG         (*((volatile unsigned char *)0x2010001F))
#define SMPC_REG_OREG(o)        (*((volatile unsigned char *)0x20100021+((o) * 2)))
#define SMPC_REG_SR             (*((volatile unsigned char *)0x20100061))
#define SMPC_REG_SF             (*((volatile unsigned char *)0x20100063))
#define SMPC_REG_PDR1           (*((volatile unsigned char *)0x20100075))
#define SMPC_REG_PDR2           (*((volatile unsigned char *)0x20100077))
#define SMPC_REG_DDR1           (*((volatile unsigned char *)0x20100079))
#define SMPC_REG_DDR2           (*((volatile unsigned char *)0x2010007B))
#define SMPC_REG_IOSEL          (*((volatile unsigned char *)0x2010007D))
#define SMPC_REG_EXLE           (*((volatile unsigned char *)0x2010007F))

#define SMPC_CMD_MSHON              0x00
#define SMPC_CMD_SSHON              0x02
#define SMPC_CMD_SSHOFF             0x03
#define SMPC_CMD_SNDON              0x06
#define SMPC_CMD_SNDOFF             0x07
#define SMPC_CMD_CDON               0x08
#define SMPC_CMD_CDOFF              0x09
#define SMPC_CMD_CARTON             0x0A
#define SMPC_CMD_CARTOFF            0x0B
#define SMPC_CMD_SYSRES             0x0D
#define SMPC_CMD_CKCHG352           0x0E
#define SMPC_CMD_CKCHG320           0x0F
#define SMPC_CMD_INTBACK            0x10
#define SMPC_CMD_SETTIM             0x16
#define SMPC_CMD_SETSM              0x17
#define SMPC_CMD_NMIREQ             0x18
#define SMPC_CMD_RESENA             0x19
#define SMPC_CMD_RESDIS             0x1A

#define SNDRAM (0x25A00000)
#define SATURN_COMMS (*((volatile DRIVER_COMMS *)(SNDRAM + 0x8000)))
#define ZMD_ADDR ((volatile Uint8 *)(&SATURN_COMMS + sizeof(DRIVER_COMMS)))
#define MASTER_VOLUME (*((volatile Uint16 *)(SNDRAM + 0x100400)))

#if DEVCART_LOAD == 0
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
#endif

//must be called after cd_init
void sound_init() {
    if (!DEVCART_LOAD) {
        sound_external_audio_enable(5, 5);
    }
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
    return;
    // pcm_play(num, PCM_SEMI, 6);
}

