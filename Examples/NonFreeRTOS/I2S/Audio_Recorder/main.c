/*
 * Example of Audio Recording
 * 
 * Wiring:
 * 
 *   AIR32F103                  MAX98357A / PT8211
 *   PB13(SPI1_SCK/I2S_CK)       -> BCLK, BCK
 *   PB15(SPI1_MOSI/I2S_SD)      -> DIN
 *   PB12(SPI1_NSS/I2S_WS)       -> LRC, WS
 *                               GND  -> GND
 *                               VIN  -> 3.3V
 *                               +    -> speaker
 *                               -    -> speaker
 * 
 *   AIR32F103                  MAX9814
 *   PA2                        -> Out
 *   3.3V                       -> VDD
 *   GND                        -> GND
 *   GND                        -> A/R
 *                                 GAIN -> float:60dB, gnd:50dB, 3.3v:40dB
 * 
 * Change AUDIO_FREQ to test the sound quality in different audio frequencies
 * 
*/
#include <air32f10x_adc.h>
#include <air32f10x_dma.h>
#include <air32f10x_spi.h>
#include <air32f10x_tim.h>
#include "debug.h"

#define AUDIO_FREQ 8000
//#define AUDIO_FREQ 11000
//#define AUDIO_FREQ 16000

#define BUFF_SIZE 30000

uint16_t dma_buf[BUFF_SIZE];
uint32_t TxIdx;
__IO uint8_t lr = 0;

void RCC_Configuration(void)
{
    // Enable periphal clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // ADC clock =  PCLK2 / 8
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 9 - 1;
#if AUDIO_FREQ == 8000
    // Period = 72,000,000 / 8,000 = 1000 * 9
    TIM_TimeBaseStructure.TIM_Prescaler = 1000 - 1;
#elif AUDIO_FREQ == 11000
    // Period = 72,000,000 / 11,000 = 727 * 9
    TIM_TimeBaseStructure.TIM_Prescaler = 727 - 1;
#else
    // Period = 72,000,000 / 16,000 = 500 * 9
    TIM_TimeBaseStructure.TIM_Prescaler = 500 - 1;
#endif
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // Enable TIM3 'TIM update' trigger for adc
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    // Timer will be started in main()
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // PA2 as analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // PB12,PB13,PB15 as I2S AF output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // PC13 as GPIO output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void IIS_Configuration(void)
{
    I2S_InitTypeDef I2S_InitStructure;

    SPI_I2S_DeInit(SPI2);
    I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
    I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
    // 16-bit data resolution
    I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
#if AUDIO_FREQ == 8000
    // 8K sampling rate
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;
#elif AUDIO_FREQ == 11000
    // 11K sampling rate
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_11k;
#else
    // 16K sampling rate
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_16k;
#endif
    I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
    I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
    I2S_Init(SPI2, &I2S_InitStructure);

    SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
    I2S_Cmd(SPI2, ENABLE);
}

void ADC_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    // Reset ADC1
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    // Select TIM3 trigger output as external trigger
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    // ADC_Channel_2 for PA2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);

    // Enable ADC1 external trigger
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    // Enable ADC1 with DMA
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    // Calibration
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

void DMA_Configuration(DMA_Channel_TypeDef *DMA_CHx, uint32_t ppadr, uint32_t memadr, uint16_t bufsize)
{
    DMA_InitTypeDef DMA_InitStructure;

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
    // DMA1 interrupts
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // SPI2 interrupts
    NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void DMA1_Channel1_IRQHandler(void)
{
    // DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL1);
        // Stop ADC(by stopping TIM3)
        TIM_Cmd(TIM3, DISABLE);
        ADC_Cmd(ADC1, DISABLE);
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
}

void SPI2_IRQHandler(void)
{
    // If TX Empty flag is set
    if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
    {
        // Put data to both channels
        if (lr == 0)
        {
            lr = 1;
            SPI_I2S_SendData(SPI2, (uint16_t)dma_buf[TxIdx] << 4);
        }
        else
        {
            lr = 0;
            SPI_I2S_SendData(SPI2, (uint16_t)dma_buf[TxIdx++] << 4);
            if (TxIdx == BUFF_SIZE)
            {
                TxIdx = 0;
                // Disable the I2S1 TXE Interrupt to stop playing
                SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
            }
        }
    }
}

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    RCC_Configuration();
    GPIO_Configuration();
    ADC_Configuration();
    DMA_Configuration(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)dma_buf, BUFF_SIZE);
    NVIC_Configuration();
    TIM_Configuration();
    IIS_Configuration();
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    Delay_S(1);
    // Start timer to start recording
    printf("Start recording\r\n");
    TIM_Cmd(TIM3, ENABLE);
    // Turn on LED, DMA TC1 interrupt will turn it off 
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    Delay_S(4);
    printf("Start playing\r\n");
    while (1)
    {
        // Restart the playing
        SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
        Delay_S(5);
    }

}
