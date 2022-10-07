/*
 * @Note AT24C32 R/W Example
 *   PB6: I2C1_SCL
 *   PB7: I2C1_SDA
*/

#include <air32f10x_i2c.h>
#include "debug.h"

#define HOST_ADDR   0xA0
#define AT24C_ADDR  0xAE

/* EERPOM DATA ADDRESS Length Definition */
#define Address_8bit  0
#define Address_16bit  1

/* EERPOM DATA ADDRESS Length Selection */
//#define Address_Lenth   Address_8bit
#define Address_Lenth   Address_16bit

/* Global define */
#define SIZE sizeof(test_text)

/* Global Variable */ 
const uint8_t test_text[]={"I2C read/write test"};


void IIC_Init(uint32_t clockSpeed, uint16_t address)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitTSturcture;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTSturcture.I2C_ClockSpeed = clockSpeed;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitTSturcture);

    I2C_Cmd(I2C1, ENABLE);

    I2C_AcknowledgeConfig(I2C1, ENABLE);
}

/**
 * Read one byte
 *   Start > device addr(tx) > data addr > start > device addr(rx) > receive > stop.
 */
uint8_t I2C_ReadOneByte(uint8_t devAddr, uint16_t dataAddr)
{
    uint8_t temp = 0;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
    I2C_GenerateSTART(I2C1, ENABLE);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

#if (Address_Lenth == Address_8bit)
    I2C_SendData(I2C1, (uint8_t)(dataAddr & 0x00FF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

#elif (Address_Lenth == Address_16bit)
    I2C_SendData(I2C1, (uint8_t)(dataAddr >> 8));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, (uint8_t)(dataAddr & 0x00FF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

#endif

    I2C_GenerateSTART(I2C1, ENABLE);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET)
        I2C_AcknowledgeConfig(I2C1, DISABLE);

    temp = I2C_ReceiveData(I2C1);
    I2C_GenerateSTOP(I2C1, ENABLE);

    return temp;
}

/**
 * Write one byte
 *   Start > device addr(tx) > data addr > write > stop
 */
void I2C_WriteOneByte(uint8_t devAddr, uint16_t dataAddr, uint8_t DataToWrite)
{
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

#if (Address_Lenth == Address_8bit)
    I2C_SendData(I2C1, (uint8_t)(dataAddr & 0x00FF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

#elif (Address_Lenth == Address_16bit)
    I2C_SendData(I2C1, (uint8_t)(dataAddr >> 8));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, (uint8_t)(dataAddr & 0x00FF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

#endif

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE) == RESET);
    I2C_SendData(I2C1, DataToWrite);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C1, ENABLE);
}

void AT24CXX_Read(uint16_t dataAddr, uint8_t *pBuffer, uint16_t len)
{
    while (len--)
    {
        *pBuffer++ = I2C_ReadOneByte(AT24C_ADDR, dataAddr++);
    }
}

void AT24CXX_Write(uint16_t dataAddr, uint8_t *pBuffer, uint16_t len)
{
    while (len--)
    {
        I2C_WriteOneByte(AT24C_ADDR, dataAddr++, *pBuffer++);
        // Writing might halt if delay is too short
        Delay_Ms(5);
    }
}

int main(void)
{
    uint8_t data[SIZE];

    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    // I2C clock = 100KHz
    IIC_Init(100000, HOST_ADDR);

    printf("Start writing...");
    AT24CXX_Write(100, (uint8_t *)test_text, SIZE);
    printf("succ\r\n");

    Delay_Ms(500);

    printf("Start reading...");
    AT24CXX_Read(100, data, SIZE);
    printf("read: %s\r\n", data);

    while (1);
}
