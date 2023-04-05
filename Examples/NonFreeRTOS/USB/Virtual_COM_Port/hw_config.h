#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "air32f10x.h"
#include "usb_type.h" 

 
#define USB_USART_TXFIFO_SIZE   1024    // TX buffer size
#define USB_USART_REC_LEN        200    // RX buffer size

// usb virtual serial port FIFO
typedef struct
{
    uint8_t buffer[USB_USART_TXFIFO_SIZE]; // buffer
    __IO uint16_t writeptr;                // write point
    __IO uint16_t readptr;                 // read point
} _usb_usart_fifo;
extern _usb_usart_fifo uu_txfifo;          // TX FIFO

extern uint8_t USB_USART_RX_BUF[USB_USART_REC_LEN]; // RX buffer, end with LR/CF
extern u16 USB_USART_RX_STA;                        // RX flag

/**
 * Common USB methods
*/
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config(FunctionalState NewState);
void USB_Port_Set(uint8_t enable);
void IntToUnicode(u32 value, uint8_t *pbuf, uint8_t len);
void Get_SerialNum(void);

/**
 * Methods for specific USB functions
*/
bool USART_Config(void);
void USB_To_USART_Send_Data(uint8_t *data_buffer, uint8_t Nb_bytes);
void USART_To_USB_Send_Data(void);
void USB_USART_SendData(uint8_t data);
void usb_printf(char *fmt, ...);

#endif
