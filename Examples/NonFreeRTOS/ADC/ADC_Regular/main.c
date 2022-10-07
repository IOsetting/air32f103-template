/*
 * @Note Regular conversion, ch1(PA1)
 *
 * Regarding multiple channel scan: Unlike F4 series, ADC in F1 series can generate 
 * only one (single) interrupt at the end of the whole scanning sequence. That's 
 * why DMA is a must when using scanning mode.
 * If you want to configure ADC for interrupt based scanning, you have to do this manually:
 * 1. Configure ADC for single conversion, triggered manually or by interrupts.
 * 2. Select the ADC channel
 * 3. In interrupt service routine (ISR), collect the result from ADC->DR.
 * 4. In ISR, Repeat step 2 to configure ADC for the next channel.
*/
#include <air32f10x_adc.h>
#include "debug.h"


void ADC_Function_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    // PA1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

int main(void)
{
    uint16_t adc_val;

    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    ADC_Function_Init();

    while (1)
    {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        // Get last conversion value
        adc_val = ADC_GetConversionValue(ADC1);
        printf("val:%04d\r\n", adc_val);
        Delay_Ms(500);
    }
}
