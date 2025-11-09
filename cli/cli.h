/*
 * File: cli.h
 * Summary: Command-line parsing (--scan, --trace, --monitor, flags).
 *
 * Responsibilities:
 *  - Parse argc/argv into a CommandLine struct
 *  - Validate raw args and provide help text
 *
 * Public API:
 *  - int  cli_parse(int argc, char **argv, CommandLine *out);
 *  - void cli_print_help(const char *prog);
 */
#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

typedef struct {
  bool show_help, json, csv;
  char target[256];
  char iface[64];
  int ports_from, ports_to;
  int ttl_start, ttl_max;
  int interval_ms;
  enum { MODE_NONE=0, MODE_SCAN, MODE_TRACE, MODE_MONITOR } mode;
} CommandLine;

int  cli_parse(int argc, char **argv, CommandLine *out);
void cli_print_help(const char *prog);

#endif /* CLI_H */
