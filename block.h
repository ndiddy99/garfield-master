#ifndef BLOCK_H
#define BLOCK_H

typedef enum {
    BLOCK_I = 0,
    BLOCK_Z,
    BLOCK_S,
    BLOCK_J,
    BLOCK_L,
    BLOCK_O,
    BLOCK_T,
} BLOCK;

#define BLOCK_COUNT (7)
#define BLOCK_ROTATIONS (4)
#define BLOCK_SIZE (4)

extern int blocks[BLOCK_COUNT][BLOCK_ROTATIONS][BLOCK_SIZE][BLOCK_SIZE];

#endif

