#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "debug.h"

#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

int main(void)
{
  uint8_t usbstatus=0;
  uint16_t t, len, times=0;    
  RCC_ClocksTypeDef clocks;

  Delay_Init();
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  USART_Printf_Init(115200);
  RCC_GetClocksFreq(&clocks);

  printf("\n");
  printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
         clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
         clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
  printf("AIR32F103 USB Vritual Serial Port Demo.\n");

  Delay_Ms(1800);
  USB_Port_Set(0); // Disconnect
  Delay_Ms(700);
  USB_Port_Set(1); // Reconnect
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  *DP_PUUP = 1;

  while (1)
  {
    if (usbstatus != bDeviceState) // If connection status changed
    {
      usbstatus = bDeviceState; // Save new status
      if (usbstatus == CONFIGURED)
      {
        printf("USB connected\n");
      }
      else
      {
        printf("USB disconnected\n");
      }
    }
    if (USB_USART_RX_STA & 0x8000)
    {
      len = USB_USART_RX_STA & 0x3FFF; // received data length
      usb_printf("Data sent, %d bytes, content\r\n", len);
      for (t = 0; t < len; t++)
      {
        USB_USART_SendData(USB_USART_RX_BUF[t]);
      }
      usb_printf("\r\n");
      USB_USART_RX_STA = 0;
    }
    else
    {
      times++;
      if (times % 500 == 0)
      {
        usb_printf("Input something, end with return\r\n");
      }
      Delay_Ms(10);
    }
  }
}
