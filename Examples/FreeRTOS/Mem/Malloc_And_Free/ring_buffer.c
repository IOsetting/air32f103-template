#include "ring_buffer.h"

uint8_t ring_buffer[RING_BUFFER_SIZE];
static volatile uint16_t from, to;

void ring_buffer_reset(void)
{
    from = 0;
    to = 0;
}

void ring_buffer_push(uint8_t c)
{
    ring_buffer[to++] = c;
    if (to >= RING_BUFFER_SIZE)
    {
        to = 0;
    }
    if (to == from)
    {
        from++;
        if (from >= RING_BUFFER_SIZE)
        {
            from = 0;
        }
    }
}

uint8_t ring_buffer_pop(void)
{
    to = (to > 0)? to - 1 : RING_BUFFER_SIZE - 1;
    return ring_buffer[to];
}

uint16_t ring_buffer_size(void)
{
    if (to >= from)
    {
        return to - from;
    }
    else
    {
        return RING_BUFFER_SIZE - from + to;
    }
}

uint16_t ring_buffer_read(uint8_t *buf)
{
    uint16_t pos;
    if (to >= from)
    {
        pos = from;
        while(pos < to)
        {
            *buf++ = ring_buffer[pos++];
        }
        return to - from;
    }
    else
    {
        pos = from;
        while(pos < RING_BUFFER_SIZE)
        {
            *buf++ = ring_buffer[pos++];
        }
        pos = 0;
        while(pos < to)
        {
            *buf++ = ring_buffer[pos++];
        }
        return RING_BUFFER_SIZE - from + to;
    }
}