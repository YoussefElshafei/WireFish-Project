/*
 * File: tests/test_scanner.c
 * Summary: Unit tests for the TCP port-scanning module.
 *
 * Purpose:
 *  - Validate port range parsing and iteration.
 *  - Verify TCP connect-based classification of OPEN/CLOSED/FILTERED.
 *  - Ensure timeouts and latency measurements behave deterministically.
 *
 * Test Matrix (examples):
 *  - Localhost happy path:
 *      cfg.target = "127.0.0.1", ports 1–3 (expect mostly CLOSED)
 *  - Known-open port (if available):
 *      cfg.target = "127.0.0.1", port of a test server started by the test (expect OPEN)
 *  - Timeout behavior:
 *      cfg.target = "10.255.255.1" (non-routable) or firewall-blocked host (expect FILTERED/timeout)
 *  - Input validation:
 *      ports_from > ports_to → expect error return
 *      ports outside [1..65535] → expect error return
 *
 * What is Verified:
 *  - Return codes (0 on success; <0 on invalid args or runtime failure).
 *  - For each scanned port: state ∈ {OPEN, CLOSED, FILTERED}.
 *  - Latency field is either a non-negative ms value (when measured) or -1.
 *  - Out table allocated and freed without leaks (valgrind clean run).
 *
 * Modules Covered:
 *  - scanner.c / scanner.h
 *  - net.c (connect with timeout), timeutil.c (latency), config.c (sanity)
 *
 * Fixtures / Setup:
 *  - Optional ephemeral TCP server bound to 127.0.0.1:0 to guarantee an OPEN port.
 *  - Environment variable override for target/port range (e.g., NETGUARD_TEST_TARGET).
 */
