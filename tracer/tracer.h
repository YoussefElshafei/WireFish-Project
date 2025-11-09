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
 */
#ifndef TRACER_H
#define TRACER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
  int  hop;
  char host[256];
  char ip[64];
  int  rtt_ms;    /* -1 if timeout */
  bool timeout;
} Hop;

typedef struct {
  Hop *rows;
  size_t len, cap;
} TraceRoute;

struct Config;

int  tracer_run(const struct Config *cfg, TraceRoute *out);
void traceroute_free(TraceRoute *t);

#endif /* TRACER_H */
