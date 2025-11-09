/*
 * File: ringbuf.h
 * Summary: Rolling window buffer for smoothing rates/RTT.
 *
 * Public API:
 *  - int  ring_init(RingBuf *rb, size_t cap);
 *  - void ring_push(RingBuf *rb, double v);
 *  - double ring_mean(const RingBuf *rb);
 *  - void ring_free(RingBuf *rb);
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stddef.h>

typedef struct {
  double *data;
  size_t len, cap;
  size_t head;
} RingBuf;

int  ring_init(RingBuf *rb, size_t cap);
void ring_push(RingBuf *rb, double v);
double ring_mean(const RingBuf *rb);
void ring_free(RingBuf *rb);

#endif /* RINGBUF_H */
