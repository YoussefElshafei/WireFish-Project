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
 */
#ifndef ICMP_H
#define ICMP_H

#include <stddef.h>
#include <stdint.h>

uint16_t icmp_checksum(const void *buf, size_t len);
int icmp_build_echo(uint16_t id, uint16_t seq,
                    const void *payload, size_t payload_len,
                    unsigned char *out, size_t *out_len);

#endif /* ICMP_H */
