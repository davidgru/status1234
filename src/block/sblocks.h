#pragma once

#include "sblock.h"

// shows the date and time
void stime_routine(sblock_t* block);


// shows the wifi connection
void swifi_routine(sblock_t* block);


// shows the audio volume
void saudio_routine(sblock_t* block);


// shows if microphone is muted
void smic_routine(sblock_t* block);


// shows battery status
void sbattery_routine(sblock_t* block);

// shows cpu, ram, disk usage
void ssysinfo_routine(sblock_t* block);


// gets stock quotes
void squotes_routine(sblock_t* block);
