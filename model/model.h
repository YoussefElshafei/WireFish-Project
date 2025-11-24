/*
 * File: model.h
 * Summary: Shared data models used by fmt/* and public APIs.
 *
 * Purpose:
 *  - Centralize the common structs so formatters can include a single header
 *  - Avoid circular includes between feature modules
 *
 * Contains:
 *  - typedefs mirrored from scanner.h (ScanResult, ScanTable)
 *  - typedefs mirrored from tracer.h  (Hop, TraceRoute)
 *  - typedefs mirrored from monitor.h (IfaceStats, MonitorSeries)
 *
 * Note:
 *  - Keep in sync with feature headers or include them conditionally.
 */
#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {PORT_CLOSED = 0, PORT_OPEN = 1, PORT_FILTERED = 2 } PortState;

typedef struct ScanResult{
    int port;
    PortState state;
    int latency_ms;
} ScanResult;

typedef struct ScanTable{
    ScanResult *rows;
    size_t len, cap;
} ScanTable;

typedef struct Hop{
    int hop;
    char host[256];
    char ip[64];
    int rtt_ms;
    bool timeout;
} Hop;

typedef struct TraceRoute{
    Hop *rows;
    size_t len, cap;
} TraceRoute;

typedef struct IfaceStats{
    char iface[64];
    unsigned long long rx_bytes, tx_bytes;
    double rx_rate_bps, tx_rate_bps;
    double rx_avg_bps, tx_avg_bps;
} IfaceStats;

typedef struct MonitorSeries{
    IfaceStats *samples;
    size_t len, cap;
} MonitorSeries;


#endif
