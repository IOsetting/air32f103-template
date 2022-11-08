/**
 * Example of TIM3 Interrupt
*/
#include <inttypes.h>
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include <air32f10x_tim.h>
#include <misc.h>
#include "debug.h"

RCC_ClocksTypeDef clocks;

void TIM_Configuration(void);

int main(void)
{
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf(
        "SYSCLK: %3.1fMHz, HCLK: %3.1fMHz, PCLK1: %3.1fMHz, PCLK2: %3.1fMHz, ADCCLK: %3.1fMHz\n", 
    (float)clocks.SYSCLK_Frequency/1000000, (float)clocks.HCLK_Frequency/1000000, 
    (float)clocks.PCLK1_Frequency/1000000, (float)clocks.PCLK2_Frequency/1000000,
    (float)clocks.ADCCLK_Frequency/1000000);

    TIM_Configuration();

    while(1);
}

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // Set counter limit to 10000
    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;
    /**
     * Clock source of TIM2,3,4,5,6,7: if(APB1 prescaler =1) then PCLK1 x1, else PCLK1 x2
     * */
    if (clocks.HCLK_Frequency == clocks.PCLK1_Frequency)
    {
        // clock source is PCLK1 x1.
        // Note: TIM_Prescaler is 16bit, [0, 65535], given PCLK1 is 36MHz, divider should > 550
        TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency / 10000 - 1;
    }
    else
    {
        // clock source is PCLK1 x2
        TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency * 2 / 10000 - 1;
    }
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // Enable interrupt from 'TIM update'
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // NVIC config
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // Clear INT flag
        printf("%s\n", __FUNCTION__);
    }
}
