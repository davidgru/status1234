#include "sblocks.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "util.h"

static const int interval = 15 * 1000;


typedef enum market_state_t {
    regular, pre, post
} market_state_t;

int getquote(const char* stock, market_state_t* out_state, float* out_quote, float* out_change_percent)
{
    char state[8];

    char* cmd = smprintf("~/.status-scripts/fetchprice.py %s", stock);

    FILE* fd = popen(cmd, "r");
    if (fd == NULL) {
        goto error;
    }
    if (fscanf(fd, "%7s %f %f", state, out_quote, out_change_percent) != 3) {
        goto error;
    }

    if (strncmp(state, "PRE", 7) == 0) {
        *out_state = pre;
    } else if (strncmp(state, "POST", 7) == 0) {
        *out_state = post;
    } else {
        *out_state = regular;
    }

    pclose(fd);
    free(cmd);
    return 0
    ;
error:
    if (cmd != NULL) {
        free(cmd);
    }
    if (fd != NULL) {
        pclose(fd);
    }
    return -1;
}


void squotes_routine(sblock_t* block)
{
    float last_quote = -1;
    for(;;msleep(interval)) {
        market_state_t state;
        float quote, change_percent;
        if (getquote("NVAX", &state, &quote, &change_percent) || quote == last_quote) {
            continue;
        }
        sblock_lock(block);
        if (block->status) {
            free(block->status);
        }
        block->status = smprintf("$NVAX %s%.2f %s%.2f%%",
            state == pre ? "(pre) " : state == post ? "(post) " : "",
            quote,
            change_percent < 0 ? "" : "",
            fabs(change_percent));
        sblock_unlock(block);
        sblock_signal_main(block);
        last_quote = quote;
    }
}
