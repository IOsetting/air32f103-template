/**
 * Waveshare 1.54' E-Paper Demo
 * 
 * AIR32        E-Paper
 * - PA2        BUSY
 * - PA3        CS
 * - PA4        DC(Data/Command)
 * - PA5        SCK/SCL
 * - PA6        RES
 * - PA7        SI/SDA
 * - GND        GND
 * - 3.3V       VCC
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "debug.h"
#include "EPD_Test.h"

void APP_GPIO_Config(void);
void APP_SPI_Config(void);

int main(void)
{
  RCC_ClocksTypeDef clocks;

  Delay_Init();
  USART_Printf_Init(115200);
  RCC_GetClocksFreq(&clocks);

  printf("\n");
  printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
         clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
         clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
  printf("AIR32F103 E-Paper Demo.\n");

  APP_GPIO_Config();
  APP_SPI_Config();

  EPD_test();

  while (1);
}

void APP_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

void APP_SPI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 0;
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI_TxRx(uint8_t data)
{
  uint8_t retry = 0;
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET && ++retry);
  SPI_I2S_SendData(SPI1, data);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET && ++retry);
  return SPI_I2S_ReceiveData(SPI1);
}
