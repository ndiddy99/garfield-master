#include <sega_mth.h>
#include <sega_per.h>

#include "block.h"
#define HISTORY_LEN (4)
static Uint8 history[HISTORY_LEN];
static int first;

static void HistoryPush(int block) {
    history[3] = history[2];
    history[2] = history[1];
    history[1] = history[0];
    history[0] = block;
}

static int HistoryContains(int block) {
    for (int i = 0; i < HISTORY_LEN; i++) {
        if (history[i] == block) {
            return 1;
        }
    }

    return 0;
}

void RNG_Init() {
    Uint8 *time = PER_GET_TIM();
    Uint32 timestamp = (time[4] << 24) | (time[3] << 16) | (time[2] << 8) | time[1];
    MTH_InitialRand(timestamp);
    
    // initialize the history
    HistoryPush(BLOCK_Z);
    HistoryPush(BLOCK_Z);
    HistoryPush(BLOCK_S);
    HistoryPush(BLOCK_S);

    // we're going to give the first piece
    first = 1;
}

int RNG_GetNum() {
    int tries = 6;
    int candidate;

    while (tries > 0) {
        candidate = (MTH_GetRand() >> 8) % BLOCK_COUNT;
        
        // don't allow S, Z, or O to be the first block (retry until we get a different block)
        if (first && ((candidate == BLOCK_S) || (candidate == BLOCK_Z) || (candidate == BLOCK_O))) {
            continue;
        }
        
        // if the block was in the history, retry a finite number of times
        else if (HistoryContains(candidate)) {
            tries--;
            continue;
        }

        // our block wasn't in the history so quit the loop
        else {
            break;
        }
    }
    
    first = 0;
    HistoryPush(candidate);
    return candidate;
}
