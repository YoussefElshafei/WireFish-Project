/*
 * File: timeutil.h
 * Summary: Millisecond-precision time helpers.
 *
 * Public API:
 *  - long ms_now(void);
 *  - int  ms_sleep(int ms);
 *  - long ms_diff(long start_ms, long end_ms);
 */
#ifndef TIMEUTIL_H
#define TIMEUTIL_H

long ms_now(void);
int  ms_sleep(int ms);
long ms_diff(long start_ms, long end_ms);

#endif /* TIMEUTIL_H */
