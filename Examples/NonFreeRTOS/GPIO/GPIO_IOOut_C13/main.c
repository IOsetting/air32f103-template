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

int main(void)
{
    RCC_ClocksTypeDef clocks;
    Delay_Init();
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf("\n");
    printf("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);
    printf("AIR32F103 GPIO Output Test\n");
    GPIO_Configuration();

    while (1)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        Delay_Ms(500);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        Delay_Ms(500);
    }
}
