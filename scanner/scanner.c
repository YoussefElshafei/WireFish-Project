/*
 * File: scanner.c
 * Implements TCP connect-based port scanning.
 *
 * Implementation Notes:
 *  - Resolves cfg->target once; iterates port range
 *  - connect() with timeout (non-blocking or SO_SNDTIMEO)
 *  - Classifies states: OPEN (connect OK), CLOSED (RST/refused), FILTERED (timeout)
 *  - Measures latency (connect start->end) if enabled in cfg
 *
 * Error Handling:
 *  - Validates cfg fields; returns negative errno-style codes on failure
 *  - Frees 'out' on error
 *
 * TODOs:
 *  - Parallel scanning (select/poll or pthreads)
 *  - IPv6 support toggle
 *  - CIDR host enumeration
 */