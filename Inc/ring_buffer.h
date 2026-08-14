#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

#define RING_BUFFER_CAPACITY 8U

typedef struct {
    uint32_t sequence;
    uint32_t produced_at_ms;
} ring_item_t;

typedef struct {
    ring_item_t storage[RING_BUFFER_CAPACITY];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t high_watermark;
    uint32_t pushes;
    uint32_t pops;
    uint32_t overflows;
    uint32_t underflows;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *buffer);
uint8_t ring_buffer_push(ring_buffer_t *buffer, ring_item_t item);
uint8_t ring_buffer_pop(ring_buffer_t *buffer, ring_item_t *item);
uint8_t ring_buffer_is_empty(const ring_buffer_t *buffer);
uint8_t ring_buffer_is_full(const ring_buffer_t *buffer);

#endif
