/*
 * Example of Audio Recording Using ADPCM
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
 * 1. Adjust BUFF_SIZE according to MCU RAM size, Size 62000 is for 64K
 * 2. Change AUDIO_FREQ for different sound quality
 * 3. Change I2S standard according to your I2S device: LSB for PT8211, Phillips for MAX98357A
 * 
*/
#include <air32f10x_adc.h>
#include <air32f10x_spi.h>
#include <air32f10x_tim.h>
#include "debug.h"
#include "adpcm.h"

#define AUDIO_FREQ 8000
//#define AUDIO_FREQ 11025
//#define AUDIO_FREQ 16000

#define BUFF_SIZE 60000

RCC_ClocksTypeDef clocks;
uint8_t voice[BUFF_SIZE];
__IO uint8_t finish = 0;

void RCC_Configuration(void);
void TIM_Configuration(void);
void GPIO_Configuration(void);
void IIS_Configuration(void);
void ADC_Configuration(void);
void NVIC_Configuration(void);

void Audio_Encode(void)
{
    static uint32_t idx = 0;
    static uint8_t msb = 0;
    uint8_t val;

    val = ADPCM_Encode((uint16_t)(ADC1->DR << 2)) & 0x0F;
    if (msb == 0)
    {
        voice[idx] = val;
        msb = 1;
    }
    else
    {
        voice[idx] |= (val << 4);
        msb = 0;
        idx++;
        if (idx == BUFF_SIZE)
        {
            // Stop ADC(by stopping TIM3)
            TIM_Cmd(TIM3, DISABLE);
            ADC_Cmd(ADC1, DISABLE);
            ADC_ExternalTrigConvCmd(ADC1, DISABLE);
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
            idx = 0;
            finish = 1;
        }
    }
}

uint16_t Audio_Decode(void)
{
    static uint32_t idx = 0;
    static __IO uint8_t msb = 0, lr = 0;
    static uint16_t val;

    if (msb == 0)
    {
        // Put data to both channels
        if (lr == 0)
        {
            val = ADPCM_Decode(voice[idx] & 0x0F);
            lr = 1;
        }
        else if (lr == 1)
        {
            lr = 0;
            msb = 1;
        }
    }
    else
    {
        if (lr == 0)
        {
            val = ADPCM_Decode((voice[idx] >> 4) & 0x0F);
            lr = 1;
        }
        else if (lr == 1)
        {
            lr = 0;
            msb = 0;
            idx++;
            if (idx == BUFF_SIZE)
            {
                idx = 0;
                ADPCM_Reset();
            }
        }
    }
    return val;
}

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf("\n");
    printf("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);

    RCC_Configuration();
    GPIO_Configuration();
    TIM_Configuration();
    ADC_Configuration();
    NVIC_Configuration();
    IIS_Configuration();
    Delay_Ms(500);
    printf("Start recording\r\n");
    // Turn on LED, DMA TC1 interrupt will turn it off 
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    // Start TIM3 to start recording
    TIM_Cmd(TIM3, ENABLE);
    while(finish == 0);

    printf("Start playing\r\n");
    ADPCM_Reset();
    SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
    while (1);
}

void RCC_Configuration(void)
{
    // Enable periphal clock
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
#elif AUDIO_FREQ == 11025
    // Period = 72,000,000 / 11,025 = 726 * 9
    TIM_TimeBaseStructure.TIM_Prescaler = 726 - 1;
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
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void IIS_Configuration(void)
{
    I2S_InitTypeDef I2S_InitStructure;

    SPI_I2S_DeInit(SPI2);
    I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
    // PT8211:LSB,  MAX98357A:Phillips
    I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;
    // 16-bit data resolution
    I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
#if AUDIO_FREQ == 8000
    // 8K sampling rate
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;
#elif AUDIO_FREQ == 11025
    // 11K sampling rate
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_11k;
#else
    // 16K sampling rate
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_16k;
#endif
    I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
    I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
    I2S_Init(SPI2, &I2S_InitStructure);
    //SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
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
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_7Cycles5);

    // Enable ADC1 external trigger
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);

    // Calibration
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

void NVIC_Configuration(void)
{
    // ADC1 interrupts
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // SPI2 interrupts
    NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
    {
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
        if (finish == 0)
        {
            Audio_Encode();
        }
    }
}

void SPI2_IRQHandler(void)
{
    // If TX Empty flag is set
    if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
    {
        SPI2->DR = Audio_Decode();
    }
}