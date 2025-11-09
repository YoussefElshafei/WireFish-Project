/*
 * File: fmt.h
 * Summary: Output formatters for human, CSV, and JSON.
 *
 * Responsibilities:
 *  - Render ScanTable, TraceRoute, MonitorSeries in consistent schema
 *  - Avoid business logic; pure presentation
 *
 * Public API:
 *  - void fmt_scan_table(const ScanTable *t, bool json, bool csv);
 *  - void fmt_traceroute(const TraceRoute *t, bool json, bool csv);
 *  - void fmt_monitor_series(const MonitorSeries *s, bool json, bool csv);
 */
#ifndef FMT_H
#define FMT_H

#include <stdbool.h>

struct ScanTable;
struct TraceRoute;
struct MonitorSeries;

void fmt_scan_table(const struct ScanTable *t, bool json, bool csv);
void fmt_traceroute(const struct TraceRoute *t, bool json, bool csv);
void fmt_monitor_series(const struct MonitorSeries *s, bool json, bool csv);

#endif /* FMT_H */
