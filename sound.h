#ifndef SOUND_H
#define SOUND_H

//must be called after cd_init
void sound_init(void);

// play a vgm file off the disc
void sound_vgm_load(char *filename);
void sound_vgm_play(void);
void sound_vgm_stop(void);

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
void sound_cdda(int track, int loop);

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
void sound_play(short num);
#endif
