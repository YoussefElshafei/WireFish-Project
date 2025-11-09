/*
 * File: net.h
 * Summary: Networking helpers (resolve, connect with timeout, TTL, raw).
 *
 * Public API:
 *  - int net_resolve(const char *host, struct sockaddr_storage *out, socklen_t *outlen);
 *  - int net_tcp_connect(const struct sockaddr *sa, socklen_t slen, int timeout_ms);
 *  - int net_set_ttl(int sockfd, int ttl);
 *  - int net_icmp_raw_socket(void);
 */
#ifndef NET_H
#define NET_H

#include <sys/socket.h>
#include <netinet/in.h>

int net_resolve(const char *host, struct sockaddr_storage *out, socklen_t *outlen);
int net_tcp_connect(const struct sockaddr *sa, socklen_t slen, int timeout_ms);
int net_set_ttl(int sockfd, int ttl);
int net_icmp_raw_socket(void);

#endif /* NET_H */
