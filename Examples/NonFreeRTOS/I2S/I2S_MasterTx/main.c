/*
 * @Note Example of I2S Audio TX
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
 * Change I2S standard according to your I2S device: LSB for PT8211, Phillips for MAX98357A
*/
#include "air32f10x_spi.h"
#include "air32f10x_dma.h"
#include "debug.h"
#include "voice.h"

#define DATA_SIZE 27451

uint32_t idx;
__IO uint8_t lr = 0;

void IIS_Configuration(void)
{
    I2S_InitTypeDef I2S_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    // PB12,PB13,PB15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    // PC13
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	SPI_I2S_DeInit(SPI2);
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
    // PT8211:LSB,  MAX98357A:Phillips
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
	I2S_Init(SPI2, &I2S_InitStructure);

	I2S_Cmd(SPI2, ENABLE);
}

void SPI2_IRQHandler(void)
{
    // If TX Empty flag is set
    if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_TXE) == SET)
    {
        // Use lr to put all data to one channel
        if (lr == 0)
        {
            lr = 1;
            SPI_I2S_SendData(SPI2, (uint16_t)voice_bulk[idx] << 6);
        }
        else
        {
            lr = 0;
            SPI_I2S_SendData(SPI2, (uint16_t)voice_bulk[idx++] << 6);
            if (idx == DATA_SIZE)
            {
                idx = 0;
                // Disable the I2S1 TXE Interrupt to stop playing
                SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
            }
        }
    }
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

    IIS_Configuration();

    while (1)
    {
        // Restart the playing
        SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
        Delay_Ms(5000);
    }
}
