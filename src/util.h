#pragma once


// sleep for msec milliseconds
void msleep(long msec);

// rounds (num / denom) to nearest integer without using float arithmetic
int round_int(int num, int denom);

// returns new formated string according to fmt and args
char* smprintf(const char *fmt, ...);
