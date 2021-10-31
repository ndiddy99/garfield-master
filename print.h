#ifndef PRINT_H
#define PRINT_H
void Print_Load(); //load font gfx into sprite ram
void Print_Init();
void Print_Num(Uint32 num, int row, int col);
void Print_String(char *ch, int row, int col);
void Print_Display();
#endif
