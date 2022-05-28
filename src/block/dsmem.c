#include "dstime.h"

#include "util.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/sysinfo.h>

static const int interval = 1000;


static int getramusage(unsigned long* out_used, unsigned long* out_total)
{
    struct sysinfo info;
    if (sysinfo(&info)) {
        return -1;
    }
    *out_used = info.totalram - info.freeram;
    *out_total = info.totalram;
    return 0;
}

static void updatestatus(dsblock_t* block)
{
    unsigned long used, total;
    if (getramusage(&used, &total)) {
        return;
    }

    unsigned long usedmib = used / (1024 * 1024);
    unsigned long totalmib = total / (1024 * 1024);
    unsigned long percent = used * 100 / total;

    dsblock_lock(block);
    if (block->status) {
        free(block->status);
    }
    block->status = smprintf("RAM %lu/%luMiB(%lu%%)", usedmib, totalmib, percent);
    dsblock_unlock(block);   
}


static void* routine(void* arg)
{
    dsblock_t* block = (dsblock_t*)arg;

    for(;;msleep(interval)) {
        updatestatus(block);
        dsblock_signal_main(block);
    }
    return 0;
}

void dsmem_init(dsblock_t* block, struct dsmain_t* dsmain)
{
    dsblock_init(block, dsmain, sizeof(pthread_t));
    pthread_create((pthread_t*)block->data, NULL, routine, (void*)block);
}

void dsmem_deinit(dsblock_t* block)
{
    pthread_cancel(*(pthread_t*)block->data);
    dsblock_deinit(block);
}
