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
 *  - int  monitor_run(const char *iface, int interval_ms, int duration_sec);
 *  - void monitor_print_header(void);
 *  - void monitor_print_stats(const IfaceStats *stats);
 *
 * Inputs:
 *  - iface: interface name (e.g., "eth0", "wlan0", NULL for first available)
 *  - interval_ms: sampling interval in milliseconds
 *  - duration_sec: monitoring duration in seconds (0 for infinite)
 *
 * Outputs:
 *  - Series of timestamped samples with computed rates
 *  - Format: IFACE RX_BYTES TX_BYTES RX_BPS TX_BPS RX_AVG_BPS TX_AVG_BPS
 *
 * Returns:
 *  - 0 on success; <0 on error (iface not found, file read error)
 *
 * Dependencies: timeutil.h
 */
#ifndef MONITOR_H
#define MONITOR_H

#include <stddef.h>
#include "../model/model.h"

/* Run bandwidth monitoring on interface */
int monitor_run(const char *iface, int interval_ms, int duration_sec, MonitorSeries *out);

/* Stop monitoring (signal handler safe) */
void monitor_stop(void);

/* Free any heap memory owned by a MonitorSeries */
void monitorseries_free(MonitorSeries *series);

#endif /* MONITOR_H */