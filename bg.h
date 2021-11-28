#ifndef BG_H
#define BG_H

// should be run at the beginning of the program (after CD and scroll)
void BG_Init();

// should be run every gameplay frame
void BG_Run();

// run to advance to the next BG
void BG_Next();

#endif

