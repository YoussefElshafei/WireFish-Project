/*
 * File: app.c
 * Implements main control flow and error-to-exit-code mapping / Implements the high-level dispatcher for the DDOS/wirefish tool.
 *
 * Responsibilities:
 *  - Look at CommandLine.mode and choose which feature to run:
 *      * MODE_SCAN    → port scanner
 *      * MODE_TRACE   → traceroute
 *      * MODE_MONITOR → interface monitor
 *  - Call the corresponding module (scanner/tracer/monitor)
 *  - Pass results to fmt.c for table/CSV/JSON output
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#include "app.h"
#include "../cli/cli.h"
#include "../scanner/scanner.h"
#include "../tracer/tracer.h"
#include "../monitor/monitor.h"
#include "../fmt/fmt.h"
#include "../model/model.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_MONITOR_SAMPLES 10 //how many samples to collect in monitor mode

/**
 * Run port scanner feature
 * @param cmd Pointer to CommandLine
 * @return 0 on success, non-zero error code on failure
 */
static int run_scan(const CommandLine *cmd){

    //Initialize empty ScanTable
    ScanTable table = {0};

    int scan_result = scanner_run(cmd, &table);

    if(scan_result != 0){

        fprintf(stderr, "Scan failed (code %d).\n", scan_result);
        return scan_result;
    }

    fmt_scan_table(&table, cmd->json, cmd->csv);

    scantable_free(&table);   // <- if scanner allocates rows, this is where you free

    return 0;
}

/**
 * Run traceroute feature
 * @param cmd Pointer to CommandLine
 * @return 0 on success, non-zero error code on failure
 */
static int run_trace(const CommandLine *cmd){

    //Initialize empty TraceRoute
    TraceRoute route = {0};

    int trace_result = tracer_run(cmd, &route);

    if(trace_result != 0){

        fprintf(stderr, "Traceroute failed (code %d).\n", trace_result);
        return trace_result;
    }

    fmt_traceroute(&route, cmd->json, cmd->csv);

    traceroute_free(&route);  // <- if tracer allocates rows, this is where you free

    return 0;
}

/**
 * Run interface monitor feature
 * @param cmd Pointer to CommandLine
 * @return 0 on success, non-zero error code on failure
 */
static int run_monitor(const CommandLine *cmd){

    // If user passed --iface, use it; otherwise let monitor_run auto-detect
    const char *iface = (cmd->iface[0] != '\0') ? cmd->iface : NULL;

    int interval_ms = cmd->interval_ms;  // from CLI defaults / --interval
    int samples = DEFAULT_MONITOR_SAMPLES;

    // Approximate duration (in seconds) for N samples at given interval
    // duration_sec ≈ samples * interval_ms / 1000
    int duration_sec = (samples * interval_ms + 999) / 1000;  // round up

    // Output model
    MonitorSeries series = {0};

    int monitor_result = monitor_run(iface, interval_ms, duration_sec, &series);

    if(monitor_result != 0){
        fprintf(stderr, "Error: monitor mode failed\n");
        monitorseries_free(&series);
        return -1;
    }

    // Now display via fmt.c (table/CSV/JSON)
    fmt_monitor_series(&series, cmd->json, cmd->csv);

    monitorseries_free(&series);
    return 0;
}


/**
 * Run the application based on CommandLine mode
 * @param cmd Pointer to CommandLine
 */
int app_run(const CommandLine *cmd){

    if(cmd == NULL){
        fprintf(stderr, "Internal error: app_run received NULL CommandLine pointer.\n");
        return -1;
    }

    else if(cmd->mode == MODE_SCAN){

        return run_scan(cmd);
    } 
    
    else if(cmd->mode == MODE_TRACE){

        return run_trace(cmd);
    } 
    
    else if(cmd->mode == MODE_MONITOR){

        return run_monitor(cmd);
    } 
    
    else{

        fprintf(stderr, "Internal error: app_run called with MODE_NONE or unknown mode.\n");
        return -1;
    }
}