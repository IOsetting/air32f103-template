/*
 * Example of ADC conversion triggerred by TIM3 trigger output
*/
#include <air32f10x_adc.h>
#include <air32f10x_tim.h>
#include "debug.h"

void RCC_Configuration(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    // Set TIM3 period to 1 second (clock at 72MHz)
    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 72000000 / 10000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // Enable TIM3 'TIM update' trigger output
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    /*
     * Don't set timer interrupt and NVIC 
    */
    TIM_Cmd(TIM3, ENABLE);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // PA1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void ADC_Function_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    // Select TIM3 trigger output as external trigger
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable ADC1 external trigger
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);
    // Enable 'End Of Conversion' interrupt
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

void ADC1_2_IRQHandler(void)
{
    uint16_t adc_val;
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
    {
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
        adc_val = ADC_GetConversionValue(ADC1);
        printf("val:%04d\r\n", adc_val);
    }
}

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    RCC_Configuration();
    GPIO_Configuration();
    ADC_Function_Init();
    TIM_Configuration();

    while (1);
}
