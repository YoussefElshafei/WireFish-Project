/*
 * File: app.h
 * Summary: Top-level orchestrator (CLI → Config → Module → Formatter) / High-level application dispatcher for the DDOS/wirefish tool.
 *
 * Responsibilities:
 *  - Take a parsed CommandLine (from cli.c)
 *  - Dispatch to the correct feature module:
 *      * scanner (port scan)
 *      * tracer (traceroute)
 *      * monitor (interface monitoring)
 *  - Delegate all output formatting to fmt.c (table, CSV, JSON)
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#ifndef APP_H
#define APP_H

#include "../cli/cli.h"   // for CommandLine

int app_run(const CommandLine *cmd);

#endif /* APP_H */