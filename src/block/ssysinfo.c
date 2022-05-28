#include "sblocks.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/sysinfo.h>

#include "util.h"


static const int interval = 2000;


static int get_cpu_usage(long long* last_total, long long* last_work)
{
    long long user, nice, system, idle, iowait, irq, softirq;
    FILE* fd;

    if ((fd = fopen("/proc/stat", "r")) == NULL
        || fscanf(fd, "cpu %lld %lld %lld %lld %lld %lld %lld", &user, &nice, &system, &idle, &iowait, &irq, &softirq) != 7) {
        goto error;
    }
    fclose(fd);
    
    long long total = user + nice + system + idle + iowait + irq + softirq;
    long long work = user + nice + system;

    int ret = -1;
    if (*last_total != -1) {
        ret = round_int(*last_work - work, *last_total - total);
    }
    *last_total = total;
    *last_work = work;
    return ret;

error:
    if (fd != NULL) {
        fclose(fd);
    }
    *last_total = -1;
    *last_work = -1;
    return -1;
}


static int get_ram_usage(unsigned long* out_usedmib, unsigned long* out_totalmib, unsigned long* out_percent)
{
    struct sysinfo info;
    if (sysinfo(&info)) {
        return -1;
    }
    *out_usedmib = (info.totalram - info.freeram) / (1024 * 1024);
    *out_totalmib = info.totalram / (1024 * 1024);
    *out_percent = (info.totalram - info.freeram) * 100 / info.totalram;
    return 0;
}


void ssysinfo_routine(sblock_t* block)
{
    long long cpu_total = -1, cpu_work = -1;
    long long last_cpu_usage = -1;
    unsigned long last_ram_usedmib = -1;

    for (;;msleep(interval)) {
        int cpu_usage = get_cpu_usage(&cpu_total, &cpu_work);
        unsigned long ram_usedmib = -1, ram_totalmib = -1, ram_used_percent = -1;
        get_ram_usage(&ram_usedmib, &ram_totalmib, &ram_used_percent);
        if (cpu_usage == last_cpu_usage && ram_usedmib == last_ram_usedmib) {
            continue;
        }
        sblock_lock(block);
        if (block->status) {
            free(block->status);
        }
        block->status = smprintf("CPU %d%%  RAM %lu/%luMiB(%lu%%)", cpu_usage, ram_usedmib, ram_totalmib, ram_used_percent);
        sblock_unlock(block);
        sblock_signal_main(block);

        last_cpu_usage = cpu_usage;
        last_ram_usedmib = ram_usedmib;
    }
    if (block->status) {
        free(block->status);
    }
    block->status = NULL;
}
