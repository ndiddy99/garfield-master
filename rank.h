#ifndef RANK_H
#define RANK_H

// so the game can say what rank the player got
void Rank_Setup(int ranking);

// loads graphics off cd
void Rank_Init();

// should be run every frame after Rank_Init
int Rank_Run();
#endif
