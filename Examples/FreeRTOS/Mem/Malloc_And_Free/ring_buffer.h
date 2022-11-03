#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <stdint.h>

// Max string length would be RING_BUFFER_SIZE - 2
#define RING_BUFFER_SIZE 30

void ring_buffer_reset(void);
void ring_buffer_push(uint8_t c);
uint8_t ring_buffer_pop(void);
uint16_t ring_buffer_size(void);
uint16_t ring_buffer_read(uint8_t *buf);


#endif /* __RING_BUFFER_H */
