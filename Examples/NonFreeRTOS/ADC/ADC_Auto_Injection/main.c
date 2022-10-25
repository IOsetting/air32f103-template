/*
 * @Note Auto Injection Conversion, channel2(PA2) and injected channel1(PA3)
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
    // PA2, PA3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    /*
     * Note: different from STM32F1, need to enable ADC_ScanConvMode to make auto injection work
     */
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    /* ADC1 regular channel2 configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);

    /* Set injected sequencer length */
    ADC_InjectedSequencerLengthConfig(ADC1, 1);
    /* ADC1 injected channel Configuration */
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_239Cycles5);
    /* ADC1 injected external trigger configuration */
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);
    /* Enable automatic injected conversion start after regular one */
    ADC_AutoInjectedConvCmd(ADC1, ENABLE);
    /* Enable JEOC interrupt */
    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);

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
        // Get last regular conversion result
        adc_val = ADC_GetConversionValue(ADC1);
        printf("val:%04d jval:", adc_val);
        adc_val = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        printf("%04d ", adc_val);
        printf("\r\n");
        Delay_Ms(500);
    }
}
