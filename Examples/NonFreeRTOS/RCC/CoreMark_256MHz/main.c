/**
 * CoreMark Benchmark
 * 
 * -- Change system clock by changing SYSCLK_HSE in air32f10x_conf.h
 * 
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "air32f10x.h"
#include "air32f10x_tim.h"
#include "misc.h"

RCC_ClocksTypeDef clocks;

void TIM2_Init(void)
{
    uint16_t pll = (uint16_t)(clocks.SYSCLK_Frequency / (uint32_t)8000000);
    // Set TIM2 interrupt on each millisecond, for coremark ticks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 999;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 8 * pll - 1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
}

void main_original(void)
{
    Delay_Init();
    USART_Printf_Init(115200);

    printf("AIR32F103 CoreMark\n");
    RCC_GetClocksFreq(&clocks);
    printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
               clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
               clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
    TIM2_Init();
}