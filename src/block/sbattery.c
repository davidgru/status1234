#include "sblocks.h"

#include "util.h"

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define BATTERY_PATH "/sys/class/power_supply/BAT0"


static const int interval = 5000;


static const char* levelsymb[] = {
    "", "", "", "", "", ""
};

static const char* chargesymb[] = {
    "", ""
};


static int level_to_levelsymbi(int level)
{
    if (level < 0) return 0;
    if (level < 10) return 1;
    if (level < 30) return 2;
    if (level < 60) return 3;
    if (level < 90) return 4;
    return 5;
}

static int charge_to_chargesymbi(int level, int charge)
{
    if (level < 0 || charge == 0) return 0;
    return 1;
}


static int read_file_property(const char* filename, const char* type_modifier, void* property)
{
    FILE* fd = fopen(filename, "r");
    if (fd == NULL) {
        return -1;
    }
    int c = fscanf(fd, type_modifier, property);
    fclose(fd);
    return c == 1 ? 0 : -1;
}


static int getbattery(int* out_charging)
{
    char charging_str[12];
    int present, capacity, charge;

    // check if battery is present	
    if (read_file_property(BATTERY_PATH "/present", "%d", &present) < 0 || present != 1 // battery present
        || read_file_property(BATTERY_PATH "/energy_full", "%d", &capacity) < 0 // get capacity
        || read_file_property(BATTERY_PATH "/energy_now", "%d", &charge) < 0 // get current charge
        || read_file_property(BATTERY_PATH "/status", "%11s", charging_str) < 0) { // check if charging
        return -1;
    }

    *out_charging = strncmp(charging_str, "Charging", 8) == 0;
    return (int)roundf(((float)charge / (float)capacity) * 100.0f);
}


void sbattery_routine(sblock_t* block)
{
    int last_level = -1;
    int last_charging = 0;

    for (;;msleep(interval)) {
        int charging = 0;
        int level = getbattery(&charging);
        if (level == last_level && charging == last_charging) {
            continue;
        }

        int levelsymbi = level_to_levelsymbi(level);
        int chargesymbi = charge_to_chargesymbi(level, charging);

        sblock_lock(block);
        if (block->status) {
            free(block->status);
        }
        block->status = smprintf("%s %d %s", levelsymb[levelsymbi], level, chargesymb[chargesymbi]);
        sblock_unlock(block);
        sblock_signal_main(block);
    
        last_level = level;
        last_charging = charging;
    }

    if (block->status) {
        free(block->status);
    }
    block->status = NULL;
}
