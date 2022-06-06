#include "sblocks.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

static const int interval = 60 * 1000;


int getquote(const char* stock, float* out_quote, float* out_change_percent)
{
    char* cmd = smprintf("~/.status-scripts/fetchprice.py %s", stock);

    FILE* fd = popen(cmd, "r");
    if (fd == NULL) {
        goto error;
    }
    if (fscanf(fd, "%f %f", out_quote, out_change_percent) != 2) {
        goto error;
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
        float quote, change_percent;
        if (getquote("NVAX", &quote, &change_percent) || quote == last_quote) {
            continue;
        }
        sblock_lock(block);
        if (block->status) {
            free(block->status);
        }
        block->status = smprintf("$NVAX %.2f %s%.2f%%", quote,  change_percent < 0 ? "" : "", fabs(change_percent));
        sblock_unlock(block);
        sblock_signal_main(block);
        last_quote = quote;
    }
}
