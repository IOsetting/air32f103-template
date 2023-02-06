/**
 * @Note Read Chip Part ID
 * 
*/
#include <string.h>
#include <air32f10x_adc.h>
#include "debug.h"

/**
 * AIR32F103CBT6 0x1A5A5BBx
 * AIR32F103CCT6 0x1A5A5CCx
 * AIR32F103RPT6 0x1A6A5CDx
 * AIR32F103VET6 0x1A8A6DDx
 * AIR32F103VGT6 0x1A8A6EDx
 * AIR32F103ZET6 0x1A9A6DDx
*/
#define PART_ID_ADDR    0x1FFFF7E8

/**
 * Read size of halfword/2-bytes from address
 * addr: address should be an integer multiple of 2
*/
void APP_FlashRead(uint32_t addr, uint16_t *pBuf, uint16_t size)
{
    uint16_t i;
    for (i = 0; i < size; i++)
    {
        pBuf[i] = *(volatile uint16_t *)addr; // Read 2 bytes
        addr += 2;
    }
}

int main(void)
{
    uint32_t chipId;
    RCC_ClocksTypeDef clocks;

    Delay_Init();
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf("\n");
    printf("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);
    printf("AIR32F103 Chip Part ID Test - Read from address: 0x%08X\n", PART_ID_ADDR);
    APP_FlashRead(PART_ID_ADDR, (uint16_t *)&chipId, 2);
    printf("0x%08lX\r\n", chipId);
    while (1);
}
