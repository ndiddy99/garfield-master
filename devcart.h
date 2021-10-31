#ifndef DEVCART_H
#define DEVCART_H

//loads file with filename specified from computer
int Devcart_LoadFile(char *filename, void *dest);
//prints string to computer
void Devcart_PrintStr(char *string);
//reset back to file menu
void Devcart_Reset(void);

#endif
