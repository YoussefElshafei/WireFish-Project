/*
 * File: monitor.c
 * Implementation of interface bandwidth monitoring.
 * 
 * Reads /proc/net/dev to get RX/TX byte counters and computes rates.
 */
#include "monitor.h"
#include "timeutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define PROC_NET_DEV "/proc/net/dev"
#define WINDOW_SIZE 10  // For rolling average

static volatile int running = 1;

/* Ring buffer for rolling average */
typedef struct {
    double *values;
    size_t size;
    size_t head;
    size_t count;
} RingBuffer;

static RingBuffer* ringbuf_create(size_t size) {
    RingBuffer *rb = malloc(sizeof(RingBuffer));
    if (!rb) return NULL;
    
    rb->values = calloc(size, sizeof(double));
    if (!rb->values) {
        free(rb);
        return NULL;
    }
    
    rb->size = size;
    rb->head = 0;
    rb->count = 0;
    return rb;
}

static void ringbuf_push(RingBuffer *rb, double value) {
    rb->values[rb->head] = value;
    rb->head = (rb->head + 1) % rb->size;
    if (rb->count < rb->size) {
        rb->count++;
    }
}

static double ringbuf_average(const RingBuffer *rb) {
    if (rb->count == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < rb->count; i++) {
        sum += rb->values[i];
    }
    return sum / rb->count;
}

static void ringbuf_free(RingBuffer *rb) {
    if (rb) {
        free(rb->values);
        free(rb);
    }
}

/* Signal handler for clean shutdown */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

void monitor_stop(void) {
    running = 0;
}

/*
 * read_iface_stats - Read RX/TX bytes for an interface from /proc/net/dev
 * Returns: 0 on success, -1 on error
 */
