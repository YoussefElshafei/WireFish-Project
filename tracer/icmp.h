/*
 * File: icmp.h
 * Summary: ICMP structures and helpers (checksum, echo build).
 *
 * Responsibilities:
 *  - Build ICMP Echo packets
 *  - Compute checksum
 *
 * Public API:
 *  - uint16_t icmp_checksum(const void *buf, size_t len);
 *  - int icmp_build_echo(uint16_t id, uint16_t seq,
 *                        const void *payload, size_t payload_len,
 *                        unsigned char *out, size_t *out_len);
 *
 * Notes:
 *  - Wire format must match platform endianness requirements
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#ifndef ICMP_H
#define ICMP_H

#include <stddef.h>
#include <stdint.h>

uint16_t icmp_checksum(const void *buf, size_t len);
int icmp_build_echo(uint16_t id, uint16_t seq, const void *payload, size_t payload_len, unsigned char *out, size_t *out_len);
int icmp_parse_response(const void *packet, size_t len, const char *expected_ip, int *out_type);

#endif /* ICMP_H */
