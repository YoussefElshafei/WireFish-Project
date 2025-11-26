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
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>
#include <stdbool.h>

// PortState enum for port scanning
typedef enum {PORT_CLOSED = 0, PORT_OPEN = 1, PORT_FILTERED = 2 } PortState;

/**
 * Data model for a single port scan result.
 * - port: TCP port number
 * - state: PortState enum (open/closed/filtered)
 * - latency_ms: Measured latency in milliseconds (-1 if not measured)
 */
typedef struct ScanResult{
    int port;
    PortState state;
    int latency_ms;
} ScanResult;

/**
 * Data model for a table of port scan results.
 * - rows: Dynamically allocated array of ScanResult
 * - len: Number of valid entries in rows
 * - cap: Allocated capacity of rows
 */
typedef struct ScanTable{
    ScanResult *rows;
    size_t len, cap;
} ScanTable;

/**
 * Data model for a single traceroute hop.
 * - hop: Hop number (TTL)
 * - host: Resolved hostname (or "?" if unknown)
 * - ip: IP address as string
 * - rtt_ms: Round-trip time in milliseconds (-1 if timeout)
 * - timeout: true if the hop timed out
 * - icmp_type: ICMP type received (e.g., ICMP_ECHOREPLY, ICMP_TIME_EXCEEDED)
 */
typedef struct Hop{
    int hop;
    char host[256];
    char ip[64];
    int rtt_ms;
    bool timeout;
    int  icmp_type;  // 0 = ECHO_REPLY, 11 = TIME_EXCEEDED, etc.
} Hop;

/**
 * Data model for a traceroute result.
 * - rows: Dynamically allocated array of Hop
 * - len: Number of valid entries in rows
 * - cap: Allocated capacity of rows
 */
typedef struct TraceRoute{
    Hop *rows;
    size_t len, cap;
} TraceRoute;

/**
 * Data model for interface statistics sample.
 * - iface: Interface name
 * - rx_bytes: Total received bytes
 * - tx_bytes: Total transmitted bytes
 * - rx_rate_bps: Receive rate in bits per second
 */
typedef struct IfaceStats{
    char iface[64];
    unsigned long long rx_bytes, tx_bytes;
    double rx_rate_bps, tx_rate_bps;
    double rx_avg_bps, tx_avg_bps;
} IfaceStats;

/**
 * Data model for a series of interface statistics samples.
 * - samples: Dynamically allocated array of IfaceStats
 * - len: Number of valid entries in samples
 * - cap: Allocated capacity of samples
 */
typedef struct MonitorSeries{
    IfaceStats *samples;
    size_t len, cap;
} MonitorSeries;

#endif /* MODEL_H */