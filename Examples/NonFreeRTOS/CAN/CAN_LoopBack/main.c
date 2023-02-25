/**
 * CAN Loop Back Demo
 * 
 * Loopback mode will allow internal transmission of messages from the transmit buffers 
 * to the receive buffers without actually transmitting messages on the CAN bus. This mode 
 * can be used in system development and testing. In this mode, the ACK bit is ignored and 
 * the device will allow incoming messages from itself, just as if they were coming from 
 * another node. The Loopback mode is a silent mode, meaning no messages will be 
 * transmitted while in this state (including error flags or Acknowledge signals). The 
 * TXCAN pin will be in a recessive state.
 * 
 * Connections:
 * PB9 -> CANTX
 * PB8 -> CANRX
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "debug.h"
#include "can.h"


#define DATA_LEN	8


void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t GetCmd(void)
{
  uint8_t tmp = 0;

  if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
  {
    tmp = USART_ReceiveData(USART1);
  }
  return tmp;
}

int main(void)
{
  uint8_t res,i,key;
	uint8_t canbuf[8];
  RCC_ClocksTypeDef clocks;

  Delay_Init();
  USART_Printf_Init(115200);
  RCC_GetClocksFreq(&clocks);

  printf("\n");
  printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
         clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
         clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
  printf("AIR32F103 CAN Loopback.\n");
  // loop back mode, baud rate 500Kbps
  CAN_Mode_Init(CAN_SJW_1tq, CAN_BS2_8tq, CAN_BS1_9tq, 4, CAN_Mode_LoopBack);

  while (1)
  {
    if (GetCmd() == 's')
    {
      for (i = 0; i < DATA_LEN; i++)
      {
        canbuf[i] = 0x5A + i;
      }
      res = CAN_SendData(canbuf, 8);
      if (res == 0)
        printf("Loop back tx succ\n");
      else
        printf("Loop back tx fail\n");
    }

    key = CAN_ReceiveData(canbuf);
    if (key)
    {
      printf("Loop back rx succ\n");
      for (i = 0; i < key; i++)
      {
        printf("canbuf[%d] = 0x%x\n", i, canbuf[i]);
      }
    }
  }
}
