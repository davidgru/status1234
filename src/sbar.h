#pragma once

#include <pthread.h>

#include <X11/Xlib.h>


#include "block/sblock.h"


typedef enum selem_type_t {
    ELEM_TYPE_BLOCK,
    ELEM_TYPE_DELIM
} selem_type_t;


typedef struct selem_t {
    selem_type_t type;
    void* data; // a string if type = ELEM_TYPE_DELIM or a block_t* if type = ELEM_TYPE_BLOCK
} selem_t;


typedef struct sbar_t {
    Display *dpy; // X11 status display

    selem_t* elems;
    size_t num_elems;

    pthread_mutex_t mut; // protects cond
    pthread_cond_t cond; // used by blocks to force the bar to update
    int32_t update; // set to 1 to force bar update and signal cond
} sbar_t;



int sbar_init(sbar_t* bar);


// adds a block to the bar, elements are added in the order they are added in
int sbar_add_block(sbar_t* bar, sblock_t* block);


// adds a delimiter to the bar
int sbar_add_delim(sbar_t* bar, const char* delim);


// called by a block if he updated and wants the bar to redraw, displays new status
int sbar_update(sbar_t* bar);


// blocks and bar waites for calls to sbar_update()
int sbar_run(sbar_t* bar);


void sbar_deinit(sbar_t* bar);
