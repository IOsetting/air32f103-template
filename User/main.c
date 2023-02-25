#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "debug.h"


void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

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
  printf("AIR32F103 GPIO Output Test.\n");
  GPIO_Configuration();

  while (1)
  {
    GPIO_SetBits(GPIOB, GPIO_Pin_2);
    Delay_Ms(200);
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
    Delay_Ms(200);
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
    Delay_Ms(200);
    GPIO_ResetBits(GPIOB, GPIO_Pin_2);
    Delay_Ms(200);
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
    Delay_Ms(200);
    GPIO_ResetBits(GPIOB, GPIO_Pin_11);
    Delay_Ms(200);
  }
}
