/*
 * File: net.c
 * Implements address resolution and socket helpers
 *
 * This file provides the implementation for all networking operations used by wirefish
 * It handles DNS resolution, TCP connections with timeouts, TTL settings, and raw ICMP sockets
 *
 * Notes:
 *  - Uses getaddrinfo() for DNS resolution (supports IPv4)
 *  - net_tcp_connect supports timeout via non-blocking + select()
 *  - Focuses on IPv4 (IPv6 support can be added later)
 *
 * Aryan Verma, 400575438, McMaster University
 */


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
 * Library provides inet_ntop() to convert IP addresses to strings (debugging)
 */
#include <arpa/inet.h>

/*
 * Library provides getaddrinfo() and related structures
 * - struct addrinfo: hints and results for address resolution
 * - getaddrinfo(): DNS lookup function
 * - freeaddrinfo(): free the results
 * - gai_strerror(): convert getaddrinfo error to string
 */
#include <netdb.h>

/*
 * Library provides select() function and fd_set structure 
 * select() lets us wait for a socket to become ready with a timeout
 */
#include <sys/select.h>

#include "net.h"



