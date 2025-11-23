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

#include "cli.h"

/*
 * Function: parse_range
 * 
 * Created to parse a string like "80-443" into two integers (from=80, to=443)
 * This is a helper function used for both --ports and --ttl parsing
 *
 * Parameters:
 *   str  - The input string (ex, "80-443")
 *   from - Pointer to store the first number
 *   to   - Pointer to store the second number
 *
 * Returns:
 *   EXIT_SUCCESS if successful
 */
static void parse_range(const char *str, int *from, int *to) {

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
}

/*
 * Function: cli_parse
 * 
 * This is the main parsing function
 * It processes all command-line arguments and fills in the CommandLine struct
 * 
 * Parameters:
 *   out - pointer that points to a CommandLine struct which holds all the information about user input 
 * 
 * Returns:
 *   EXIT_SUCCESS if successful 
 */
int cli_parse(int argc, char *argv[], CommandLine *out) {
    
    // Initializing the struct with default values
    out->json = false;
    out->csv = false;
    out->mode = MODE_NONE;
    
    out->target[0] = '\0';  
    out->iface[0] = '\0';
    
    out->ports_from = DEFAULT_PORTS_FROM;
    out->ports_to = DEFAULT_PORTS_TO;
    out->ttl_start = DEFAULT_TTL_START;
    out->ttl_max = DEFAULT_TTL_MAX;
    out->interval_ms = DEFAULT_INTERVAL_MS;
    
    // Checking for help flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            cli_print_help();
            exit(EXIT_SUCCESS); 
        }
    }
    
    // Parsing every argument
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--scan") == 0) {
            // Check if mode was already set (only one should be set)
            if (out->mode != MODE_NONE) {
                fprintf(stderr, "Error: Only one mode (--scan, --trace, --monitor) allowed\n");
                exit(EXIT_FAILURE);
            }
            out->mode = MODE_SCAN;
        }
        else if (strcmp(argv[i], "--trace") == 0) {
            // Check if mode was already set (only one should be set)
            if (out->mode != MODE_NONE) {
                fprintf(stderr, "Error: Only one mode (--scan, --trace, --monitor) allowed\n");
                exit(EXIT_FAILURE);
            }
            out->mode = MODE_TRACE;
        }
        else if (strcmp(argv[i], "--monitor") == 0) {
            // Check if mode was already set (only one should be set)
            if (out->mode != MODE_NONE) {
                fprintf(stderr, "Error: Only one mode (--scan, --trace, --monitor) allowed\n");
                exit(EXIT_FAILURE);
            }
            out->mode = MODE_MONITOR;
        }
        
        // Output formats
        else if (strcmp(argv[i], "--json") == 0) {
            out->json = true;
        }
        else if (strcmp(argv[i], "--csv") == 0) {
            out->csv = true;
        }
        
        
        else if (strcmp(argv[i], "--target") == 0) {
            // Making sure there's a next argument for the target value since it requires a host target
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --target requires a hostname or IP\n");
                exit(EXIT_FAILURE);
            }
            
            // Move to the target value
            i++;  

            // Copy the value
            strncpy(out->target, argv[i], sizeof(out->target) - 1);

            // Making sure there's a null terminator
            out->target[sizeof(out->target) - 1] = '\0';  
        }

        else if (strcmp(argv[i], "--ports") == 0) {
            // Making sure there's a next argument
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --ports requires a range (ex, 80-443)\n");
                exit(EXIT_FAILURE);
            }
            
            // Parse the range
            i++;
            parse_range(argv[i], &out->ports_from, &out->ports_to);
        }

        else if (strcmp(argv[i], "--ttl") == 0) {
            // Making sure there's a next argument
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --ttl requires a range (e.g., 1-30)\n");
                exit(EXIT_FAILURE);
            }
            
            // Parse the range
            i++;
            parse_range(argv[i], &out->ttl_start, &out->ttl_max);
        }

        else if (strcmp(argv[i], "--iface") == 0) {
            // Making sure there's a next argument
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --iface requires an interface name (e.g., eth0)\n");
                exit(EXIT_FAILURE);
            }
            
            // Copying the interface name
            i++;
            strncpy(out->iface, argv[i], sizeof(out->iface) - 1);

            // Making sure there's a null terminator
            out->iface[sizeof(out->iface) - 1] = '\0';
        }

        else if (strcmp(argv[i], "--interval") == 0) {
            // Making sure there's a next argument
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --interval requires a number (milliseconds)\n");
                exit(EXIT_FAILURE);
            }
            
            // Convert to integer using strtol for proper validation
            i++;
            char *endptr;
            long interval_long = strtol(argv[i], &endptr, 10);
            
            // Check if conversion failed 
            if (endptr == argv[i] || *endptr != '\0') {
                fprintf(stderr, "Error: Invalid interval value '%s' (must be a number)\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            
            out->interval_ms = (int)interval_long;
            
            // Must be positive 
            if (out->interval_ms <= 0) {
                fprintf(stderr, "Error: Interval must be positive\n");
                exit(EXIT_FAILURE);
            }
        }
        
        else {
            fprintf(stderr, "Error: Unknown argument '%s'\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    
   
     // After parsing all arguments, check that the combination makes sense
    
    // Check if the user specified exactly one mode
    if (out->mode == MODE_NONE) {
        fprintf(stderr, "Error: Must specify one mode: --scan, --trace, or --monitor\n");
        exit(EXIT_FAILURE);
    }
    
    // Check that --scan and --trace have a target
    if ((out->mode == MODE_SCAN || out->mode == MODE_TRACE) && 
        out->target[0] == '\0') {
        fprintf(stderr, "Error: --target required for %s mode\n", out->mode == MODE_SCAN ? "scan" : "trace");
        exit(EXIT_FAILURE);
    }
    
    // Can't use both --json and --csv
    if (out->json && out->csv) {
        fprintf(stderr, "Error: Cannot use both --json and --csv\n");
        exit(EXIT_FAILURE);
    }
    

    // Check that options make sense for the selected mode
    
    // SCAN mode: validate port range was specified correctly
    if (out->mode == MODE_SCAN) {
        if (out->ports_from < MIN_PORT || out->ports_from > MAX_PORT || out->ports_to < MIN_PORT || out->ports_to > MAX_PORT) {
            fprintf(stderr, "Error: Ports must be in range %d-%d\n", MIN_PORT, MAX_PORT);
            exit(EXIT_FAILURE);
        }
    }
    
    // TRACE mode: validate TTL range
    if (out->mode == MODE_TRACE) {
        if (out->ttl_start < MIN_TTL || out->ttl_start > MAX_TTL || out->ttl_max < MIN_TTL || out->ttl_max > MAX_TTL) {
            fprintf(stderr, "Error: TTL values must be in range %d-%d\n", MIN_TTL, MAX_TTL);
            exit(EXIT_FAILURE);
        }
    }
    
    // MONITOR mode: validate interval 
    if (out->mode == MODE_MONITOR) {
        if (out->interval_ms <= 0) {
            fprintf(stderr, "Error: Interval must be positive\n");
            exit(EXIT_FAILURE);
        }
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


