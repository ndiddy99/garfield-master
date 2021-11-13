//pcm_sys.c
//this file is compiled separately
//hopefully somewhat portable
//

#include <sega_dma.h>

#include "cd.h"
#include "devcart.h"
#include "pcmsys.h"
#include "release.h"


static const int logtbl[] = {
/* 0 */		0, 
/* 1 */		1, 
/* 2 */		2, 2, 
/* 4 */		3, 3, 3, 3, 
/* 8 */		4, 4, 4, 4, 4, 4, 4, 4, 
/* 16 */	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
/* 32 */	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
/* 64 */	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
			7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
/* 128 */	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};
	
#define PCM_MSK1(a)				((a)&0x0001)
#define PCM_MSK3(a)				((a)&0x0007)
#define PCM_MSK4(a)				((a)&0x000F)
#define PCM_MSK5(a)				((a)&0x001F)
#define PCM_MSK10(a)			((a)&0x03FF)

#define PCM_SCSP_FREQUENCY					(44100L)

#define PCM_CALC_OCT(smpling_rate) 											\
		((int)logtbl[PCM_SCSP_FREQUENCY / ((smpling_rate) + 1)])
		
#define PCM_CALC_SHIFT_FREQ(oct)											\
		(PCM_SCSP_FREQUENCY >> (oct))
		
#define PCM_CALC_FNS(smpling_rate, shift_freq)								\
		((((smpling_rate) - (shift_freq)) << 10) / (shift_freq))
		
#define PCM_SET_PITCH_WORD(oct, fns)										\
		((int)((PCM_MSK4(-(oct)) << 11) | PCM_MSK10(fns)))
		
		#define DRV_SYS_END (10 * 1024) //System defined safe end of driver's address space

	sysComPara * m68k_com = (sysComPara *)(SNDPRG + DRV_SYS_END);
	unsigned int * scsp_load =  (unsigned int*)(0x408 + DRV_SYS_END + 0x20); //Local loading address for sound data, is DRV_SYS_END ahead of the SNDPRG, and ahead of the communication data
	unsigned short * master_volume = (unsigned short *)(SNDRAM + 0x100400);
	short numberPCMs = 0;

//////////////////////////////////////////////////////////////////////////////

void smpc_wait_till_ready (void)
{
   // Wait until SF register is cleared
   while(SMPC_REG_SF & 0x1) { }
}

//////////////////////////////////////////////////////////////////////////////

void smpc_issue_command(unsigned char cmd)
{
   // Set SF register so that no other command can be issued.
   SMPC_REG_SF = 1;

   // Writing COMREG starts execution
   SMPC_REG_COMREG = cmd;
}

//////////////////////////////////////////////////////////////////////////////

void load_driver_binary(char *filename, void * buffer)
{
	Sint32 file_size = CD_Load(filename, buffer);
	//The immediacy of these commands is important.
	//As per SEGA technical bulletin 51, the Sound CPU is not to be turned off for more than 0.5 seconds.
	// Turn off Sound CPU
	smpc_issue_command(SMPC_CMD_SNDOFF);
	smpc_wait_till_ready();
	//
	*master_volume = 0x20F; //Set max master volume + 4mbit memory // Very important, douglath
							//if at some point you wanted to change the master volume, it is adjustable here,
							//but you'll have to read the SCSP manual to find the bits that correspond to volume.
							//Or I could just tell you it is the four least significant bits.
							//There's no restriction on the timing of changing master volume.
		for(int i = 0; i < file_size; i++)
		{
			unsigned char * binary_destination = (unsigned char *)(SNDRAM + i);
			unsigned char * binary_source = (unsigned char *)(buffer + i);
			*binary_destination = *binary_source;
		}
		
	// Turn on Sound CPU again
	smpc_wait_till_ready();
	smpc_issue_command(SMPC_CMD_SNDON);
	//
}
	
void load_drv(void)
{
	// Make sure SCSP is set to 512k mode
	*(unsigned char *)(0x25B00400) = 0x02;

	// Clear Sound Ram
	for (int i = 0; i < 0x80000; i+=4){
		*(unsigned int *)(SNDRAM + i) = 0x00000000;
	}
	void * binary_buffer = (void*)2097152;
	
	// Load driver binary from CD and copy it to Sound RAM
	load_driver_binary((char *)"SDRV.BIN", binary_buffer);

}

short calculate_bytes_per_blank(int sampleRate, int is8Bit, int isPAL)
{
	//"frame count" being the # of frames per second (used to derive bytes per vblank, ergo, frame)
	int frameCount = (isPAL) ? 50 : 60;
	//The bit depth of each sample. Could say byte depth, 1 or 2. Same idea.
	int sampleSize = (is8Bit) ? 8 : 16;
	//Mathemagically finds the bytes per blank from the input information
	//The ">>3" is for  "divide by 8" in other words, derive to bytes instead of bits.
	return ((sampleRate * sampleSize)>>3)/frameCount;
	
}

