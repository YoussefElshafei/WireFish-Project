/*
 * File: tests/test_monitor.c
 * Summary: Unit tests for interface bandwidth monitoring using /proc/net/dev.
 *
 * Purpose:
 *  - Validate parsing of /proc/net/dev for a given interface.
 *  - Verify rate computations over time (bps) and rolling averages.
 *  - Ensure robustness to missing/nonexistent interfaces.
 *
 * Test Matrix (examples):
 *  - Happy path on loopback:
 *      cfg.iface = "lo", interval_ms small (e.g., 100–200 ms), duration_sec short (1–2 s).
 *      Expect non-decreasing byte counters; rates >= 0.
 *  - Nonexistent interface:
 *      cfg.iface = "nope0" → expect error return (no crash).
 *  - Smoothing:
 *      Enable ring buffer; verify avg tracks mean of recent samples.
 *  - Low-traffic vs burst:
 *      Optionally generate traffic (e.g., ping localhost) and confirm rate spike > baseline.
 *
 * What is Verified:
 *  - Return codes and series length > 0 on success.
 *  - Each sample has iface set, rx/tx bytes monotonic (except counter wrap edge case).
 *  - Rates are finite, >= 0; averages within expected bounds.
 *  - Proper resource cleanup (no leaks).
 *
 * Modules Covered:
 *  - monitor.c / monitor.h
 *  - ringbuf.c / ringbuf.h
 *  - timeutil.c (interval timing)
 *
 * Fixtures / Setup:
 *  - Read-only access to /proc/net/dev; skip test if file unavailable.
 *  - Interface name overridable via env (NETGUARD_TEST_IFACE).
 */
