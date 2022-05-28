#include "sblocks.h"

#include <stdlib.h>
#include <stdio.h>

#include "util.h"

static const int interval = 5 * 1000;

static const char* wifisymb[] = {
    "", "", "", "", ""
};


static int link_to_symbi(int link)
{
    if (link < 0)   return 0;
    if (link == 0)  return 1;
    if (link <= 20) return 2;
    if (link <= 50) return 3;
    return 4;
}


static int get_wifi_link()
{
    FILE* file = fopen("/proc/net/wireless", "r");
    if (file == NULL) {
        return -1;
    }
    char line[512];
    for (int i = 0; i < 3; i++) {
        if (fgets(line, sizeof(line) - 1, file) == NULL) {
            fclose(file);
            return -1;
        }
    }
    int link = 0;
    if (sscanf(line, "%*s %*s %d", &link) == EOF) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return link;
}


void swifi_routine(sblock_t* block)
{
    const int max_status_length = sizeof("") - 1;
    char status[sizeof("")];
    block->status = status;

    int last_symbi = 0;
    for(;;msleep(interval)) {
        int link = get_wifi_link();
        int symbi = link_to_symbi(link);
        if (symbi == last_symbi) {
            continue;
        }

        sblock_lock(block);
        snprintf(block->status, max_status_length + 1, "%s", wifisymb[symbi]);
        sblock_unlock(block);
        sblock_signal_main(block);

        last_symbi = symbi;
    }

    block->status = NULL;
}
