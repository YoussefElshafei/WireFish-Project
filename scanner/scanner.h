/*
 * File: scanner.h
 * Summary: Public API for TCP port scanning (and optional host liveness)
 *
 * Responsibilities:
 *  - Scan a host/IP (or CIDR range, if extended) for open/closed/filtered TCP ports
 *  - Optionally measure connect latency per port
 *
 * Data & Types:
 *  - typedef enum PortState { PORT_CLOSED=0, PORT_OPEN=1, PORT_FILTERED=2 }
 *  - typedef struct ScanResult { int port; PortState state; int latency_ms; }
 *  - typedef struct ScanTable { ScanResult *rows; size_t len, cap; }
 *
 * Public API:
 *  - int  scanner_run(const Config *cfg, ScanTable *out);
 *  - void scantable_free(ScanTable *t);
 *
 * Inputs:
 *  - cfg->target (host/IP), cfg->ports_from..ports_to, timeout settings
 * Outputs:
 *  - out->rows entries with per-port state (+ optional latency)
 *
 * Returns:
 *  - 0 on success
 *  - -1 on error (invalid args, network unreachable, etc.)
 *
 * Thread-safety: stateless API; safe to call from multiple threads if 'out' is distinct.
 * Dependencies: config.h, net.h
 *
 * Notes:
 *  - Uses non-blocking connect or timeouts for responsiveness.
 *  - Extend later for parallel scanning or CIDR enumeration.
 *
 * Aryan Verma, 400575438, McMaster University
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>
#include <stdbool.h>

#include "../cli/cli.h"
#include "../model/model.h"

// Uses PortState, ScanResult, ScanTable from model.h

int scanner_run(const CommandLine *cfg, ScanTable *out);
void scantable_free(ScanTable *t);

#endif /* SCANNER_H */
