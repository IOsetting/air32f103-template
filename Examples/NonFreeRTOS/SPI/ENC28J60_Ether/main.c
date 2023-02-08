/**
 * Demo: ENC28J60 Link Layer Package Transmission Test
 * 
 * Connect ENC28J60 with PC LAN by cable, run tcpdump to capture the packets
 * 
 *   sudo tcpdump -i [net_interface] -ne
 * 
 * The output looks like:
 *   00:14:10.929753 00:00:00:00:00:00 > aa:bb:cc:dd:ee:ff, 802.3, length 0:  [|llc]
 *   00:14:11.308291 00:00:00:00:00:00 > aa:bb:cc:dd:ee:ff, 802.3, length 0:  [|llc]
 *   00:14:11.686935 00:00:00:00:00:00 > aa:bb:cc:dd:ee:ff, 802.3, length 0:  [|llc]
 * 
  * Connections:
 * 
 * | ENC28J60  | AIR32F1  | USB2TTL      | 
 * | --------- | -------- | ------------ | 
 * | GND       | GND      | GND          | 
 * | VCC       | 3.3V     |              | 
 * | RESET     | PB8      |              | 
 * | CS        | PA4      |              | 
 * | SCK       | PA5      |              | 
 * | SO        | PA6      |              | 
 * | SI        | PA7      |              | 
 * | WCL       |          |              | 
 * | INT       |          |              | 
 * | CLKOUT    |          |              | 
 * |           | PA9      | RX           | 
 * |           | PA10     | TX           | 
 * 
*/
#include <stdio.h>
#include "debug.h"
#include "enc28j60.h"

const uint8_t mac_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
const uint8_t to_addr[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

static void APP_SPI_Init(void);

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    APP_SPI_Init();

    enc28j60Init(mac_addr);
    printf("ENC28J60 initialized");

    while (1)
    {
        enc28j60PacketSend(6, to_addr);
        Delay_Ms(1000);
    }
}

static void APP_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    // A5:SCK, A6:MISO, A7:MOSI
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStruct);
    SPI_Cmd(SPI1, ENABLE);

    /* PA4: CS */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/**
* Basic SPI operation: Write and read one byte
*/
uint8_t SPI_TxRxByte(uint8_t data)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);

    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)==RESET);
    return SPI_I2S_ReceiveData(SPI1);
}
