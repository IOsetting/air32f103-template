/**
 * @Note Flash EEPROM Test
 * 
 * Edit eeprom.h
 * - AIR32F103CBT6      page size is 1KB:   set AIR32F103_PAGE_BYTES to 1024
 * - AIR32F103CCT6/RPT6 page size is 2KB:   set AIR32F103_PAGE_BYTES to 2048
*/
#include <string.h>
#include <air32f10x_adc.h>
#include <eeprom.h>
#include "debug.h"

/**
 * flash start address, must not smaller than the size of this code
 * 
 * address 0X08008000 = 32K, 0X08010000 = 64K, 0X08020000 = 128K, 0X08040000 = 256K
*/
#define FLASH_ADDR 0X08009000

#define TEST_SIZE       0x800 // 2048 * 2 byte = 4096 bytes
#define BUFF_SIZE       0x200 // 512 * 2 byte = 1024 bytes
/**
 * Data size larger than one page
*/
uint16_t test_data[TEST_SIZE], buff[BUFF_SIZE];

int main(void)
{
    uint16_t i, j;
    uint32_t addr;
    RCC_ClocksTypeDef clocks;

    Delay_Init();
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf("\n");
    printf("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);
    printf("AIR32F103 Flash EEPROM Test, Write To Address: 0x%08X\n", FLASH_ADDR);

    for (i = 0; i < TEST_SIZE; i++)
    {
        test_data[i] = i;
    }
    AIRFLASH_Write(FLASH_ADDR, test_data, TEST_SIZE);   // Write to flash

    for (i = 0; i < 6; i++)
    {
        memset(buff, 0x0000, BUFF_SIZE * sizeof(uint16_t));
        addr = FLASH_ADDR - (BUFF_SIZE * 2) + BUFF_SIZE * 2 * i;
        printf("\r\nRead from 0x%08lX\r\n", addr);

        AIRFLASH_Read(addr, buff, BUFF_SIZE);         // Read from flash
        for (j = 0; j < BUFF_SIZE; j++)
        {
            printf("%04x ", buff[j]);
        }
        printf("\r\n");
    }

    /**
     * AIR32F103CBT6: erase 1KB
     * AIR32F103CCT6/RPT6: erase 2KB
    */
    printf("Erase one page at: 0x%08X\r\n", FLASH_ADDR);
    AIRFLASH_EraseByPage(FLASH_ADDR);
    
    for (i = 0; i < 6; i++)
    {
        memset(buff, 0x0000, BUFF_SIZE * sizeof(uint16_t));
        addr = FLASH_ADDR - (BUFF_SIZE * 2) + BUFF_SIZE * 2 * i;
        printf("\r\nRead from 0x%08lX\r\n", addr);

        AIRFLASH_Read(addr, (uint16_t *)buff, BUFF_SIZE);         // Read from flash
        for (j = 0; j < BUFF_SIZE; j++)
        {
            printf("%04x ", buff[j]);
        }
        printf("\r\n");
    }

    printf("Done\r\n");
    while (1);
}
