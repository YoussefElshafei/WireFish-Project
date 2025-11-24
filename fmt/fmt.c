/*
 * File: fmt.c
 * Implements table/CSV/JSON rendering for all public models.
 *
 * Notes:
 *  - Writes to stdout; errors to stderr handled by caller
 *  - Keep schemas stable for tooling integration
 * 
 * Responsibilities:
 * - Render ScanTable, TraceRoute, MonitorSeries in consistent schema
 * - Avoid business logic; pure presentation
 */

#include "model.h"
#include "fmt.h"
#include <stdio.h>
#include <stdbool.h>

/**
 * Helper to convert PortState enum to string.
 * @param state PortState enum value
 * @return Corresponding string representation
 */
static const char *port_state_str(int state){

    //Note: check with aryan if he created typedef enum and set values for PORT_CLOSED = 0,PORT_OPEN = 1, PORT_FILTERED = 2 and setting them to PortState
    //updated note: just went into model and changed it myself
    if(state == PORT_CLOSED){
        return "closed";
    } 
    else if(state == PORT_OPEN){
        return "open";
    } 

    else if(state == PORT_FILTERED){
        return "filtered";
    }
    else {
        return "Unknown";
    }
}

/**
 * Format ScanTable in table format.
 * @param scan_table Pointer to ScanTable
 * @return void
 */
static void fmt_scan_table_table(const ScanTable *scan_table){

    printf("PORT  STATE      LATENCY(ms)\n");
    printf("----  ---------  ----------\n");

    for(size_t i = 0; i < scan_table->len; i++){

        // Pointer to the current result row
        const ScanResult *row = &scan_table->rows[i];

        printf("%-4d  %-9s  ", row->port, port_state_str(row->state));

        if(row->latency_ms >= 0){
            printf("%d\n", row->latency_ms);
        } 
        
        else{
            printf("-\n");
        }
    }
}

/**
 * Format ScanTable in CSV format.
 * @param scan_table Pointer to ScanTable
 * @return void
 */
static void fmt_scan_table_csv(const ScanTable *scan_table){

    printf("port,state,latency_ms\n");

    for(size_t i = 0; i < scan_table->len; i++){

        const ScanResult *row = &scan_table->rows[i];

        printf("%d,%s,", row->port, port_state_str(row->state));

        if(row->latency_ms >= 0){
            printf("%d\n", row->latency_ms);
        } 
        
        else{
            // no latency measured → leave blank field
            printf("\n");
        }
    }
}

/**
 * Format ScanTable in JSON format.
 * @param scan_table Pointer to ScanTable
 * @return void
 */
static void fmt_scan_table_json(const ScanTable *scan_table){

    printf("{\"type\":\"scan\",\"results\":[");
    
    for(size_t i = 0; i < scan_table->len; i++){

        const ScanResult *row = &scan_table->rows[i];

        if(i > 0){
            printf(",");
        }

        printf("{\"port\":%d,\"state\":\"%s\",", row->port, port_state_str(row->state));

        if(row->latency_ms >= 0){
            printf("\"latency_ms\":%d}", row->latency_ms);
        } 
        
        else{
            printf("\"latency_ms\":null}");
        }
    }

    printf("]}\n");
}

/**
 * Format ScanTable in specified format.
 * @param table Pointer to ScanTable
 * @param json If true, output in JSON format
 * @param csv If true, output in CSV format
 * @return void
 */
void fmt_scan_table(const struct ScanTable *table, bool json, bool csv){

    if(json){
        fmt_scan_table_json(table);
    }
    
    else if(csv){
        fmt_scan_table_csv(table);
    } 
    
    else{
        fmt_scan_table_table(table);
    }
}

/**
 * Format TraceRoute in CSV format.
 * @param route Pointer to TraceRoute
 * @return void
 */
static void fmt_traceroute_csv(const TraceRoute *route){

    printf("hop,ip,host,rtt_ms,timeout\n");

    for(size_t i = 0; i < route->len; i++){

        const Hop *current_hop = &route->rows[i];

        //Note: For safety, we could quote host if it might contain commas, but for now assume it doesn't. Ask team if needed.
        if(current_hop->rtt_ms >= 0 && !current_hop->timeout){
            printf("%d,%s,%s,%d,%s\n",
                   current_hop->hop,
                   current_hop->ip,
                   current_hop->host,
                   current_hop->rtt_ms,
                   current_hop->timeout ? "true" : "false");
        } 
        
        else{
            // timeout or unknown RTT (Round Trip Time)
            printf("%d,%s,%s,-,%s\n",
                   current_hop->hop,
                   current_hop->ip,
                   current_hop->host,
                   current_hop->timeout ? "true" : "false");
        }
    }
}

/**
 * Format TraceRoute in JSON format.
 * @param route Pointer to TraceRoute
 * @return void
 */
