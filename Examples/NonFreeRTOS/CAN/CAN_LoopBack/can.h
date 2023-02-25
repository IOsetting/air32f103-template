#ifndef __CAN_H
#define __CAN_H

#include "air32f10x.h"

#define CAN_RX0_INT_ENABLE 0

uint8_t CAN_Mode_Init(uint8_t tsjw, uint8_t tbs2, uint8_t tbs1, uint16_t brp, uint8_t mode);
uint8_t CAN_SendData(uint8_t *msg, uint8_t len);
uint8_t CAN_ReceiveData(uint8_t *buf);
#endif
