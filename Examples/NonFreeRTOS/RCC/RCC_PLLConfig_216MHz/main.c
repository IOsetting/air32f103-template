#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "air32f10x.h"

void RCC_ClkConfiguration(void)
{
    RCC_DeInit(); // Reset RCC

    RCC_HSEConfig(RCC_HSE_ON); // Enable external high speed osc
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); // Wait till HSE ready

    RCC_PLLCmd(DISABLE);    // Turn PLL off
    /*
     * Set PLL, System clock = 8MHz * PLLMul
     */
    AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_27, 1);

    RCC_PLLCmd(ENABLE);     // Turn PLL on
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); // Wait till PLL ready

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); // Select PLL as system clock

    RCC_HCLKConfig(RCC_SYSCLK_Div1);    // AHB clock = sysclk
    RCC_PCLK1Config(RCC_HCLK_Div2);     // APB1 clock = HCLK / 2
    RCC_PCLK2Config(RCC_HCLK_Div1);     // APB2 clock = HCLK
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}


int main(void)
{
    RCC_ClocksTypeDef clocks;

    RCC_ClkConfiguration();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("AIR32F103 RCC PLL Clock Config.\n");

    RCC_GetClocksFreq(&clocks);
    printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
               clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
               clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
    GPIO_Configuration();

    while (1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_2);
        Delay_Ms(200);
        GPIO_SetBits(GPIOB, GPIO_Pin_10);
        Delay_Ms(200);
        GPIO_SetBits(GPIOB, GPIO_Pin_11);
        Delay_Ms(200);
        GPIO_ResetBits(GPIOB, GPIO_Pin_2);
        Delay_Ms(200);
        GPIO_ResetBits(GPIOB, GPIO_Pin_10);
        Delay_Ms(200);
        GPIO_ResetBits(GPIOB, GPIO_Pin_11);
        Delay_Ms(200);
    }
}
