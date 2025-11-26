/*tracer.c - Implements traceroute functionality using ICMP Echo requests.
    * Responsibilities:
    *  - Send ICMP Echo requests with increasing TTL to map network path
    * - Record per-hop IP, hostname (reverse DNS), RTT, and timeout status
    * - Return results in TraceRoute struct
    * - Handle raw sockets, timeouts, and ICMP response parsing
 *
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#include "tracer.h"
#include "icmp.h"
#include "../net/net.h"
#include "../model/model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>   // gettimeofday()
#include <arpa/inet.h>  // inet_ntop()
#include <netinet/ip_icmp.h>
#include <netdb.h>   // getnameinfo, NI_MAXHOST

#define ICMP_ID  0x1234   // arbitrary unique ID
#define ICMP_SEQ 1        // sequence number

#define NI_MAXHOST 1025   // value used by GNU libc

/**
 * Get current time in milliseconds.
 * @return Current time in ms since epoch
 */
static long now_ms() {
    
    // gettimeofday returns seconds + microseconds
    struct timeval tv;

    //get current time to determine round trip time (RTT)
    gettimeofday(&tv, NULL);
    
    // Convert to milliseconds
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000);
}

/**
 * Append a Hop to TraceRoute, resizing if needed.
 * @param route Pointer to TraceRoute
 * @param h Pointer to Hop to append
 */
static void tracer_append(TraceRoute *route, const Hop *h) {

    // Resize if needed
    if(route->len == route->cap){

        // Double capacity or start at 16
        size_t newcap = route->cap ? route->cap * 2 : 16;

        // Reallocate memory for rows
        route->rows = realloc(route->rows, newcap * sizeof(Hop));

        // Update capacity
        route->cap = newcap;
    }

    // Append new hop
    route->rows[route->len++] = *h;
}

/**
 * Run traceroute using ICMP Echo requests.
 * @param cfg Pointer to CommandLine config
 * @param out Pointer to TraceRoute to fill
 * @return 0 on success, -1 on error
 */
int tracer_run(const CommandLine *cfg, TraceRoute *out){

    //clear TraceRoute to empty
    memset(out, 0, sizeof(*out));

    //resolving target
    struct sockaddr_storage target_addr;

    //length of target_addr
    socklen_t target_len;

    //casting to struct sockaddr pointer for net_resolve
    struct sockaddr *sa = (struct sockaddr *)&target_addr;

    //resolve target hostname/IP
    if(net_resolve(cfg->target, &target_addr, &target_len) != 0){
        fprintf(stderr, "Error: Failed to resolve target '%s'\n", cfg->target);
        return -1;
    }

    //creating raw ICMP socket
    int sockfd = net_icmp_raw_socket();
    if(sockfd < 0){
        return -1; // error already printed
    }

    //Iterate TTL from cfg->ttl_start → cfg->ttl_max
    for(int ttl = cfg->ttl_start; ttl <= cfg->ttl_max; ttl++){

        //Set socket TTL
        net_set_ttl(sockfd, ttl);

        //Build ICMP Echo Request packet
        unsigned char pkt[64];
        size_t pktlen = 0;

        //build ICMP packet
        if(icmp_build_echo(ICMP_ID, ttl, NULL, 0, pkt, &pktlen) < 0){

            fprintf(stderr, "Error: ICMP packet build failed\n");

            //clean up socket
            close(sockfd);
            return -1;
        }

        // Record start time
        long tstart = now_ms();

        //Send ICMP Echo Request
        if(sendto(sockfd, pkt, pktlen, 0, sa, target_len) < 0){

            fprintf(stderr, "sendto failed:\n");

            //clean up socket
            close(sockfd);
            return -1;
        }

        // prepare buffer to receive response
        char recvbuf[512];

        //prepare to receive from
        struct sockaddr_in reply_addr;
        socklen_t reply_len = sizeof(reply_addr);

        //Set timeout (1 second)
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        //Use select() to wait for response or timeout
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);

        //getting result of select which is either 0 (timeout) or >0 (data available)
        int sel = select(sockfd + 1, &fds, NULL, NULL, &tv);

        //initialize Hop
        Hop h;

        //clear Hop
        memset(&h, 0, sizeof(h));

        //set hop number
        h.hop = ttl;

        //Check select result
        if(sel == 0){

            // timeout
            h.timeout = true;

            //set unknown IP and host for timeout
            strcpy(h.ip, "*");

            //set unknown host for timeout
            strcpy(h.host, "?");

            //set RTT to -1 for timeout
            h.rtt_ms = -1;

            //set icmp_type to -1 for timeout (marked as unknown)
            h.icmp_type = -1;

            //append hop to TraceRoute
            tracer_append(out, &h);
            continue;
        }

        //recvfrom() for actual ICMP response
        int n = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&reply_addr, &reply_len);

        //Record end time
        long tend = now_ms();

        //Check recvfrom result
        if(n < 0){

            // treat as timeout
            h.timeout = true;

            //set unknown IP and host for timeout
            strcpy(h.ip, "*");

            //set unknown host for timeout
            strcpy(h.host, "?");

            //set RTT to -1 for timeout
            h.rtt_ms = -1;

            //set icmp_type to -1 for timeout (marked as unknown)
            tracer_append(out, &h);
            continue;
        }

        //Extract IP of hop
        inet_ntop(AF_INET, &reply_addr.sin_addr, h.ip, sizeof(h.ip));

        //Parse ICMP type
        int icmp_type = 0;
        icmp_parse_response(recvbuf, n, h.ip, &icmp_type);

        //Fill Hop details
        h.timeout = false;

        //Calculate RTT
        h.rtt_ms = (int)(tend - tstart);

        //set ICMP type
        h.icmp_type = icmp_type;

        //Resolve hostname (reverse DNS lookup)
        char hostbuf[NI_MAXHOST];

        //taking ip address from reply_addr and getting hostname
        int gi = getnameinfo((struct sockaddr *)&reply_addr, reply_len, hostbuf, sizeof(hostbuf), NULL, 0, 0);

        //check getnameinfo result
        if(gi == 0){

            // Successfully resolved a hostname
            strncpy(h.host, hostbuf, sizeof(h.host) - 1);
            h.host[sizeof(h.host) - 1] = '\0';
        }
        
        else{

            // Fallback: just use the IP string
            strncpy(h.host, h.ip, sizeof(h.host) - 1);
            h.host[sizeof(h.host) - 1] = '\0';
        }

        //Append Hop to TraceRoute
        tracer_append(out, &h);

        //If we reached destination, stop
        if(icmp_type == ICMP_ECHOREPLY){
            break;
        }
    }

    //Clean up socket
    close(sockfd);
    return 0;
}

/**
 * Free resources in TraceRoute.
 * @param t Pointer to TraceRoute to free
 */
void traceroute_free(TraceRoute *t){

    //Check for NULL
    if(t == NULL){
        return;
    }

    //Free allocated rows
    free(t->rows);

    //Reset TraceRoute
    t->rows = NULL;
    t->len = 0;
    t->cap = 0;
}
