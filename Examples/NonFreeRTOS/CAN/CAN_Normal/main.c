/**
 * CAN Normal Mode Demo
 * 
 * Connections:
 * 
 * MCU         TJA1050/MCP2551
 *  PB9   ->   CANTX
 *  PB8   ->   CANRX
 *  GND   ->   GND
 *             VCC     -> 5V
 * 
 * Note: TJA1050 and MCP2551 works at 5V
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "debug.h"
#include "can.h"

// ID 11 bits, range [0x000, 0x7FF], switch the IDs for sender and receiver
#define ID_TARGET   0x123
#define ID_RECEIV   0x456

#define DATA_LEN	8

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

  printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\r\n",
         clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
         clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
  printf("AIR32F103 CAN Normal Transmit Demo. ID: %03x\r\n", ID_RECEIV);

  // loop back mode, baud rate 500Kbps(when hse = 216MHz)
  CAN_Mode_Init(CAN_SJW_1tq, CAN_BS2_8tq, CAN_BS1_9tq, 24, CAN_Mode_Normal);
  CAN_Filter_Config(ID_RECEIV << 5, 0x0, ID_RECEIV << 5, 0x0);

  while (1)
  {
    if (GetCmd() == 's')
    {
      for (i = 0; i < DATA_LEN; i++)
      {
        canbuf[i] = 0x5A + i;
      }
      res = CAN_SendData(ID_TARGET, 0x0, canbuf, 8);
      if (res == 0)
        printf("can tx succ\r\n");
      else
        printf("can tx fail\r\n");
    }

    key = CAN_ReceiveData(canbuf);
    if (key)
    {
      printf("can rx succ\r\n");
      for (i = 0; i < key; i++)
      {
        printf("canbuf[%d] = 0x%x\r\n", i, canbuf[i]);
      }
    }
  }
}
