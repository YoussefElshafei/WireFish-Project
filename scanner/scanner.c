/*
 * File: scanner.c
 * Implements TCP connect-based port scanning
 *
 * This file contains the core port scanning logic for wirefish
 * It scans TCP ports on a target and classifies them as OPEN, CLOSED, or FILTERED
 * It also measures connection latency for each port
 *
 * Implementation Notes:
 *  - Resolves cfg->target once; iterates port range
 *  - connect() with timeout (non-blocking via net_tcp_connect)
 *  - Classifies states: OPEN (connect OK), CLOSED (RST/refused), FILTERED (timeout)
 *  - Measures latency (connect start->end) for each port
 *
 * Error Handling:
 *  - Validates cfg fields; returns -1 on failure
 *  - Frees 'out' on error
 *
 * TODOs (future enhancements):
 *  - Parallel scanning (select/poll or pthreads)
 *  - IPv6 support toggle
 *  - CIDR host enumeration
 *
 * Aryan Verma, 400575438, McMaster University
 */

#include "scanner.h"
#include "../net/net.h"
#include "../cli/cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

// Default connection timeout for port scanning (milliseconds)
#define DEFAULT_CONNECT_TIMEOUT_MS 1000

// Initial capacity for ScanTable dynamic array
#define INITIAL_TABLE_CAPACITY 100

/*
 * Function: get_time_ms
 *
 * Purpose: Get current time in milliseconds for latency measurements
 * Returns: Current time in milliseconds since epoch
 */
static long long get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // Convert seconds to milliseconds and add microseconds converted to milliseconds
    return (long long)tv.tv_sec * 1000LL + (long long)tv.tv_usec / 1000LL;
}

/*
 * Function: scantable_init
 *
 * Purpose: Initialize a ScanTable with initial capacity
 * Parameters: t - Pointer to ScanTable to initialize
 * Returns: 0 on success, -1 on memory allocation failure
 */
static int scantable_init(ScanTable *t) {
    // Allocate initial space for scan results
    t->rows = malloc(INITIAL_TABLE_CAPACITY * sizeof(ScanResult));
    if (!t->rows) {
        fprintf(stderr, "Error: Memory allocation failed for ScanTable\n");
        return -1;
    }
    
    t->len = 0;
    t->cap = INITIAL_TABLE_CAPACITY;
    
    return 0;
}

/*
 * Function: scantable_add
 *
 * Purpose: Add a scan result to the table (automatically grows array if needed)
 * Parameters:
 *   t - Pointer to ScanTable
 *   port - Port number
 *   state - Port state (OPEN/CLOSED/FILTERED)
 *   latency_ms - Connection latency in milliseconds (-1 if failed)
 * Returns: 0 on success, -1 on memory allocation failure
 */
static int scantable_add(ScanTable *t, int port, PortState state, int latency_ms) {
    // Check if we need to grow the array
    if (t->len >= t->cap) {
        size_t new_cap = t->cap * 2;
        
        ScanResult *new_rows = realloc(t->rows, new_cap * sizeof(ScanResult));
        if (!new_rows) {
            fprintf(stderr, "Error: Memory reallocation failed for ScanTable\n");
            return -1;
        }
        
        t->rows = new_rows;
        t->cap = new_cap;
    }
    
    // Add the new result
    t->rows[t->len].port = port;
    t->rows[t->len].state = state;
    t->rows[t->len].latency_ms = latency_ms;
    t->len++;
    
    return 0;
}

/*
 * Function: scantable_free
 *
 * Purpose: Free memory allocated for a ScanTable
 * Parameters: t - Pointer to ScanTable to free
 */
void scantable_free(ScanTable *t) {
    if (t && t->rows) {
        free(t->rows);
        t->rows = NULL;
        t->len = 0;
        t->cap = 0;
    }
}

/*
 * Function: scanner_run
 *
 * Purpose: Main scanning function - scans all ports in specified range
 *
 * Parameters:
 *   cfg - CommandLine configuration containing target, port range
 *   out - Pointer to ScanTable to store results
 *
 * Returns: 0 on success, -1 on error
 */
int scanner_run(const CommandLine *cfg, ScanTable *out) {
    // STEP 1: VALIDATE INPUT
    
    // Check for NULL pointers
    if (!cfg || !out) {
        fprintf(stderr, "Error: NULL pointer passed to scanner_run\n");
        return -1;
    }
    
    // Check that target is not empty
    if (cfg->target[0] == '\0') {
        fprintf(stderr, "Error: No target specified for scan\n");
        return -1;
    }
    
    // Validate port range
    if (cfg->ports_from < 1 || cfg->ports_from > 65535 ||
        cfg->ports_to < 1 || cfg->ports_to > 65535 ||
        cfg->ports_from > cfg->ports_to) {
        fprintf(stderr, "Error: Invalid port range %d-%d\n", 
                cfg->ports_from, cfg->ports_to);
        return -1;
    }
    
    // Initialize scan table
    
    if (scantable_init(out) < 0) {
        return -1;
    }
    
    // Convert hostname to IP address (do this once before scanning)
    
    struct sockaddr_storage target_addr;
    socklen_t target_addrlen;
    
    if (net_resolve(cfg->target, &target_addr, &target_addrlen) < 0) {
        fprintf(stderr, "Error: Failed to resolve target '%s'\n", cfg->target);
        scantable_free(out);
        return -1;
    }
    
    // Scan each port
    // Test each port in the range
    
    for (int port = cfg->ports_from; port <= cfg->ports_to; port++) {
        // Set up socket address for this port
        // Reuse the resolved IP address and just change the port
        struct sockaddr_in scan_addr;
        memcpy(&scan_addr, &target_addr, sizeof(struct sockaddr_in));
        scan_addr.sin_port = htons(port);
        
        // Record start time for latency measurement
        long long start_time = get_time_ms();
        
        // Attempt TCP connection with timeout
        int sockfd = net_tcp_connect((struct sockaddr *)&scan_addr, sizeof(scan_addr), DEFAULT_CONNECT_TIMEOUT_MS);
        
        // Record end time and calculate latency
        long long end_time = get_time_ms();
        int latency_ms = (int)(end_time - start_time);
        
        // Classify the result based on connection outcome
        PortState state;
        
        if (sockfd >= 0) {
            // Connection succeeded, port is OPEN
            state = PORT_OPEN;
            close(sockfd);
            
        } else {
            // Connection failed - check errno to determine why
            
            if (errno == ECONNREFUSED) {
                // Server actively refused connection, port is closed
                state = PORT_CLOSED;
                // No meaningful latency for refused connections
                latency_ms = -1;  
                
            } else {
                // Timeout or other error, port is FILTERED
                state = PORT_FILTERED;
                // No meaningful latency for timeouts
                latency_ms = -1;  
            }
        }
        
        // Store the result in our table
        if (scantable_add(out, port, state, latency_ms) < 0) {
            fprintf(stderr, "Error: Failed to store scan result\n");
            scantable_free(out);
            return -1;
        }
    }
    
    return 0;
}