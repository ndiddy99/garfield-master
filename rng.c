#include <sega_mth.h>
#include <sega_per.h>

#include "piece.h"
#define HISTORY_LEN (4)
static Uint8 history[HISTORY_LEN];
static int first;

static void HistoryPush(int num) {
    history[3] = history[2];
    history[2] = history[1];
    history[1] = history[0];
    history[0] = num;
}

static int HistoryContains(int num) {
    for (int i = 0; i < HISTORY_LEN; i++) {
        if (history[i] == num) {
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
    HistoryPush(PIECE_Z);
    HistoryPush(PIECE_Z);
    HistoryPush(PIECE_S);
    HistoryPush(PIECE_S);

    // we're going to give the first piece
    first = 1;
}

int RNG_Get() {
    int tries = 6;
    int candidate;

    while (tries > 0) {
        candidate = (MTH_GetRand() >> 8) % PIECE_COUNT;
        
        // don't allow S, Z, or O to be the first piece (retry until we get a different piece)
        if (first && ((candidate == PIECE_S) || (candidate == PIECE_Z) || (candidate == PIECE_O))) {
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
