#ifndef __LED_H
#define __LED_H

#include "air32f10x.h"
#include "air32f10x_rcc.h"
#include "air32f10x_gpio.h"
#include "FreeRTOS.h"
#include "task.h"


#define GPIO_GROUP_TEST GPIOB
#define GPIO_MODE_TEST GPIO_Mode_Out_PP
#define GPIO_SPEED_TEST GPIO_Speed_50MHz
#define GPIO_PIN1_TEST GPIO_Pin_2
#define GPIO_PIN2_TEST GPIO_Pin_10
#define GPIO_PIN3_TEST GPIO_Pin_11

void LED_Configuration(void);
void LED_TaskFunction(void *pvParameters);


#endif