short load_16bit_pcm(char *filename, int sampleRate)
{
	//Quick sanity check to see if the loading pointer has gone outside of sound RAM already.
	if( (int)scsp_load > 0x7F800)
	{
        if (DEVCART_LOAD) {
		    Devcart_PrintStr("(Sound RAM is full)");
        }
	}
	//These are intermediate pieces of information for the sample rate -> pitch word conversion.
	int octr;
	int shiftr;
	int fnsr;
	//
	//Find the file system ID of the file according to its name
	Sint32 file_size = CD_Load(filename, (Uint32 *)((unsigned int)scsp_load + SNDRAM));

	//These align the file on a 4-byte boundary. This is neccessary for the SCSP to not-crash.
	file_size += ((unsigned int)file_size & 1) ? 1 : 0;
	file_size += ((unsigned int)file_size & 3) ? 2 : 0;
	//
	
	//SBL automagical samplerate -> pitchword conversion
	octr = PCM_CALC_OCT(sampleRate);
	shiftr = PCM_CALC_SHIFT_FREQ(octr);
	fnsr = PCM_CALC_FNS(sampleRate, shiftr);
	///
	
	//Inform the PCM driver of what we've loaded
	m68k_com->pcmCtrl[numberPCMs].hiAddrBits = (unsigned short)( (unsigned int)scsp_load >> 16);
	m68k_com->pcmCtrl[numberPCMs].loAddrBits = (unsigned short)( (unsigned int)scsp_load & 0xFFFF);
	
	m68k_com->pcmCtrl[numberPCMs].pitchword = PCM_SET_PITCH_WORD(octr, fnsr);
	m68k_com->pcmCtrl[numberPCMs].playsize = (file_size>>1); //It is "file_size>>1" because 16-bit PCMs have 2 bytes per sample ( / 2).
	m68k_com->pcmCtrl[numberPCMs].bytes_per_blank = calculate_bytes_per_blank(sampleRate, 0, PCM_SYS_REGION);
	m68k_com->pcmCtrl[numberPCMs].bitDepth = 0; //Select 16-bit
	m68k_com->pcmCtrl[numberPCMs].loopType = 0; //Initialize as non-looping
	m68k_com->pcmCtrl[numberPCMs].volume = 7; //Iniitalize as max volume
	//

	numberPCMs++; //Increment pcm #
	scsp_load = (unsigned int *)((unsigned int )scsp_load + file_size); //Move the loading pointer ahead of the new sound
	return (numberPCMs-1); //Return the PCM # this sound recieved
}

short load_8bit_pcm(char *filename, int sampleRate)
{
	//Quick sanity check to see if the loading pointer has gone outside of sound RAM already.
	if( (int)scsp_load > 0x7F800)
	{
        if (DEVCART_LOAD) {
            Devcart_PrintStr("(Sound RAM is full)");
        }
	}
	
	//These are intermediate pieces of information for the sample rate -> pitch word conversion.
	int octr;
	int shiftr;
	int fnsr;
	//
	//Find the file system ID of the file according to its name
	Sint32 file_size = CD_Load(filename, (Uint32 *)((unsigned int)scsp_load + SNDRAM));

	//These align the file on a 4-byte boundary. This is neccessary for the SCSP to not-crash.
	file_size += ((unsigned int)file_size & 1) ? 1 : 0;
	file_size += ((unsigned int)file_size & 3) ? 2 : 0;
	//
	
	//SBL automagical samplerate -> pitchword conversion
	octr = PCM_CALC_OCT(sampleRate);
	shiftr = PCM_CALC_SHIFT_FREQ(octr);
	fnsr = PCM_CALC_FNS(sampleRate, shiftr);
	///
	//Inform the PCM driver of what we've loaded
	m68k_com->pcmCtrl[numberPCMs].hiAddrBits = (unsigned short)( (unsigned int)scsp_load >> 16);
	m68k_com->pcmCtrl[numberPCMs].loAddrBits = (unsigned short)( (unsigned int)scsp_load & 0xFFFF);
	
	m68k_com->pcmCtrl[numberPCMs].pitchword = PCM_SET_PITCH_WORD(octr, fnsr);
	m68k_com->pcmCtrl[numberPCMs].playsize = (file_size); //File size is issued as-is because 8-bit PCM have 1 byte = 1 sample.
	m68k_com->pcmCtrl[numberPCMs].bytes_per_blank = calculate_bytes_per_blank(sampleRate, 1, PCM_SYS_REGION);
	m68k_com->pcmCtrl[numberPCMs].bitDepth = 1; //Select 8-bit
	m68k_com->pcmCtrl[numberPCMs].loopType = 0; //Initialize as non-looping
	m68k_com->pcmCtrl[numberPCMs].volume = 7; //Iniitalize as max volume
	//

	numberPCMs++; //Increment pcm #
	scsp_load = (unsigned int *)((unsigned int )scsp_load + file_size); //Move the loading pointer ahead of the new sound
	return (numberPCMs-1); //Return the PCM # this sound recieved
}

void pcm_play(short pcmNumber, char ctrlType, char volume)
{
	m68k_com->pcmCtrl[pcmNumber].sh2_permit = 1;
	m68k_com->pcmCtrl[pcmNumber].volume = volume;
	m68k_com->pcmCtrl[pcmNumber].loopType = ctrlType;
}

void pcm_parameter_change(short pcmNumber, char volume, char pan)
{
	m68k_com->pcmCtrl[pcmNumber].volume = volume;
	m68k_com->pcmCtrl[pcmNumber].pan = pan;
}

void pcm_cease(short pcmNumber)
{

	if(m68k_com->pcmCtrl[pcmNumber].loopType <= 0)
		//If it is a volatile or protected sound, the expected control method is to mute the sound and let it end itself.
	{												//Protected sounds have a permission state of "until they end".
	m68k_com->pcmCtrl[pcmNumber].volume = 0;
	} else {
	m68k_com->pcmCtrl[pcmNumber].sh2_permit = 0; 
	//If it is a looping sound, the control method is to command it to stop.
	}
}
