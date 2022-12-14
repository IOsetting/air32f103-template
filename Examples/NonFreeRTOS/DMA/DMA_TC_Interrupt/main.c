/**
 * Example of DMA transfer complete interrupt in circular mode
*/
#include <air32f10x_adc.h>
#include <air32f10x_dma.h>
#include "debug.h"

#define BUFF_SIZE 8000
uint16_t dma_buf[BUFF_SIZE];

void ADC_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    // Enable periphal clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    // ADC clock =  PCLK2 / 36
    RCC_ADCCLKConfig(RCC_PCLK2_Div36);
    // GPIO: A2 as analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // Reset ADC1
    ADC_DeInit(ADC1);
    // Independent mode, continuous
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);

    // Enable ADC1 with DMA
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    // Calibration
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

/*******************************************************************************
* Function Name  : DMA_Tx_Init
* Description    : Initializes the DMA Channelx configuration.
* Input          : DMA_CHx:
*                    x can be 1 to 7.
*                  ppadr: Peripheral base address.
*                  memadr: Memory base address.
*                  bufsize: DMA channel buffer size.
* Return         : None
*******************************************************************************/
void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, uint32_t ppadr, uint32_t memadr, uint16_t bufsize)
{
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA_CHx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    // Addresss increase - peripheral:no, memory:yes
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    // Data unit size: 16bit
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    // Memory to memory: no
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
    // Enable 'Transfer complete' interrupt
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    // Enable DMA
    DMA_Cmd(DMA_CHx, ENABLE);
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    ADC_Configuration();
    /*
     * Only ADC1 and ADC3 have DMA capability
     *   ADC1 -> DMA1_Channel1
     *   ADC3 -> DMA2_Channel5
    */
    DMA_Tx_Init(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)dma_buf, BUFF_SIZE);

    NVIC_Configuration();

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while (1);
}

void DMA1_Channel1_IRQHandler(void)
{
    //Test on DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        // Write to DMA1_IT_GL1 will clear all 4 interrupt pending bits: GL, TC, TH and TE
        DMA_ClearITPendingBit(DMA1_IT_GL1);
        printf("%d %d\r\n", dma_buf[BUFF_SIZE - 2], dma_buf[BUFF_SIZE - 1]);
    }
}