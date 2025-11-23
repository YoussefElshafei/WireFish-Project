/*
 * File: cli.c
 * Implements command-line argument parsing and help text
 *
 * This file handles all command-line argument parsing for wirefish.
 *
 * Behavior:
 *  - Populates CommandLine with defaults and parsed values
 *  - Returns EXIT_SUCCESS on success and EXIT_FAILURE on failure 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cli.h"

/*
 * Function: parse_range
 * 
 * Created to parse a string like "80-443" into two integers (from=80, to=443)
 * This is a helper function used for both --ports and --ttl parsing
 *
 * Parameters:
 *   str  - The input string (e.g., "80-443")
 *   from - Pointer to store the first number
 *   to   - Pointer to store the second number
 *
 * Returns:
 *   EXIT_SUCCESS if successful
 */
static int parse_range(const char *str, int *from, int *to) {

    // Find the dash character that separates the two numbers (returns a pointer that points to the dash)
    const char *dash = strchr(str, '-');
    
    // Check if the user provided a dash
    if (!dash) {
        fprintf(stderr, "Error: Range must be in format 'from-to' (ex, 80-443)\n");
        exit(EXIT_FAILURE);
    }
    
    char *endptr;
    
    // Extracting the start of the range
    long from_long = strtol(str, &endptr, 10);

    // Checking if the conversion happened or were there invalid characters before the dash
    if (endptr == str) {
        fprintf(stderr, "Error: Invalid number before '-' in range '%s'\n", str);
        exit(EXIT_FAILURE);
    }
    
    // Checking if the range is valid (checking if we made it to the location of the dash)
    if (endptr != dash) {
        fprintf(stderr, "Error: Invalid characters in range '%s'\n", str);
        exit(EXIT_FAILURE);
    }
    
    long to_long = strtol(dash + 1, &endptr, 10);

    // Checking if the conversion happened or were there invalid characters after the dash
    if (endptr == dash + 1) {
        fprintf(stderr, "Error: Invalid number after '-' in range '%s'\n", str);
        exit(EXIT_FAILURE);
    }
    
    // Checking if we made it to the end of the string without any errors
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid characters at end of range '%s'\n", str);
        exit(EXIT_FAILURE);
    }

    *from = (int)from_long;
    *to = (int)to_long;
    
    // Making sure that 'from' is not greater than 'to'
    if (*from > *to) {
        fprintf(stderr, "Error: Range start (%d) cannot be greater than end (%d)\n", *from, *to);
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}


/*
 * Function: cli_print_help
 * 
 * Prints usage information to stdout
 * This is shown when the user types --help or makes an error
 */
void cli_print_help() {
    printf("Usage: wirefish [MODE] [OPTIONS]\n\n");
    
    printf("WireFish - Network reconnaissance and monitoring tool\n\n");
    
    printf("Modes (choose one):\n");
    printf("  --scan              TCP port scanning\n");
    printf("  --trace             ICMP traceroute\n");
    printf("  --monitor           Network interface monitoring\n\n");
    
    printf("Scan Options:\n");
    printf("  --target <host>     Target hostname or IP (required)\n");
    printf("  --ports <from-to>   Port range (default: %d-%d)\n\n", DEFAULT_PORTS_FROM, DEFAULT_PORTS_TO);
    
    printf("Trace Options:\n");
    printf("  --target <host>     Target hostname or IP (required)\n");
    printf("  --ttl <start-max>   TTL range (default: %d-%d)\n\n", DEFAULT_TTL_START, DEFAULT_TTL_MAX);
    
    printf("Monitor Options:\n");
    printf("  --iface <name>      Network interface (default: auto-detect)\n");
    printf("  --interval <ms>     Sample interval in milliseconds (default: %d)\n\n", DEFAULT_INTERVAL_MS);
    
    printf("Output Options:\n");
    printf("  --json              Output in JSON format\n");
    printf("  --csv               Output in CSV format\n\n");
    
    printf("Other:\n");
    printf("  --help              Show this help message\n\n");
    
    printf("Examples:\n");
    printf("  wirefish --scan --target google.com --ports 80-443\n");
    printf("  wirefish --trace --target 8.8.8.8 --json\n");
    printf("  wirefish --monitor --iface eth0 --interval 500\n");
}


