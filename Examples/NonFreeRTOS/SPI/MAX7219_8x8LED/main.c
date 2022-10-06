/*
 *@Note MAX7219 8x8LED
 * PA4 -> CS
 * PA5 -> SCK
 * PA7 -> DIN
*/
#include "air32f10x_spi.h"
#include "debug.h"

#define DECODE_MODE  0x09
#define INTENSITY    0x0A
#define SCAN_LIMIT   0x0B
#define SHUT_DOWN    0x0C
#define DISPLAY_TEST 0x0F

const uint8_t numbers[] = {
    0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xCE, 0xD6, 0xD6, // -0-.
    0xE6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x38, 0x78, 0x18, 0x18, 0x18, // -1-
    0x18, 0x18, 0x18, 0x7E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0x06, 0x0C, 0x18, 0x30, // -2-
    0x60, 0xC0, 0xC6, 0xFE, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0x06, 0x06, 0x3C, 0x06, // -3-
    0x06, 0x06, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0C, 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, // -4-
    0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFE, 0xC0, 0xC0, 0xC0, 0xFC, 0x0E, // -5-
    0x06, 0x06, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x60, 0xC0, 0xC0, 0xFC, 0xC6, // -6-
    0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xFE, 0xC6, 0x06, 0x06, 0x0C, 0x18, // -7-
    0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0xC6, // -8-
    0xC6, 0xC6, 0xC6, 0x7C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7E, 0x06, // -9-
    0x06, 0x06, 0x0C, 0x78, 0x00, 0x00, 0x00, 0x00};

/*******************************************************************************
* Function Name  : SPI_FullDuplex_Init
* Description    : Configuring the SPI for full-duplex communication.
* Input          : None
* Return         : None
*******************************************************************************/
void SPI_FullDuplex_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);
    // PA4: CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // PA5: SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // PA6: MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // PA7: MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // SPI1
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI_TxRxByte(uint8_t TxData)
{
    uint8_t SPITimeout = 0xFF;
    /* Check the status of Transmit buffer Empty flag */
    while ((SPI1->SR & SPI_I2S_FLAG_TXE) == RESET)
    {
        if (SPITimeout-- == 0) return 0;
    }
    SPI1->DR = TxData;
    SPITimeout = 0xFF;
    while ((SPI1->SR & SPI_I2S_FLAG_RXNE) == RESET)
    {
        if (SPITimeout-- == 0) return 0;
    }
    // Read from RX buffer
    return SPI1->DR;
}

void MAX7219_write(uint8_t addr, uint8_t dat)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    SPI_TxRxByte(addr);
    SPI_TxRxByte(dat);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

void MAX7219_init(void)
{
    MAX7219_write(SHUT_DOWN, 0x01);    // 0x00:shutdown, 0x01:normal
    MAX7219_write(DECODE_MODE, 0x00);  // Bypass code B decoder, no-decode operation
    MAX7219_write(SCAN_LIMIT, 0x07);   // Scan-limit, 0:1-digit, 1:2-digits, ... 7:8-digits
    MAX7219_write(INTENSITY, 0x01);    // 0x00:min, 0xFF:max
    MAX7219_write(DISPLAY_TEST, 0x00); // 0x00:normal, 0x01:test mode
}

int main(void)
{
    uint8_t pos = 0, size = sizeof(numbers), i, j;

    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    SPI_FullDuplex_Init();
    printf("Host Mode\r\n");
    Delay_Ms(2000);

    MAX7219_init();

    while (1)
    {
        for (i = 0; i < 8; i++)
        {
            j = (pos + i) % size;
            MAX7219_write(i + 1, numbers[j]);
        }
        pos = (pos + 1) % size;
        Delay_Ms(200);
    }
}
