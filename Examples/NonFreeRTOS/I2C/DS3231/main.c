/*
 * @Note DS3231 Example
 *   PB6: I2C1_SCL
 *   PB7: I2C1_SDA
*/

#include <air32f10x_i2c.h>
#include "debug.h"

#define HOST_ADDR       0xA0
#define DS3231_ADDR     0xD0

#define DS3231_REG_SECOND               0x00        /**< second register */
#define DS3231_REG_MINUTE               0x01        /**< minute register */
#define DS3231_REG_HOUR                 0x02        /**< hour register */
#define DS3231_REG_WEEK                 0x03        /**< week register */
#define DS3231_REG_DATE                 0x04        /**< date register */
#define DS3231_REG_MONTH                0x05        /**< month register */
#define DS3231_REG_YEAR                 0x06        /**< year register */
#define DS3231_REG_ALARM1_SECOND        0x07        /**< alarm1 second register */
#define DS3231_REG_ALARM1_MINUTE        0x08        /**< alarm1 minute register */
#define DS3231_REG_ALARM1_HOUR          0x09        /**< alarm1 hour register */
#define DS3231_REG_ALARM1_WEEK          0x0A        /**< alarm1 week register */
#define DS3231_REG_ALARM2_MINUTE        0x0B        /**< alarm2 minute register */
#define DS3231_REG_ALARM2_HOUR          0x0C        /**< alarm2 hour register */
#define DS3231_REG_ALARM2_WEEK          0x0D        /**< alarm2 week register */
#define DS3231_REG_CONTROL              0x0E        /**< control register */
#define DS3231_REG_STATUS               0x0F        /**< status register */
#define DS3231_REG_XTAL                 0x10        /**< xtal register */
#define DS3231_REG_TEMPERATUREH         0x11        /**< temperature high register */
#define DS3231_REG_TEMPERATUREL         0x12        /**< temperature low register */

typedef enum
{
    DS3231_FORMAT_12H = 0x01,        /**< 12h format */
    DS3231_FORMAT_24H = 0x00,        /**< 24h format */
} DS3231_HourFormat_t;

uint8_t buff[7];

uint8_t DS3231_Hex2Bcd(uint8_t hex)
{
    return (hex % 10) + ((hex / 10) << 4);
}

uint8_t DS3231_Bcd2Hex(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

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
 * Read multiple bytes
 */
void I2C_ReadBytes(uint8_t devAddr, uint8_t dataAddr, uint8_t *buf, uint16_t size)
{
    // Turn on ACK
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    // Start
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    // Send device address for TX
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    // Send data address
    I2C_SendData(I2C1, (uint8_t)(dataAddr & 0x00FF));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    // Stop
    I2C_GenerateSTOP(I2C1,ENABLE);
	while(I2C_GetFlagStatus(I2C1,I2C_FLAG_STOPF));
    // Start
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    // Send deivce address for RX
    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    // Receive data
    while(size--)
    {
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));// wait receive a byte
        *buf++ = I2C_ReceiveData(I2C1);
    }
    // Indicates that current byte is the last received byte
    I2C_NACKPositionConfig(I2C1, I2C_NACKPosition_Current);
    // Turn off ACK
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    // Stop
    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t DS3231_GetStatus(void)
{
    I2C_ReadBytes(DS3231_ADDR, DS3231_REG_STATUS, buff, 1);
    return buff[0];
}

void DS3231_GetTime(uint8_t *t)
{
    I2C_ReadBytes(DS3231_ADDR, DS3231_REG_SECOND, buff, 7);
    t[0] = DS3231_Bcd2Hex(buff[6]) + ((buff[5] >> 7) & 0x01) * 100; // year
    t[1] = DS3231_Bcd2Hex(buff[5] & 0x1F);                          // month
    t[2] = DS3231_Bcd2Hex(buff[3]); // week
    t[3] = DS3231_Bcd2Hex(buff[4]); // date
    t[7] = (buff[2] >> 6) & 0x01; // 12h/24h
    t[8] = (buff[2] >> 5) & 0x01; // am/pm
    if (t[7] == DS3231_FORMAT_12H)
    {
        t[4] = DS3231_Bcd2Hex(buff[2] & 0x1F); // hour
    }
    else
    {
        t[4] = DS3231_Bcd2Hex(buff[2] & 0x3F); // hour
    }
    t[5] = DS3231_Bcd2Hex(buff[1]); // minute
    t[6] = DS3231_Bcd2Hex(buff[0]); // second
}

int main(void)
{
    uint8_t time[10];

    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    // I2C clock = 100KHz
    IIC_Init(100000, HOST_ADDR);

    time[0] = DS3231_GetStatus();
    printf("Status: %d\r\n", time[0]);

    while(1)
    {
        DS3231_GetTime(time);
        printf("%02d-%02d-%02d %02d:%02d:%02d %d-%d\r\n", 
            time[0], time[1], time[3], time[4], time[5], time[6], time[7], time[8]);
        Delay_Ms(1000);
    }
}
