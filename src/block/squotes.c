#include "sblocks.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "util.h"

static const int interval = 60 * 1000;


int getquote(const char* stock, float* out_quote, float* out_last_close)
{
    char* cmd = smprintf("~/.status-scripts/fetchprice.py %s", stock);

    FILE* fd = popen(cmd, "r");
    if (fd == NULL) {
        goto error;
    }
    if (fscanf(fd, "%f %f", out_quote, out_last_close) != 2) {
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
        float quote, last_close;
        if (getquote("NVAX", &quote, &last_close) || quote == last_quote) {
            continue;
        }
        sblock_lock(block);
        if (block->status) {
            free(block->status);
        }
        block->status = smprintf("NVAX $%.2f %s%.2f%%", quote, quote < last_close ? "" : "", fabs(quote - last_close) / last_close * 100);
        sblock_unlock(block);
        sblock_signal_main(block);
        last_quote = quote;
    }
}
