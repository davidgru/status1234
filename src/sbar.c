#include "sbar.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char* makestatus(sbar_t* bar)
{
    // lock all blocks
    for (size_t i = 0; i < bar->num_elems; i++) {
        if (bar->elems[i].type == ELEM_TYPE_BLOCK) {
            sblock_t* block = (sblock_t*)bar->elems[i].data;
            pthread_mutex_lock(&block->mut);
        }
    }

    // determine length of status
    size_t len = 0;
    for (size_t i = 0; i < bar->num_elems; i++) {
        if (bar->elems[i].type == ELEM_TYPE_BLOCK) {
            sblock_t* block = (sblock_t*)bar->elems[i].data;
            if (block->status == NULL) {
                continue;
            }
            len += strlen(block->status);
        } else { // delim
            len += strlen((char*)bar->elems[i].data);
        }
    }
    
    char* status = (char*)calloc(len + 1, sizeof(char));
    if (status == NULL) {
        return NULL;
    }

    // assemble status
    size_t off = 0;
    for (size_t i = 0; i < bar->num_elems; i++) {
        if (bar->elems[i].type == ELEM_TYPE_BLOCK) {
            sblock_t* block = (sblock_t*)bar->elems[i].data;
            if (block->status != NULL) { // ignore if NULL
                size_t len = strlen(block->status);
                memcpy(status + off, block->status, len);
                off += len;
            }
        } else { // delim
            const char* delim = (const char*)bar->elems[i].data;
            size_t len = strlen(delim);
            memcpy(status + off, delim, len);
            off += len;
        }
    }
    status[len] = '\0';

    // unlock all blocks
    for (size_t i = 0; i < bar->num_elems; i++) {
        if (bar->elems[i].type == ELEM_TYPE_BLOCK) {
            sblock_t* block = (sblock_t*)bar->elems[i].data;
            pthread_mutex_unlock(&block->mut);
        }
    }

    return status;
}

static void update(sbar_t* bar, char* status)
{
    // printf("%s\n", status);
	XStoreName(bar->dpy, DefaultRootWindow(bar->dpy), status);
	XSync(bar->dpy, False);
}


static int add_elem(sbar_t* bar, selem_t* elem)
{
    // increase array size
    selem_t* new_elems = (selem_t*)calloc(bar->num_elems + 1, sizeof(selem_t));
    if (new_elems == NULL) {
        return -1;
    }
    if (bar->elems) {
        memcpy(new_elems, bar->elems, bar->num_elems * sizeof(selem_t));
        free(bar->elems);
    }
    bar->elems = new_elems;
    // add new elem
    memcpy(&bar->elems[bar->num_elems++], elem, sizeof(selem_t));
    return 0;
}


int sbar_init(sbar_t* bar)
{
    bar->dpy = XOpenDisplay(NULL);
    if (bar->dpy == NULL) {
        return -1;
    }

    bar->elems = NULL;
    bar->num_elems = 0;

    if (pthread_mutex_init(&bar->mut, NULL) || pthread_cond_init(&bar->cond, NULL)) {
        return -1;
    }
    bar->update = 0;
    return 0;
}


int sbar_add_block(sbar_t* bar, sblock_t* block)
{
    block->bar = bar; // block needs to know bar to signal update
    selem_t elem = {
        .type = ELEM_TYPE_BLOCK,
        .data = (void*)block
    };
    return add_elem(bar, &elem);
}


int sbar_add_delim(sbar_t* bar, const char* delim)
{
    selem_t elem = {
        .type = ELEM_TYPE_DELIM,
        .data = (void*)delim
    };
    return add_elem(bar, &elem);
}


int sbar_update(sbar_t* bar)
{
    pthread_mutex_lock(&bar->mut);
    if (!bar->update) {
        bar->update = 1;
        pthread_cond_broadcast(&bar->cond);
    }
    pthread_mutex_unlock(&bar->mut);
    return 0;
}


int sbar_run(sbar_t* bar)
{
    // blocks might have tried to update before they were linked to the bar
    sbar_update(bar);
    for(;;) {
        pthread_mutex_lock(&bar->mut);
        while(!bar->update) {
            pthread_cond_wait(&bar->cond, &bar->mut);
        }
        char* status = makestatus(bar);
        if (status != NULL) {
            update(bar, status);
            free(status);
        }
        bar->update = 0;
        pthread_mutex_unlock(&bar->mut);
    }
    return 0;
}


void sbar_deinit(sbar_t* bar)
{
    pthread_mutex_lock(&bar->mut);
    for (size_t i = 0; i < bar->num_elems; i++) {
        if (bar->elems[i].type == ELEM_TYPE_BLOCK) {
            sblock_t* block = (sblock_t*)bar->elems[i].data;
            block->bar = NULL;
        }
    }
    pthread_mutex_unlock(&bar->mut);

    pthread_mutex_destroy(&bar->mut);
    pthread_cond_destroy(&bar->cond);

    if (bar->elems) {
        free(bar->elems);
    }
    bar->num_elems = 0;
    if (bar->dpy) {
        XCloseDisplay(bar->dpy);
    }
}
