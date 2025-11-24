/*
 * File: net.c
 * Implements address resolution and socket helpers
 *
 * This file provides the implementation for all networking operations used by wirefish
 * It handles DNS resolution, TCP connections with timeouts, TTL settings, and raw ICMP sockets
 *
 * Notes:
 *  - Uses getaddrinfo() for DNS resolution (supports IPv4)
 *  - net_tcp_connect supports timeout via non-blocking and select()
 *  - Focuses on IPv4 (IPv6 support can be added later)
 *
 * Aryan Verma, 400575438, McMaster University
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * Provides close() which is used to close sockets
 */
#include <unistd.h>

/*
 * Provides fcntl() which can set socket to non-blocking mode
 * - F_GETFL: get socket flags
 * - F_SETFL: set socket flags
 * - O_NONBLOCK: non-blocking mode flag
 */
#include <fcntl.h>

/*
 * Provides basic system types (required on some systems)
 */
#include <sys/types.h>

/*
 * Provides socket operations (socket(), connect(), setsockopt())
 */
#include <sys/socket.h>

/*
 * Provides internet protocol structures (sockaddr_in, IPPROTO_*)
 */
#include <netinet/in.h>

/*
 * Provides ICMP protocol definitions (IPPROTO_ICMP)
 */
#include <netinet/ip_icmp.h>

/*
 * Library provides getaddrinfo() and related structures
 * - struct addrinfo: hints and results for address resolution
 * - getaddrinfo(): DNS lookup function
 * - freeaddrinfo(): free the results
 * - gai_strerror(): convert getaddrinfo error to string
 */
#include <netdb.h>

/*
 * Library provides inet_ntop() to convert IP addresses to strings (debugging)
 */
#include <arpa/inet.h>

/*
 * Library provides select() function and fd_set structure 
 * select() lets us wait for a socket to become ready with a timeout
 */
#include <sys/select.h>

#include "net.h"

/*
 * Function: net_resolve
 *
 * Resolves a hostname or IP address string into a socket address structure
 *
 * The getaddrinfo() function does DNS lookups
 *
 * How it works:
 *   1. Give it a hostname (ex, "google.com")
 *   2. Give it "hints" about what kind of address you want
 *   3. It returns a linked list of possible addresses
 *   4. We take the first one and copy it
 * 
 * Returns:
 *  - O for success
 *  - -1 for error
 */
int net_resolve(const char *host, struct sockaddr_storage *out, socklen_t *outlen) {
    
     // The hints structure tells getaddrinfo() what kind of address we want
     // Intially set the structs memory block to all zero
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));  
    
    
    // ai_family is the address family (could be IPv4 or IPv6)
    // AF_INET = IPv4 (addresses like 192.168.1.1)
    hints.ai_family = AF_INET; // Setting to IPv4
    
    
    // ai_socktype is the type of socket
    // SOCK_STREAM = TCP (connection-oriented, reliable)
    // SOCK_DGRAM = UDP (connectionless, unreliable but fast)
    hints.ai_socktype = SOCK_STREAM; // Setting to TCP
    
    
    // Getting the address info 
    // host - the hostname or IP string to resolve
    // NULL - the service (port) - we do not need it yet
    // &hints - our preferences
    // &result - where to store the results
    struct addrinfo *result;
    int status = getaddrinfo(host, NULL, &hints, &result);
    
    // Checking for errors
    if (status != 0) {
        // gai_strerror() simply just converts the error code to a human readable string
        fprintf(stderr, "Error: DNS resolution failed for '%s': %s\n", host, gai_strerror(status));
        return -1;
    }

    // getaddrinfo() returns a linked list of addresses
    // For simplicity, we just use the first one
    
    // Copy the address from result->ai_addr to our output buffer
    // We use sockaddr_storage because it's large enough for any address type
    memcpy(out, result->ai_addr, result->ai_addrlen);
    *outlen = result->ai_addrlen;
    
    freeaddrinfo(result);
    
    return 0; 
}

/*
 * Function: net_tcp_connect
 *
 * Creates a TCP connection with a timeout
 *
 * Port States:
 * - OPEN: Server accepts connection (SYN-ACK received)
 * - CLOSED: Server refuses connection (RST received)
 * - FILTERED: No response (firewall blocking, or host down)
 *
 * Uses Non-Blocking
 *   Normal connect() blocks (waits) until connection succeeds or fails
 *   This could take forever if the host is down
 *   Non-blocking mode lets us set our own timeout
 */
