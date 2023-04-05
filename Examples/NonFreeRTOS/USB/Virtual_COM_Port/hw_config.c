#include "hw_config.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "debug.h"

_usb_usart_fifo uu_txfifo;                      // TX FIFO
uint8_t USART_PRINTF_Buffer[USB_USART_REC_LEN]; // usb_printf buffer
uint8_t USB_USART_RX_BUF[USB_USART_REC_LEN];    // RX bufer
/**
 * RX flags:
 *  bit 15: RX ready
 *  bit 14: received 0x0D
 *  bit 0~13: number of received bytes
*/
uint16_t USB_USART_RX_STA=0;

extern LINE_CODING linecoding;                  // USB serial port profile

/**
 * USB wakeup interrupt handler
*/
void USBWakeUp_IRQHandler(void) 
{
    EXTI_ClearITPendingBit(EXTI_Line18);        // Clear wakeup interrupt
} 

/**
 * USB RX interrupt handler
*/
void USB_LP_CAN1_RX0_IRQHandler(void) 
{
    USB_Istr();
} 

/**
 * USB clock config
*/
void Set_USBClock(void)
{
    // USB clock = HCLK / 4.5 = 216MHz / 4.5 = 48MHz
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_4Div5);
    // Enable USB clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

void Enter_LowPowerMode(void)
{
    printf("usb enter low power mode\r\n");
    bDeviceState = SUSPENDED;
}

void Leave_LowPowerMode(void)
{
    DEVICE_INFO *pInfo = &Device_Info;
    printf("leave low power mode\r\n");
    if (pInfo->Current_Configuration != 0)
        bDeviceState = CONFIGURED;
    else
        bDeviceState = ATTACHED;
}

void USB_Interrupts_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Configure the EXTI line 18 connected internally to the USB IP */
    EXTI_ClearITPendingBit(EXTI_Line18);

    /* Enable intrrupt on Line18 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;            // USB resume from suspend mode
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable the USB interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USB Wake-up interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}

void USB_Cable_Config(FunctionalState NewState)
{
    if (NewState != DISABLE)
        printf("usb pull up enable\r\n");
    else
        printf("usb pull up disable\r\n");
}

/**
 * enable 0:no connection, 1:allow connection
*/
void USB_Port_Set(uint8_t enable)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // Turn on peripheral clock
    if (enable)
    {
        _SetCNTR(_GetCNTR() & (~(1 << 1))); // exit poweroff mode
    }
    else
    {
        _SetCNTR(_GetCNTR() | (1 << 1));    // enter poweroff mode
        GPIOA->CRH &= 0XFFF00FFF;
        GPIOA->CRH |= 0X00033000;
        PAout(12) = 0;
    }
}

void Get_SerialNum(void)
{
    uint32_t Device_Serial0, Device_Serial1, Device_Serial2;
    Device_Serial0 = *(uint32_t *)(0x1FFFF7E8);
    Device_Serial1 = *(uint32_t *)(0x1FFFF7EC);
    Device_Serial2 = *(uint32_t *)(0x1FFFF7F0);
    Device_Serial0 += Device_Serial2;
    if (Device_Serial0 != 0)
    {
        IntToUnicode(Device_Serial0, &Virtual_Com_Port_StringSerial[2], 8);
        IntToUnicode(Device_Serial1, &Virtual_Com_Port_StringSerial[18], 4);
    }
}

/**
 * Convert 32bit value to unicode
*/
void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
    uint8_t idx = 0;
    for (idx = 0; idx < len; idx++)
    {
        if (((value >> 28)) < 0xA)
        {
            pbuf[2 * idx] = (value >> 28) + '0';
        }
        else
        {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }
        value = value << 4;
        pbuf[2 * idx + 1] = 0;
    }
}

bool USART_Config(void)
{
    uu_txfifo.readptr = 0;  // Reset read point
    uu_txfifo.writeptr = 0; // Reset write point
    USB_USART_RX_STA = 0;   // Port status reset

    printf("linecoding.format:%d\r\n", linecoding.format);
    printf("linecoding.paritytype:%d\r\n", linecoding.paritytype);
    printf("linecoding.datatype:%d\r\n", linecoding.datatype);
    printf("linecoding.bitrate:%ld\r\n", linecoding.bitrate);
    return (TRUE);
}

/**
 * databuffer: data buffer
 * Nb_bytes: number of received bytes
*/
void USB_To_USART_Send_Data(uint8_t *data_buffer, uint8_t Nb_bytes)
{
    uint8_t i;
    uint8_t res;
    for (i = 0; i < Nb_bytes; i++)
    {
        res = data_buffer[i];
        if ((USB_USART_RX_STA & 0x8000) == 0)   // RX unfinished
        {
            if (USB_USART_RX_STA & 0x4000)
            {
                if (res != 0x0A)
                    USB_USART_RX_STA = 0;       // RX error, restart
                else
                    USB_USART_RX_STA |= 0x8000; // RX completed
            }
            else // 还没收到0X0D
            {
                if (res == 0x0D)
                    USB_USART_RX_STA |= 0x4000;
                else
                {
                    USB_USART_RX_BUF[USB_USART_RX_STA & 0X3FFF] = res;
                    USB_USART_RX_STA++;
                    if (USB_USART_RX_STA > (USB_USART_REC_LEN - 1))
                        USB_USART_RX_STA = 0; // RX error, restart
                }
            }
        }
    }
}


void USB_USART_SendData(uint8_t data)
{
    uu_txfifo.buffer[uu_txfifo.writeptr] = data;
    uu_txfifo.writeptr++;
    if (uu_txfifo.writeptr == USB_USART_TXFIFO_SIZE)
    {
        uu_txfifo.writeptr = 0;
    }
}

/**
 * Print to USB serial port
 * 
 * size limit: USB_USART_REC_LEN
*/
void usb_printf(char *fmt, ...)
{
    uint16_t i, j;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char *)USART_PRINTF_Buffer, fmt, ap);
    va_end(ap);
    i = strlen((const char *)USART_PRINTF_Buffer); // get data length
    for (j = 0; j < i; j++)
    {
        USB_USART_SendData(USART_PRINTF_Buffer[j]);
    }
}
