#ifndef __CAN_H
#define __CAN_H

#include "air32f10x.h"

#define CAN_RX0_INT_ENABLE 0

uint8_t CAN_Mode_Init(uint8_t tsjw, uint8_t tbs2, uint8_t tbs1, uint16_t brp, uint8_t mode);
void CAN_Filter_Config(uint16_t filterIdHigh, uint16_t filterIdLow, uint16_t filterMaskHigh, uint16_t filterMaskLow);
uint8_t CAN_SendData(uint16_t stdId, uint16_t extId, uint8_t *msg, uint8_t len);
uint8_t CAN_ReceiveData(uint8_t *buf);
#endif
