/*
 * File: tracer.c
 * Implements TTL-stepped ICMP echo probes and hop collection.
 *
 * Implementation:
 *  - Raw socket sendto() ICMP ECHO with TTL=ttl
 *  - recvfrom() ICMP TIME_EXCEEDED or ECHO_REPLY
 *  - Measure RTT with timeutil; resolve IP to hostname optionally
 *
 * Requirements:
 *  - Root or CAP_NET_RAW for raw sockets
 *
 * Error Handling:
 *  - Clean teardown on signal/interrupts; timeouts per hop
 */

 // tracer.c
#include "tracer.h"

int tracer_run(const CommandLine *cfg, TraceRoute *out){
    (void)cfg; (void)out;
    return -1; // stub
}

void traceroute_free(TraceRoute *t){
    (void)t;
}
