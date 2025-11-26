/*icmp.c - ICMP packet building and parsing
 * Summary: ICMP packet construction and parsing utilities.
 * 
 * Author: Shan Truong - 400576105 - truons8
 * Date: December 3, 2025
 * Coures: 2XC3
 */

#include "icmp.h"
#include <string.h>         // memcpy, memset
#include <arpa/inet.h>      // htons()
#include <netinet/ip_icmp.h> // struct icmphdr, ICMP_ECHO
#include <netinet/ip.h>       // for struct iphdr

/**
 * Compute ICMP checksum.
 * @param buf Pointer to ICMP message
 * @param len Length of ICMP message in bytes
 * @return 16-bit checksum
 */
uint16_t icmp_checksum(const void *buf, size_t len) {

    // Interpret buffer as array of 16-bit words
    const uint16_t *data = (const uint16_t *)buf;

    // Initialize sum
    uint32_t sum = 0;

    // Sum 16-bit words
    while(len > 1) {
        sum += *data++;
        len -= 2;
    }

    // If we have a leftover byte, pad it to 16 bits and add
    if(len == 1) {

        //pad last byte with zeros to make 16 bits
        uint16_t last = 0;

        //cast to uint8_t pointer to copy single byte
        *(uint8_t *)&last = *(const uint8_t *)data;

        //add padded last byte to sum
        sum += last;
    }

    // Fold 32-bit sum to 16 bits: keep adding carry into low 16 bits
    while(sum >> 16) {

        // Add high 16 bits to low 16 bits
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // One's complement of the result
    return (uint16_t)(~sum);
}

/**
 * Build ICMP Echo Request packet.
 * @param id Identifier
 * @param seq Sequence number
 * @param payload Pointer to payload data
 * @param payload_len Length of payload data
 * @param out Output buffer for ICMP packet
 * @param out_len Pointer to size of output buffer; updated with actual length
 * @return 0 on success, -1 on error
 */
int icmp_build_echo(uint16_t id, uint16_t seq, const void *payload, size_t payload_len, unsigned char *out, size_t *out_len) {

    //Validate output parameters
    if(out == NULL || out_len == NULL) {
        return -1;
    }

    // Total packet length = header + payload
    size_t total_len = sizeof(struct icmphdr) + payload_len;

    // Point header into the output buffer
    struct icmphdr *hdr = (struct icmphdr *)out;

    // Clear header
    memset(hdr, 0, sizeof(*hdr));

    // Fill ICMP Echo Request fields
    hdr->type = ICMP_ECHO;   
    hdr->code = 0;           
    hdr->un.echo.id = htons(id);
    hdr->un.echo.sequence = htons(seq);

    // Copy payload (if any) right after the header
    if(payload_len > 0 && payload != NULL) {

        //copy payload into output buffer after header
        memcpy(out + sizeof(struct icmphdr), payload, payload_len);
    }

    // Checksum calculated over entire ICMP message (header + payload)
    hdr->checksum = 0; // must be zeroed before computing checksum

    //compute checksum
    hdr->checksum = icmp_checksum(out, total_len);

    // Set output length
    *out_len = total_len;
    return 0;
}

/**
 * Parse ICMP response packet.
 * @param packet Pointer to received packet
 * @param len Length of received packet
 * @param expected_ip Expected source IP address as string (for filtering)
 * @param out_type Pointer to store ICMP type
 * @return 0 on success, -1 on error
 */
int icmp_parse_response(const void *packet, size_t len, const char *expected_ip, int *out_type){

    //currently not using expected_ip, so to avoid unused parameter warning
    (void)expected_ip;

    //Validate parameters
    if(packet == NULL || out_type == NULL){
        return -1;
    }

    // Interpret packet as byte array
    const unsigned char *buf = (const unsigned char *)packet;

    // Need at least an IP header (IPv4)
    if(len < sizeof(struct iphdr)){

        // Not enough data for IP header
        *out_type = -1;
        return -1;
    }

    // Interpret the start of the buffer as an IPv4 header
    const struct iphdr *iph = (const struct iphdr *)buf;

    // ihl = "IP header length" in 32-bit words, so multiply by 4 to get bytes
    int iphdr_len = iph->ihl * 4;

    // Make sure we have enough bytes for IP + ICMP header
    if(len < (size_t)(iphdr_len + sizeof(struct icmphdr))){
        
        // Not enough data for ICMP header
        *out_type = -1;
        return -1;
    }

    // Point to ICMP header after IP header
    const struct icmphdr *icmph = (const struct icmphdr *)(buf + iphdr_len);

    // Return ICMP type
    *out_type = icmph->type;
    return 0;
}