#pragma once

#include <pthread.h>


typedef struct sblock_t {
    char* status; // the thing that gets printed to screen

    struct sbar_t* bar; // need to know the bar to update

    pthread_mutex_t mut; // lock while updating status

    pthread_t thread; // thread for the block
} sblock_t;



// function ran by block thread. gets block as input
typedef void(*sblock_routine_t)(sblock_t* block);



// initializes a block, starts a thread running routine
int sblock_init(sblock_t* block, sblock_routine_t routine);

// tell the bar to redraw
int sblock_signal_main(sblock_t* block);

// lock the block mutex
int sblock_lock(sblock_t* block);

// unlock the block mutex
int sblock_unlock(sblock_t* block);

// deinit block
void sblock_deinit(sblock_t* block);
