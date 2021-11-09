#ifndef PIECE_H
#define PIECE_H

typedef enum {
    PIECE_I = 0,
    PIECE_Z,
    PIECE_S,
    PIECE_J,
    PIECE_L,
    PIECE_O,
    PIECE_T,
} PIECES;

#define PIECE_COUNT (7)
#define PIECE_ROTATIONS (4)
#define PIECE_SIZE (4)

extern int pieces[PIECE_COUNT][PIECE_ROTATIONS][PIECE_SIZE][PIECE_SIZE];

#endif