int net_tcp_connect(const struct sockaddr *sa, socklen_t slen, int timeout_ms) {

    // socket() creates an endpoint for communication
    // AF_INET is the IPv4 address family
    // SOCK_STREAM is TCP (connection-oriented, reliable stream)
    // Parameter with value 0 lets the system choose the protocol (TCP for SOCK_STREAM)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    // By default, sockets block on operations like connect()
    // We want non-blocking so we can implement our own timeout
    // fcntl() = "file control" - manipulate file descriptor properties
    // Process:
    // 1. Get current flags with F_GETFL
    // 2. Add O_NONBLOCK flag
    // 3. Set new flags with F_SETFL
    
    // Get current socket flags
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL");
        close(sockfd);
        return -1;
    }
    
    // Add non-blocking flag
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL");
        close(sockfd);
        return -1;
    }
    
    // connect() initiates the TCP 3-way handshake
    // In non-blocking mode:
    // connect() returns immediately
    // Returns -1 with errno = EINPROGRESS (connection in progress)
    // We must wait for connection to complete
    int result = connect(sockfd, sa, slen);
    
    // Check immediate connection success (rare but possible for localhost)
    if (result == 0) {
        return sockfd;
    }
    
    // For non-blocking sockets, errno should be EINPROGRESS
    // This means "connection is in progress, check back later"
    if (errno != EINPROGRESS) {
        // Real error 
        close(sockfd);
        return -1;
    }
    

    // Use select() to wait for the socket to become writable
    // socket becomes writable when the connection succeeds (we can write data), or Connection fails (error pending)
    // select() lets us specify a timeout so we do not wait forever
    
    
     // fd_set: a set of file descriptors to monitor
     // Use FD_ZERO and FD_SET to manipulate this set
    fd_set writefds;
    FD_ZERO(&writefds);          // Clear the set
    FD_SET(sockfd, &writefds);   // Add our socket to the set
    

    // struct timeval: represents timeout
    // tv_sec: seconds
    // tv_usec: microseconds (millionths of a second)
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;              // Convert ms to seconds
    tv.tv_usec = (timeout_ms % 1000) * 1000;    // Remainder as microseconds
    
    // select() waits for socket to become ready
    // Parameters:
    // sockfd + 1: highest file descriptor number + 1 (required for select)
    //   NULL: read fds (we don't care about reads)
    //   &writefds: write fds (we care when socket becomes writable)
    //   NULL: exception fds (we don't use this)
    //   &tv: timeout
    //
    // Returns:
    //   > 0: number of ready file descriptors
    //   0: timeout (no sockets became ready)
    //   -1: error
    result = select(sockfd + 1, NULL, &writefds, NULL, &tv);
    
    if (result <= 0) {
        // Timeout or error
        close(sockfd);
        return -1;
    }
    
    // Check if the connection succeeded
    // Must check for pending errors using getsockopt()
    int error = 0;
    socklen_t error_len = sizeof(error);
    
    // getsockopt() gets the socket option
    // SOL_SOCKET: socket level options
    // SO_ERROR: retrieve pending error (if any)
     
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0) {
        // Couldn't get error status
        close(sockfd);
        return -1;
    }
    

    // If error != 0, connection failed
    // Common errors:
    //   ECONNREFUSED: port is closed (server actively refused)
    //   ETIMEDOUT: connection timed out
    //   EHOSTUNREACH: no route to host
    if (error != 0) {
        // Connection failed
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

/*
 * Function: net_set_ttl
 *
 * Sets the Time-To-Live (TTL) value on a socket
 * This controls how many router hops a packet can traverse
 *
 * What is TTL?
 *  - Every IP packet has a TTL field (8 bits, so 0-255)
 *  - Each router that handles the packet decrements TTL by 1
 *  - When TTL reaches 0, the router drops the packet and sends an ICMP "Time Exceeded" message back to the sender
 *
 * Why is TTL important?
 * - Prevents routing loops (packets bouncing forever)
 * - Used by traceroute to discover network paths (find addresses)
 */
int net_set_ttl(int sockfd, int ttl) {

    // setsockopt() is used to set options on a socket

    // sockfd is the socket to modify
    // IPPROTO_IP is IP protocol level
    // IP_TTL is the TTL option
    // &ttl is the pointer to the new TTL value
    // sizeof(ttl) is the size of the value
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt IP_TTL");
        return -1;
    }
    
    return 0;
}

/*
 * Function: net_icmp_raw_socket
 *
 * Creates a raw ICMP socket for sending/receiving ICMP packets
 * This is needed for implementing traceroute
 *
 * TCP/UDP sockets automatically add protocol headers
 * Raw sockets let you manually build the entire packet
 *   
 * Why we need it:
 * - There's no "normal" ICMP socket type (like TCP or UDP)
 * - We need to manually construct ICMP packets for traceroute
 * - We need to receive ICMP responses from routers
 *
 * Requires root permissions since raw sockets can be used for spoofing, sending malicious packets or perform network attacks 
 *
 */
int net_icmp_raw_socket() {
    
    // socket() with raw ICMP protocol
    // AF_INET specifies IPv4
    // SOCK_RAW specifies that it is a raw socket
    // IPPROTO_ICMP refers to the ICMP protocol
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
         // Special error message for permission denied
        if (errno == EPERM) {
            fprintf(stderr, "Error: ICMP raw socket requires root privileges\n");
            fprintf(stderr, "       Run with: sudo ./wirefish --trace ...\n");
        } else {
            perror("socket IPPROTO_ICMP");
        }
        return -1;
    }
    
    return sockfd;
}