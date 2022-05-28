#include "dstime.h"

#include "util.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static const int interval = 1000;


static int getusage()
{
    static long long last_total = -1;
    static long long last_work = -1;

    FILE* fd = fopen("/proc/stat", "r");
    if (fd == NULL) {
        last_total = -1;
        last_work = -1;
        return -1;
    }
    long long user, nice, system, idle, iowait, irq, softirq;
    if (fscanf(fd, "cpu %lld %lld %lld %lld %lld %lld %lld", &user, &nice, &system, &idle, &iowait, &irq, &softirq) != 7) {
        last_total = -1;
        last_work = -1;
        fclose(fd);
        return -1;
    }
    fclose(fd);
    long long total = user + nice + system + idle + iowait + irq + softirq;
    long long work = user + nice + system;

    int ret = -1;
    if (last_total != -1) {
        ret = 1000 * (last_work - work) / (last_total - total);
        ret = ret % 10 < 5 ? ret / 10 : ret / 10 + 1;
    }
    last_total = total;
    last_work = work;
    return ret;
}


static void updatestatus(dsblock_t* block)
{
    static int last_usage = -1;

    int usage = getusage();
    if (usage == last_usage) {
        return;
    }
    last_usage = usage;

    dsblock_lock(block);
    if (block->status) {
        free(block->status);
    }
    block->status = smprintf("CPU %d%%", usage);
    dsblock_unlock(block);
    dsblock_signal_main(block);
}

static void* routine(void* arg)
{
    dsblock_t* block = (dsblock_t*)arg;

    for(;;msleep(interval)) {
        updatestatus(block);
    }
    return 0;
}

void dscpu_init(dsblock_t* block, struct dsmain_t* dsmain)
{
    dsblock_init(block, dsmain, sizeof(pthread_t));
    pthread_create((pthread_t*)block->data, NULL, routine, (void*)block);
}

void dscpu_deinit(dsblock_t* block)
{
    pthread_cancel(*(pthread_t*)block->data);
    dsblock_deinit(block);
}
