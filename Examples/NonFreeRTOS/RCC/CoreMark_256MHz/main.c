#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "air32f10x.h"
#include "air32f10x_tim.h"
#include "misc.h"

#define RCC_PLL 32
#define RCC_PLL_NUM RCC_PLLMul_32 

void RCC_ClkConfiguration(void)
{
    RCC_DeInit(); // Reset RCC

    RCC_HSEConfig(RCC_HSE_ON); // Enable external high speed osc
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); // Wait till HSE ready

    RCC_PLLCmd(DISABLE);    // Turn PLL off
    /*
     * Set PLL, System clock = 8MHz * PLLMul
     */
    AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLL_NUM, 1);

    RCC_PLLCmd(ENABLE);     // Turn PLL on
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); // Wait till PLL ready

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // Select PLL as system clock

    RCC_HCLKConfig(RCC_SYSCLK_Div1);    // AHB clock = sysclk
    RCC_PCLK1Config(RCC_HCLK_Div2);     // APB1 clock = HCLK / 2
    RCC_PCLK2Config(RCC_HCLK_Div1);     // APB2 clock = HCLK
}

void TIM2_Init(void)
{
    // Set TIM2 interrupt on each millisecond, for coremark ticks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 999;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 8 * RCC_PLL - 1;
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
    RCC_ClocksTypeDef clocks;

    RCC_ClkConfiguration(); // Set PLL clock
    Delay_Init();
    USART_Printf_Init(115200);

    printf("AIR32F103 CoreMark\n");
    RCC_GetClocksFreq(&clocks);
    printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
               clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
               clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
    TIM2_Init();
}