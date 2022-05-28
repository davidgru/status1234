#include "sblock.h"

#include "sbar.h"

#include <stdlib.h>


// used as info for the newly created thread
typedef struct sblock_info_t {
    sblock_t* block;
    sblock_routine_t routine;
} sblock_info_t;


static void* thread_routine(void* arg)
{
    sblock_info_t* info = (sblock_info_t*)arg;
    
    sblock_t* block = info->block;
    sblock_routine_t routine = info->routine;

    free(info);

    routine(block);
    return 0;
}


int sblock_init(sblock_t* block, sblock_routine_t routine)
{
    block->status = NULL;
    block->bar = NULL;

    if (pthread_mutex_init(&block->mut, NULL)) {
        return -1;
    }

    sblock_info_t* info = (sblock_info_t*)malloc(sizeof(sblock_info_t));
    info->block = block;
    info->routine = routine;

    if (pthread_create(&block->thread, NULL, thread_routine, (void*)info)) {
        pthread_mutex_destroy(&block->mut);
        return -1;
    }
    
    return 0;
}


int sblock_signal_main(sblock_t* block)
{
    if (block->bar == NULL) {
        return -1;
    }
    return sbar_update(block->bar);
}

int sblock_lock(sblock_t* block)
{
    return pthread_mutex_lock(&block->mut) ? -1 : 0;
}

int sblock_unlock(sblock_t* block)
{
    return pthread_mutex_unlock(&block->mut) ? -1 : 0;
}

void sblock_deinit(sblock_t* block)
{
    pthread_cancel(block->thread);
    pthread_mutex_destroy(&block->mut);
}
