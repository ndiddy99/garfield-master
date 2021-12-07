#ifndef SOUND_H
#define SOUND_H

// sets cdda volume (between 0 and 7)
void Sound_CDVolume(Uint8 vol_l, Uint8 vol_r);

//must be called after cd_init
void Sound_Init(void);

#define CDDA_START (2)
typedef enum {
    LOGO_TRACK = CDDA_START,
    INTRO_TRACK,
    MENU_TRACK,
    PAUSE_TRACK,
    GAMEOVER_TRACK,
    TOMO_TRACK,
    LEVEL1_TRACK,
    SOON_TRACK,
} CDDA_INDEX;

//play an audio track. loop: 1 if we want to loop the track
void Sound_CDDA(int track, int loop);

typedef enum {
    SOUND_I = 0,
    SOUND_Z,
    SOUND_S,
    SOUND_J,
    SOUND_L,
    SOUND_O,
    SOUND_T,
    SOUND_CLEAR,
    SOUND_FALL,
    SOUND_LAND,
    SOUND_LOCK,
    SOUND_ROTATE,
} PCM_INDEX;

//play a pcm sound
void Sound_Play(short num);
#endif