static void fmt_traceroute_json(const TraceRoute *route){
    
    printf("{\"type\":\"trace\",\"hops\":[");
    
    for(size_t i = 0; i < route->len; i++){

        const Hop *current_hop = &route->rows[i];

        if(i > 0){
            printf(",");
        }

        printf("{\"hop\":%d,\"ip\":\"%s\",\"host\":\"%s\",",
               current_hop->hop, current_hop->ip, current_hop->host);

        if(current_hop->timeout || current_hop->rtt_ms < 0){
            printf("\"rtt_ms\":null,\"timeout\":true}");
        } 
        
        else{
            printf("\"rtt_ms\":%d,\"timeout\":%s}",
                   current_hop->rtt_ms,
                   current_hop->timeout ? "true" : "false");
        }
    }

    printf("]}\n");
}

/**
 * Format TraceRoute in table format.
 * @param route Pointer to TraceRoute
 * @return void
 */
static void fmt_traceroute_table(const TraceRoute *route){

    printf("HOP  IP               HOST                       RTT(ms)  STATUS\n");
    printf("---  ---------------- -------------------------- -------  ------\n");

    for(size_t i = 0; i < route->len; i++){

        const Hop *current_hop = &route->rows[i];

        const char *status = current_hop->timeout ? "TIMEOUT" : "OK";

        if(current_hop->timeout || current_hop->rtt_ms < 0) {
            printf("%-3d  %-16s %-26s %-7s  %s\n", current_hop->hop, current_hop->ip, current_hop->host, "-", status);
        } 
        
        else{
            printf("%-3d  %-16s %-26s %-7d  %s\n", current_hop->hop, current_hop->ip, current_hop->host, current_hop->rtt_ms, status);
        }
    }
}

/**
 * Format TraceRoute in specified format.
 * @param route Pointer to TraceRoute
 * @param json If true, output in JSON format
 * @param csv If true, output in CSV format
 * @return void
 */
void fmt_traceroute(const struct TraceRoute *route, bool json, bool csv){

    if(json){
        fmt_traceroute_json(route);
    } 
    
    else if(csv){
        fmt_traceroute_csv(route);
    }
    
    else{
        fmt_traceroute_table(route);
    }
}

/**
 * Format MonitorSeries in CSV format.
 * @param series Pointer to MonitorSeries
 * @return void
 */
static void fmt_monitor_series_csv(const MonitorSeries *series){

    printf("iface,rx_bytes,tx_bytes,rx_bps,tx_bps,rx_avg_bps,tx_avg_bps\n");

    for(size_t i = 0; i < series->len; i++){
        
        const IfaceStats *sample = &series->samples[i];

        printf("%s,%llu,%llu,%.2f,%.2f,%.2f,%.2f\n",
               sample->iface,
               sample->rx_bytes,
               sample->tx_bytes,
               sample->rx_rate_bps,
               sample->tx_rate_bps,
               sample->rx_avg_bps,
               sample->tx_avg_bps);
    }
}

/**
 * Format MonitorSeries in JSON format.
 * @param series Pointer to MonitorSeries
 * @return void
 */
static void fmt_monitor_series_json(const MonitorSeries *series){

    printf("{\"type\":\"monitor\",\"samples\":[");
    
    for(size_t i = 0; i < series->len; i++){

        const IfaceStats *sample = &series->samples[i];

        if(i > 0){
            printf(",");
        }

        printf("{\"iface\":\"%s\",\"rx_bytes\":%llu,\"tx_bytes\":%llu,"
               "\"rx_bps\":%.2f,\"tx_bps\":%.2f,"
               "\"rx_avg_bps\":%.2f,\"tx_avg_bps\":%.2f}",
               sample->iface,
               sample->rx_bytes,
               sample->tx_bytes,
               sample->rx_rate_bps,
               sample->tx_rate_bps,
               sample->rx_avg_bps,
               sample->tx_avg_bps);
    }

    printf("]}\n");
}

/**
 * Format MonitorSeries in table format.
 * @param series Pointer to MonitorSeries
 * @return void
 */
static void fmt_monitor_series_table(const MonitorSeries *series){

    printf("IFACE  RX_BYTES   TX_BYTES   RX_BPS      TX_BPS      RX_AVG_BPS   TX_AVG_BPS\n");
    printf("-----  --------   --------   ----------  ----------  -----------  -----------\n");

    for(size_t i = 0; i < series->len; i++){

        const IfaceStats *sample = &series->samples[i];

        printf("%-5s  %-8llu  %-8llu  %-10.2f  %-10.2f  %-11.2f  %-11.2f\n",
               sample->iface,
               sample->rx_bytes,
               sample->tx_bytes,
               sample->rx_rate_bps,
               sample->tx_rate_bps,
               sample->rx_avg_bps,
               sample->tx_avg_bps);
    }
}

/**
 * Format MonitorSeries in specified format.
 * @param series Pointer to MonitorSeries
 * @param json If true, output in JSON format
 * @param csv If true, output in CSV format
 * @return void
 */
void fmt_monitor_series(const struct MonitorSeries *series, bool json, bool csv){

    if (json){
        fmt_monitor_series_json(series);
    } 
    
    else if (csv){
        fmt_monitor_series_csv(series);
    } 
    
    else{
        fmt_monitor_series_table(series);
    }
}