static int read_iface_stats(const char *iface, unsigned long long *rx_bytes, unsigned long long *tx_bytes) {
    FILE *fp = fopen(PROC_NET_DEV, "r");
    if (!fp) {
        perror("Cannot open /proc/net/dev");
        return -1;
    }
    
    char line[256];
    int found = 0;
    
    /* Skip first two header lines */
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    
    /* Parse interface lines */
    while (fgets(line, sizeof(line), fp)) {
        char iface_name[64];
        unsigned long long rx, tx;
        unsigned long long dummy;
        
        /* Format: iface: rx_bytes rx_packets ... tx_bytes tx_packets ... */
        int n = sscanf(line, " %63[^:]: %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                       iface_name, &rx, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &tx);
        
        if (n >= 10 && strcmp(iface_name, iface) == 0) {
            *rx_bytes = rx;
            *tx_bytes = tx;
            found = 1;
            break;
        }
    }
    
    fclose(fp);
    
    if (!found) {
        fprintf(stderr, "Interface '%s' not found in /proc/net/dev\n", iface);
        return -1;
    }
    
    return 0;
}

/*
 * get_default_interface - Get first non-loopback interface
 * Returns: 0 on success, -1 on error
 */
static int get_default_interface(char *iface_out, size_t len) {
    FILE *fp = fopen(PROC_NET_DEV, "r");
    if (!fp) {
        return -1;
    }
    
    char line[256];
    
    /* Skip header lines */
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    
    /* Find first non-loopback interface */
    while (fgets(line, sizeof(line), fp)) {
        char iface_name[64];
        if (sscanf(line, " %63[^:]:", iface_name) == 1) {
            if (strcmp(iface_name, "lo") != 0) {
                strncpy(iface_out, iface_name, len - 1);
                iface_out[len - 1] = '\0';
                fclose(fp);
                return 0;
            }
        }
    }
    
    fclose(fp);
    return -1;
}

void monitor_print_header(void) {
    printf("%-10s %15s %15s %15s %15s %15s %15s\n",
           "IFACE", "RX_BYTES", "TX_BYTES", "RX_BPS", "TX_BPS", "RX_AVG_BPS", "TX_AVG_BPS");
    printf("------------------------------------------------------------------------------------------------------\n");
}

void monitor_print_stats(const IfaceStats *stats) {
    printf("%-10s %15llu %15llu %15.2f %15.2f %15.2f %15.2f\n",
           stats->iface,
           stats->rx_bytes,
           stats->tx_bytes,
           stats->rx_rate_bps,
           stats->tx_rate_bps,
           stats->rx_avg_bps,
           stats->tx_avg_bps);
}

/*
 * monitor_run - Start monitoring interface bandwidth
 * Args:
 *   iface: interface name (NULL for auto-detect)
 *   interval_ms: sampling interval in milliseconds
 *   duration_sec: monitoring duration in seconds (0 for infinite)
 * Returns: 0 on success, -1 on error
 */
int monitor_run(const char *iface, int interval_ms, int duration_sec) {
    char iface_name[64];
    
    /* Determine interface to monitor */
    if (iface == NULL) {
        if (get_default_interface(iface_name, sizeof(iface_name)) < 0) {
            fprintf(stderr, "Could not auto-detect interface\n");
            return -1;
        }
        printf("Auto-detected interface: %s\n", iface_name);
    } else {
        strncpy(iface_name, iface, sizeof(iface_name) - 1);
        iface_name[sizeof(iface_name) - 1] = '\0';
    }
    
    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Create ring buffers for rolling averages */
    RingBuffer *rx_ring = ringbuf_create(WINDOW_SIZE);
    RingBuffer *tx_ring = ringbuf_create(WINDOW_SIZE);
    if (!rx_ring || !tx_ring) {
        fprintf(stderr, "Failed to allocate ring buffers\n");
        ringbuf_free(rx_ring);
        ringbuf_free(tx_ring);
        return -1;
    }
    
    /* Initial reading */
    unsigned long long prev_rx, prev_tx, curr_rx, curr_tx;
    if (read_iface_stats(iface_name, &prev_rx, &prev_tx) < 0) {
        ringbuf_free(rx_ring);
        ringbuf_free(tx_ring);
        return -1;
    }
    
    printf("\nMonitoring interface: %s (interval: %d ms)\n", iface_name, interval_ms);
    if (duration_sec > 0) {
        printf("Duration: %d seconds\n", duration_sec);
    } else {
        printf("Duration: infinite (Ctrl+C to stop)\n");
    }
    printf("\n");
    
    monitor_print_header();
    
    /* Start monitoring loop */
    running = 1;
    long start_time = ms_now();
    long end_time = (duration_sec > 0) ? start_time + (duration_sec * 1000) : 0;
    long prev_time = start_time;
    
    while (running) {
        /* Sleep for interval */
        if (ms_sleep(interval_ms) < 0) {
            break;
        }
        
        /* Check duration timeout */
        long curr_time = ms_now();
        if (end_time > 0 && curr_time >= end_time) {
            break;
        }
        
        /* Read current stats */
        if (read_iface_stats(iface_name, &curr_rx, &curr_tx) < 0) {
            continue;
        }
        
        /* Calculate time delta in seconds */
        long time_delta_ms = ms_diff(prev_time, curr_time);
        double time_delta_sec = time_delta_ms / 1000.0;
        
        if (time_delta_sec <= 0) {
            continue;
        }
        
        /* Calculate byte deltas */
        unsigned long long rx_delta = curr_rx - prev_rx;
        unsigned long long tx_delta = curr_tx - prev_tx;
        
        /* Calculate instantaneous rates (bits per second) */
        double rx_rate = (rx_delta * 8.0) / time_delta_sec;
        double tx_rate = (tx_delta * 8.0) / time_delta_sec;
        
        /* Update rolling averages */
        ringbuf_push(rx_ring, rx_rate);
        ringbuf_push(tx_ring, tx_rate);
        
        double rx_avg = ringbuf_average(rx_ring);
        double tx_avg = ringbuf_average(tx_ring);
        
        /* Build stats struct */
        IfaceStats stats;
        strncpy(stats.iface, iface_name, sizeof(stats.iface) - 1);
        stats.iface[sizeof(stats.iface) - 1] = '\0';
        stats.rx_bytes = curr_rx;
        stats.tx_bytes = curr_tx;
        stats.rx_rate_bps = rx_rate;
        stats.tx_rate_bps = tx_rate;
        stats.rx_avg_bps = rx_avg;
        stats.tx_avg_bps = tx_avg;
        
        /* Print stats */
        monitor_print_stats(&stats);
        
        /* Update previous values */
        prev_rx = curr_rx;
        prev_tx = curr_tx;
        prev_time = curr_time;
    }
    
    printf("\nMonitoring stopped.\n");
    
    ringbuf_free(rx_ring);
    ringbuf_free(tx_ring);
    
    return 0;
}