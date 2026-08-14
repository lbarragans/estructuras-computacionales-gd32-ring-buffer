#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t *buffer)
{
    uint32_t index;

    for (index = 0U; index < RING_BUFFER_CAPACITY; index++) {
        buffer->storage[index].sequence = 0U;
        buffer->storage[index].produced_at_ms = 0U;
    }

    buffer->head = 0U;
    buffer->tail = 0U;
    buffer->count = 0U;
    buffer->high_watermark = 0U;
    buffer->pushes = 0U;
    buffer->pops = 0U;
    buffer->overflows = 0U;
    buffer->underflows = 0U;
}

uint8_t ring_buffer_push(ring_buffer_t *buffer, ring_item_t item)
{
    if (ring_buffer_is_full(buffer) != 0U) {
        buffer->overflows++;
        return 0U;
    }

    buffer->storage[buffer->head] = item;
    buffer->head = (buffer->head + 1U) % RING_BUFFER_CAPACITY;
    buffer->count++;
    buffer->pushes++;

    if (buffer->count > buffer->high_watermark) {
        buffer->high_watermark = buffer->count;
    }

    return 1U;
}

uint8_t ring_buffer_pop(ring_buffer_t *buffer, ring_item_t *item)
{
    if (ring_buffer_is_empty(buffer) != 0U) {
        buffer->underflows++;
        return 0U;
    }

    *item = buffer->storage[buffer->tail];
    buffer->tail = (buffer->tail + 1U) % RING_BUFFER_CAPACITY;
    buffer->count--;
    buffer->pops++;

    return 1U;
}

uint8_t ring_buffer_is_empty(const ring_buffer_t *buffer)
{
    return (buffer->count == 0U) ? 1U : 0U;
}

uint8_t ring_buffer_is_full(const ring_buffer_t *buffer)
{
    return (buffer->count >= RING_BUFFER_CAPACITY) ? 1U : 0U;
}
