#include "util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>


void msleep(long msec)
{
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec= (msec % 1000) * 1000000;
    int res;
    do {
        res = nanosleep(&ts, &ts);
    } while(res && errno == EINTR);
}


int round_int(int num, int denom)
{
    int c = (1000 * num) / denom;
    return c % 10 < 5 ? c / 10 : c / 10 + 1;
}


char* smprintf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char* out = malloc(len);
    if (out == NULL) {
        return NULL;
    }

    va_start(args, fmt);
    vsnprintf(out, len, fmt, args);
    va_end(args);

    return out;
}
