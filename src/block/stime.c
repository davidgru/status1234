#include "sblocks.h"

#include <stdlib.h>
#include <stdio.h>

#include "util.h"


static const char* fmt = "%H:%M %d.%m.%Y";
static const size_t max_block_size = 128;


static void updatestatus(sblock_t* block)
{
    time_t tim;
	struct tm *timtm;
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		return;
    }
    strftime(block->status, max_block_size, fmt, timtm);
}


static void waitminute()
{
    time_t tim = time(NULL);
    struct tm *timtm = localtime(&tim);
    msleep((60 - timtm->tm_sec) * 1000);
}


void stime_routine(sblock_t* block)
{
    block->status = (char*)malloc(max_block_size + 1);
    if (block->status == NULL) {
        fprintf(stderr, "Failed to allocate status in stime_routine.\n");
        return;
    }

    for(;;waitminute()) {
        sblock_lock(block);
        updatestatus(block);
        sblock_unlock(block);
        sblock_signal_main(block);
    }

    if (block->status) {
        free(block->status);
        block->status = NULL;
    }
}
