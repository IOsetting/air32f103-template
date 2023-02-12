#include "air32f10x_gpio.h"

#ifndef __ENC28J60_UIP_H
#define __ENC28J60_UIP_H

void etherdev_init(void);
void etherdev_send(u8 *p_char, u16 length);
u16 etherdev_read(u8 *p_char);
u16 etherdev_poll(void);
void tcp_server_appcall(void);

#endif
