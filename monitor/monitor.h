/*
 * File: monitor.h
 * Summary: Interface bandwidth monitor using /proc/net/dev sampling.
 *
 * Responsibilities:
 *  - Sample RX/TX byte counters for an interface at fixed intervals
 *  - Compute instantaneous rates (bps) and rolling averages
 *
 * Data & Types:
 *  - typedef struct IfaceStats { char iface[64]; U64 rx_bytes, tx_bytes; double rx_rate_bps, tx_rate_bps; double rx_avg_bps, tx_avg_bps; }
 *  - typedef struct MonitorSeries { IfaceStats *samples; size_t len, cap; }
 *
 * Public API:
 *  - int  monitor_run(const Config *cfg, int duration_sec, MonitorSeries *out);
 *  - void monitor_free(MonitorSeries *s);
 *
 * Inputs:
 *  - cfg->iface, cfg->interval_ms
 * Outputs:
 *  - Series of timestamped samples with computed rates
 *
 * Returns:
 *  - 0 on success; <0 on error (iface not found, file read error)
 *
 * Dependencies: timeutil.h, ringbuf.h (for smoothing), config.h
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <stddef.h>

#include "../cli/cli.h"
#include "../model/model.h"

// Uses IfaceStats and MonitorSeries from model.h

// sample_count: how many samples to collect (e.g., DEFAULT_MONITOR_SAMPLES)
int monitor_run(const CommandLine *cfg, MonitorSeries *out, int sample_count);

// free any heap-allocated memory in MonitorSeries
void monitorseries_free(MonitorSeries *s);

#endif /* MONITOR_H */

