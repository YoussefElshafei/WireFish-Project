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

#include "monitor.h"
#include <stdlib.h>

/*
 * Temporary stub implementation.
 * 
 * For now:
 *  - Ignores cfg
 *  - Allocates no samples
 *  - Just zeros the MonitorSeries and returns success
 * 
 * You’ll later replace this with real sampling logic
 * that fills series->samples over time.
 */
int monitor_run(const CommandLine *cfg, MonitorSeries *out, int sample_count){
    (void)cfg;
    (void)sample_count;

    if(out == NULL){
        return -1;
    }

    out->samples = NULL;
    out->len = 0;
    out->cap = 0;

    return 0; // “success” for now
}

/*
 * Free any memory owned by the MonitorSeries.
 * Right now we only free samples (if allocated).
 */
void monitorseries_free(MonitorSeries *series){
    if(series == NULL){
        return;
    }

    free(series->samples);
    series->samples = NULL;
    series->len = 0;
    series->cap = 0;
}

