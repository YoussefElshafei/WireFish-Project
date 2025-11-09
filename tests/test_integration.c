/*
 * File: tests/test_integration.c
 * Summary: End-to-end smoke tests spanning CLI → Config → Module → Formatter.
 *
 * Purpose:
 *  - Validate CLI parsing, config building, and module dispatch paths.
 *  - Exercise real flows for: --scan, --trace, and --monitor with sample args.
 *  - Sanity-check formatted outputs (table/CSV/JSON) for expected keys/tokens.
 *
 * Tests (examples):
 *  - Scan flow:
 *      argv = {"DDOS","--scan","127.0.0.1","--ports","80-81","--json"}
 *      Expect exit code 0; output contains "port" and "state" fields in JSON.
 *  - Trace flow (skip if no raw perms):
 *      argv = {"DDOS","--trace","8.8.8.8","--ttl","1-3"}
 *      Expect exit code 0; output includes hop numbers and RTTs/timeouts.
 *  - Monitor flow:
 *      argv = {"DDOS","--monitor","--iface","lo","--interval","200","--csv"}
 *      Expect exit code 0; output contains header with rx_bps, tx_bps columns.
 *  - Help & errors:
 *      argv = {"DDOS","--help"} → exit 0, prints usage.
 *      argv = {"DDOS","--scan","127.0.0.1","--ports","99999-2"} → nonzero exit, error msg to stderr.
 *
 * What is Verified:
 *  - Correct exit codes and no crashes.
 *  - Outputs contain canonical tokens/columns for each mode.
 *  - Logging to stderr doesn’t leak into stdout (when parsing output).
 *
 * Modules Covered:
 *  - cli.c, config.c, fmt.c, log.c, app.c
 *  - (Indirectly touches: scanner.c, tracer.c, monitor.c)
 *
 * Fixtures / Setup:
 *  - Gate tracer tests on raw-socket capability; mark as skipped when not available.
 *  - Allow env overrides for target, iface, and formats.
 */
