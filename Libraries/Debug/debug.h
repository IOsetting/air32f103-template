/********************************** (C) COPYRIGHT  *******************************
* File Name          : debug.h
* Description        : This file contains all the functions prototypes for UART
*                      Printf , Delay and Bit-Banding functions.
*******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H 	

#include "air32f10x.h"
#include "air32f10x_rcc.h"
#include "air32f10x_gpio.h"
#include "air32f10x_usart.h"
#include "misc.h"
#include "stdio.h"

/* UART Printf Definition */
#define DEBUG_UART1    1
#define DEBUG_UART2    2
#define DEBUG_UART3    3

/* DEBUG UATR Definition */
#define DEBUG_UART   DEBUG_UART1
//#define DEBUG_UART   DEBUG_UART2
//#define DEBUG_UART   DEBUG_UART3

/* GPIO Bit-Banding Macro Definition */
#define BITBAND(adr, number)  ((adr & 0xF0000000)+0x2000000+((adr &0xFFFFF)<<5)+(number<<2)) 
#define MEM_ADR(adr)  *((volatile unsigned long  *)(adr)) 
#define BIT_ADR(adr, number)  MEM_ADR(BITBAND(adr, number)) 

/* GPIO Output Address Mapping */
#define GPIOA_ODR_Adr    (GPIOA_BASE+12) 						//0x4001080C 
#define GPIOB_ODR_Adr    (GPIOB_BASE+12) 						//0x40010C0C 
#define GPIOC_ODR_Adr    (GPIOC_BASE+12) 						//0x4001100C 
#define GPIOD_ODR_Adr    (GPIOD_BASE+12) 						//0x4001140C 
    
/* GPIO Input Address Mapping */
#define GPIOA_IDR_Adr    (GPIOA_BASE+8) 						//0x40010808 
#define GPIOB_IDR_Adr    (GPIOB_BASE+8) 						//0x40010C08 
#define GPIOC_IDR_Adr    (GPIOC_BASE+8) 						//0x40011008 
#define GPIOD_IDR_Adr    (GPIOD_BASE+8) 						//0x40011408 

/* GPIO Output */
#define PAout(n)   BIT_ADR(GPIOA_ODR_Adr,n) 
#define PBout(n)   BIT_ADR(GPIOB_ODR_Adr,n)  
#define PCout(n)   BIT_ADR(GPIOC_ODR_Adr,n)
#define PDout(n)   BIT_ADR(GPIOD_ODR_Adr,n) 

/* GPIO Input */
#define PAin(n)    BIT_ADR(GPIOA_IDR_Adr,n)   
#define PBin(n)    BIT_ADR(GPIOB_IDR_Adr,n)     
#define PCin(n)    BIT_ADR(GPIOC_IDR_Adr,n)  
#define PDin(n)    BIT_ADR(GPIOD_IDR_Adr,n)  


void USART_Printf_Init(u32 baudrate);
void USART_Print_Hex(USART_TypeDef* USARTx, uint16_t ch);
void USART_Print(USART_TypeDef* USARTx, uint16_t ch);

void Delay_Init(void);
void Delay_Ms(uint16_t nms);
void Delay_Us(uint32_t nus);
void Delay_S(u16 ns);

#endif /* __DEBUG_H */
