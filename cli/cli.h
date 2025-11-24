/*
 * File: cli.h
 * Summary: Command-line parsing (--scan, --trace, --monitor, flags)
 *
 * Responsibilities:
 *  - Parse argc and argv into a CommandLine struct
 *  - Validate args and provide help text
 *
 * Function prototypes:
 *  - int cli_parse(int argc, char *argv[], CommandLine *out)
 *  - void cli_print_help()
 * 
 * Aryan Verma, 400575438, McMaster University 
 */

#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

// Defaults and ranges
#define DEFAULT_PORTS_FROM 1
#define DEFAULT_PORTS_TO 1024
#define DEFAULT_TTL_START 1
#define DEFAULT_TTL_MAX 30
#define DEFAULT_INTERVAL_MS 100

#define MIN_PORT 1
#define MAX_PORT 65535
#define MIN_TTL 1
#define MAX_TTL 255

typedef struct{
    bool json, csv;

    char target[256];
    char iface[64];

    int ports_from, ports_to;
    int ttl_start, ttl_max;
    int interval_ms;

    enum{
        MODE_NONE=0,
        MODE_SCAN,
        MODE_TRACE,
        MODE_MONITOR
    }mode;
}CommandLine;

int cli_parse(int argc, char *argv[], CommandLine *out);
void cli_print_help(void);

#endif /* CLI_H */
