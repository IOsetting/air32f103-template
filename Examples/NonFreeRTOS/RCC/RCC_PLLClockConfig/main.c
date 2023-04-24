#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "debug.h"

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    // PC13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * Change system clock
*/
void RCC_ClkConfiguration(uint32_t RCC_PLLMul)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

    RCC_PLLCmd(DISABLE);
    AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul, 1);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    SystemCoreClockUpdate();
}

int main(void)
{
    RCC_ClocksTypeDef clocks;
    Delay_Init();
    RCC_GetClocksFreq(&clocks);
    GPIO_Configuration();

    USART_Printf_Init(115200);
    printf("\nSYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);
    printf("AIR32F103 Clock Config Test\n");
    Delay_Ms(1000);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(1000);

    printf("SystemClk: switch to 96MHz\n");
    RCC_ClkConfiguration(RCC_PLLMul_12);
    Delay_Init();
    USART_Cmd(USART1, DISABLE);
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);

    printf("SystemClk: switch to 128MHz\n");
    RCC_ClkConfiguration(RCC_PLLMul_16);
    Delay_Init();
    USART_Cmd(USART1, DISABLE);
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);

    /** Fast PPL is enabled by setting PLLx17 */

    printf("SystemClk: switch to 136MHz\n");
    RCC_ClkConfiguration(RCC_PLLMul_17);
    Delay_Init();
    USART_Cmd(USART1, DISABLE);
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);

    printf("SystemClk: switch to 216MHz\n");
    RCC_ClkConfiguration(RCC_PLLMul_27);
    Delay_Init();
    USART_Cmd(USART1, DISABLE);
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);

    printf("SystemClk: switch to 256MHz\n");
    RCC_ClkConfiguration(RCC_PLLMul_32);
    Delay_Init();
    USART_Cmd(USART1, DISABLE);
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    Delay_Ms(500);

    while (1)
    {
        printf("SystemClk:%ld\r\n", SystemCoreClock);
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        Delay_Ms(500);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        Delay_Ms(500);
    }
}
