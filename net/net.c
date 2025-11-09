/*
 * File: net.c
 * Implements address resolution and socket helpers.
 *
 * Notes:
 *  - Uses getaddrinfo(); supports IPv4 first (extend to IPv6 later)
 *  - net_tcp_connect supports timeout via non-blocking + select()
 */
