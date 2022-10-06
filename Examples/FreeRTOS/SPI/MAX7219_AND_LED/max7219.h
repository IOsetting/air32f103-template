#ifndef __MAX7219_H
#define __MAX7219_H

#include "air32f10x.h"
#include "air32f10x_rcc.h"
#include "air32f10x_gpio.h"
#include "FreeRTOS.h"
#include "task.h"


#define DECODE_MODE  0x09
#define INTENSITY    0x0A
#define SCAN_LIMIT   0x0B
#define SHUT_DOWN    0x0C
#define DISPLAY_TEST 0x0F

void MAX7219_SPI_Init(void);
void MAX7219_Init(void);
void MAX7219_Write(uint8_t addr, uint8_t dat);
void MAX7219_TaskFunction(void *pvParameters);


#endif