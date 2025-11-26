/*
 * File: timeutil.c
 * Implementation of millisecond-precision time utilities.
 */
#include "timeutil.h"
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

/*
 * ms_now - Get current time in milliseconds since epoch
 * Returns: milliseconds timestamp, or -1 on error
 */
long ms_now(void) {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        return -1;
    }
    return (long)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}

/*
 * ms_sleep - Sleep for specified milliseconds
 * Args:
 *   ms: milliseconds to sleep
 * Returns: 0 on success, -1 on error
 */
int ms_sleep(int ms) {
    if (ms < 0) {
        return -1;
    }
    
    struct timespec req;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000L;
    
    while (nanosleep(&req, &req) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }
    return 0;
}

/*
 * ms_diff - Calculate difference between two millisecond timestamps
 * Args:
 *   start_ms: start timestamp
 *   end_ms: end timestamp
 * Returns: difference in milliseconds (end - start)
 */
long ms_diff(long start_ms, long end_ms) {
    return end_ms - start_ms;
}

/*
 * format_timestamp - Format current time as HH:MM:SS.mmm
 * Args:
 *   buf: output buffer
 *   len: buffer length
 */
void format_timestamp(char *buf, size_t len) {
    struct timeval tv;
    struct tm *tm_info;
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    snprintf(buf, len, "%02d:%02d:%02d.%03ld",
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec,
             tv.tv_usec / 1000);
}