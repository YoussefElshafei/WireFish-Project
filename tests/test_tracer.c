/*
 * File: tests/test_tracer.c
 * Summary: Unit tests for ICMP-based traceroute with TTL stepping.
 *
 * Purpose:
 *  - Confirm TTL increment logic and hop collection.
 *  - Verify RTT measurement per hop and timeout handling.
 *  - Ensure graceful behavior when raw sockets are not permitted.
 *
 * Test Matrix (examples):
 *  - Permissions check:
 *      If raw socket creation fails → expect specific error code (e.g., permission denied).
 *  - Short trace to loopback or gateway:
 *      cfg.target = "127.0.0.1" or default gateway; ttl_max small (e.g., 2–3).
 *      Expect ≥1 hop with valid RTT or timeout flag.
 *  - Timeout path:
 *      cfg.target = unreachable/non-routable IP; ensure timeouts are recorded and the run completes.
 *  - Reverse DNS (optional):
 *      If enabled, verify host field is non-empty or falls back to IP.
 *
 * What is Verified:
 *  - Return codes and non-crashing behavior under failures/timeouts.
 *  - Hops are ordered (hop numbers strictly increasing).
 *  - Each Hop has either (timeout == true && rtt_ms == -1) OR (timeout == false && rtt_ms >= 0).
 *  - No memory leaks (valgrind clean); buffers sized correctly.
 *
 * Modules Covered:
 *  - tracer.c / tracer.h
 *  - icmp.c / icmp.h (checksum + echo packet build)
 *  - net.c (raw socket, TTL), timeutil.c (RTT)
 *
 * Fixtures / Setup:
 *  - Tests may skip ICMP cases when lacking CAP_NET_RAW/root (mark as skipped but not failed).
 *  - Target host overridable via env (NETGUARD_TEST_TRACE_TARGET).
 */
