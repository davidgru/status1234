#include "sblocks.h"

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include "util.h"


static const char* volsymb[] = {
    "", "", "", "", ""
};


typedef struct smixer_t {
    snd_mixer_t *handle;
    snd_mixer_elem_t* elem;
} smixer_t;


static int smixer_setup(smixer_t* mixer, const char* selem_name, const char* card, int index, snd_mixer_elem_callback_t cb, void* arg)
{
    snd_mixer_selem_id_t *sid;

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, index);
    snd_mixer_selem_id_set_name(sid, selem_name);

    if (snd_mixer_open(&mixer->handle, 0) || 
        snd_mixer_attach(mixer->handle, card) ||
        snd_mixer_selem_register(mixer->handle, NULL, NULL) ||
        snd_mixer_load(mixer->handle)) {
        if (mixer->handle) {
            snd_mixer_close(mixer->handle);
        }
        mixer->handle = NULL;
        return 1;
    }
    mixer->elem = snd_mixer_find_selem(mixer->handle, sid);
    if (!mixer->elem) {
        snd_mixer_close(mixer->handle);
        mixer->handle = NULL;
        return 1;
    }

    snd_mixer_elem_set_callback(mixer->elem, cb);
    snd_mixer_elem_set_callback_private(mixer->elem, arg);
    return 0;
}


static void smixer_handle_next_event(smixer_t* mixer)
{
    if (!snd_mixer_wait(mixer->handle, -1)) {
        snd_mixer_handle_events(mixer->handle);
    }
}


static void smixer_destroy(smixer_t* mixer)
{
    snd_mixer_close(mixer->handle);
    mixer->handle = NULL;
    mixer->elem = NULL;
}


static int getvolume(snd_mixer_elem_t* elem, int* out_muted)
{
    long min, max, volume;
    if (!snd_mixer_selem_has_playback_switch(elem)) {
        *out_muted = 1;
    } else {
        snd_mixer_selem_get_playback_switch(elem, 0, out_muted);
        *out_muted = !*out_muted;
    }
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, 0, &volume);
    return (int)roundf(((float)(volume - min) / (float)(max - min)) * 100);
}


static int updatestatus(snd_mixer_elem_t* elem)
{
    sblock_t* block = (sblock_t*)snd_mixer_elem_get_callback_private(elem);

    int muted;
    int volume = getvolume(elem, &muted);
    volume = volume > 100 ? 100 : volume;

    int symbi;
    if (muted) {
        symbi = 0;
    } else if (volume == 0) {
        symbi = 1;
    } else if (volume < 33) {
        symbi = 2;
    } else if (volume < 66) {
        symbi = 3;
    } else {
        symbi = 4;
    }

    sblock_lock(block);
    if (block->status) {
        free(block->status);
    }
    block->status = smprintf("%s %d", volsymb[symbi], volume);
    sblock_unlock(block);
    sblock_signal_main(block);
    return 0;
}


static int volume_cb(snd_mixer_elem_t* elem, unsigned int mask)
{
    (void)mask;
    return updatestatus(elem);
}                           


void saudio_routine(sblock_t* block)
{
    msleep(2000); // give alsa time to set up
    smixer_t mixer;
    while(smixer_setup(&mixer, "Master", "default", 0, volume_cb, (void*)block)) {
        msleep(1000);
    }

    updatestatus(mixer.elem);

    for (;;) {
        smixer_handle_next_event(&mixer);
    }

    smixer_destroy(&mixer);
    if (block->status) {
        free(block->status);
    }
    block->status = NULL;
}
