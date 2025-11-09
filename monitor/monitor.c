/*
 * File: monitor.c
 * Implements periodic sampling of /proc/net/dev and rate computations.
 *
 * Implementation Notes:
 *  - Parse selected iface row
 *  - Compute deltas between consecutive samples / elapsed time
 *  - Smooth with ring buffer mean if enabled
 *
 * Errors:
 *  - Returns negative on parse error or missing interface
 */
