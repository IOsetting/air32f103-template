/*
 * @Note Regular conversion, ch1(PA1), start by software and read by interrupt handler
*/
#include <air32f10x_adc.h>
#include "debug.h"

void RCC_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // PA1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void ADC_Function_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

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
    NVIC_Configuration();
    ADC_Function_Init();

    while (1)
    {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        Delay_Ms(500);
    }
}
