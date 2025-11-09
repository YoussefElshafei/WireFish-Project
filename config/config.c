/*
 * File: config.c
 * Implements CLI → Config translation and validation.
 *
 * Validation:
 *  - ports_from <= ports_to within [1..65535]
 *  - ttl range sane, interval_ms > 0
 *  - iface non-empty for monitor mode
 */
