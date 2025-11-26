/*
 * File: tracer.h
 * Summary: Public API for ICMP-based traceroute.
 *
 * Responsibilities:
 *  - Probe path to target by incrementing TTL
 *  - Capture per-hop RTT and IP/hostname (optional reverse DNS)
 *
 * Data & Types:
 *  - typedef struct Hop { int hop; char host[256]; char ip[64]; int rtt_ms; bool timeout; }
 *  - typedef struct TraceRoute { Hop *rows; size_t len, cap; }
 *
 * Public API:
 *  - int  tracer_run(const Config *cfg, TraceRoute *out);
 *  - void traceroute_free(TraceRoute *t);
 *
 * Inputs:
 *  - cfg->target, cfg->ttl_start..ttl_max, per-probe timeout
 * Outputs:
 *  - Ordered hops with RTT or timeout flag
 *
 * Returns:
 *  - 0 on success; <0 on error (permissions for raw sockets, resolve fail, etc.)
 *
 * Thread-safety: Stateless; each call owns its TraceRoute buffer.
 * Dependencies: icmp.h, net.h, timeutil.h, config.h
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#ifndef TRACER_H
#define TRACER_H

#include <stddef.h>
#include <stdbool.h>
#include "../cli/cli.h"
#include "../model/model.h"

int  tracer_run(const CommandLine *cmd, TraceRoute *out);
void traceroute_free(TraceRoute *route);
void traceroute_free(TraceRoute *t);

#endif /* TRACER_H */

