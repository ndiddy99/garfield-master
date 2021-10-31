#include <sega_scl.h>
#define _SPR2_
#include <sega_spr.h>

#include "cd.h"
#include "sprite.h"
#include "print.h"

#define ROWS 60
#define COLS 88
#define FONT_X 8
#define FONT_Y 8
Uint8 text[ROWS][COLS];

void Print_Load() {
	Sprite_Load("FONT.SPR", NULL);
}

void Print_Init() {
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			text[i][j] = 255;
		}
	}
}

void Print_Num(Uint32 num, int row, int col) {
	int rightCol = col + 9; //rightmost column
	int i;
	for (i = 0; i <= 9; i++) {
		text[row][rightCol--] = (num % 10) + 16;
		num /= 10;
	}
}

void Print_String(char *ch, int row, int col) {
	int index = 0;
	int colBak = col;
	while (ch[index]) {
		if (ch[index] == '\n') {
			row++;
			col = colBak;
		}
		else {
			text[row][col++] = ch[index] - 32;
		}
		index++;
	}
}

void Print_Display() {
	int i, j;
	SPRITE_INFO textSpr;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if (text[i][j] != 255) {
				Sprite_Make(text[i][j],
					MTH_IntToFixed(j << 3), // * FONT_X
					MTH_IntToFixed(i << 3), // * FONT_Y
					&textSpr);
				Sprite_Draw(&textSpr);
			}
		}
	}
}
