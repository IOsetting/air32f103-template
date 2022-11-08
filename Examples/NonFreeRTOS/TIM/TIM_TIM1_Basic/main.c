/**
 * Example of TIM1 Update Interrupt
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

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    // Set counter limit to 10000
    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;
    /**
     * Clock source of TIM1,8: if(APB2 prescaler =1) then PCLK2 x1, else PCLK2 x2
     * */
    if (clocks.HCLK_Frequency == clocks.PCLK2_Frequency)
    {
        // clock source is PCLK2 x1.
        TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK2_Frequency / 10000 - 1;
    }
    else
    {
        // clock source is PCLK2 x2
        TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK2_Frequency * 2 / 10000 - 1;
    }
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    // Enable interrupt from 'TIM update'
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

    // NVIC config
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM1, ENABLE);
}

void TIM1_UP_IRQHandler(void)
{
    printf(".");
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update); // Clear INT flag
        printf("%s\n", __FUNCTION__);
    }
}
