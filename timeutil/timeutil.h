/*
 * File: timeutil.h
 * Summary: Millisecond-precision time helpers with formatted output.
 *
 * Public API:
 *  - long ms_now(void);              // Get current time in milliseconds
 *  - int  ms_sleep(int ms);          // Sleep for ms milliseconds
 *  - long ms_diff(long start, long end); // Calculate time difference
 *  - void format_timestamp(char *buf, size_t len); // Format current time as HH:MM:SS.mmm
 */
#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <stddef.h>

long ms_now(void);
int  ms_sleep(int ms);
long ms_diff(long start_ms, long end_ms);
void format_timestamp(char *buf, size_t len);

#endif /* TIMEUTIL_H */