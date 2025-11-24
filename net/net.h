/*
 * File: net.h
 * Contains networking helpers (resolve, connect with timeout, TTL, raw)
 *
 * This module provides low-level networking utilities that are used by the scanner and tracer modules
 * It wraps complex socket operations into simple, easy-to-use functions
 * 
 * Function prototypes:
 *  - int net_resolve(const char *host, struct sockaddr_storage *out, socklen_t *outlen)
 *  - int net_tcp_connect(const struct sockaddr *sa, socklen_t slen, int timeout_ms)
 *  - int net_set_ttl(int sockfd, int ttl)
 *  - int net_icmp_raw_socket()
 * 
 * Aryan Verma, 400575438, McMaster University
 */

#include <sys/socket.h>
#include <netinet/in.h>

int net_resolve(const char *host, struct sockaddr_storage *out, socklen_t *outlen);
int net_tcp_connect(const struct sockaddr *sa, socklen_t slen, int timeout_ms);
int net_set_ttl(int sockfd, int ttl);
int net_icmp_raw_socket();
