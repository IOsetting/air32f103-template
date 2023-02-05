#include <stdio.h>
#include "eeprom.h"

#define AIR32F103_PAGE_BYTES 2048
uint16_t AIRFLASH_BUF[AIR32F103_PAGE_BYTES / 2]; // 2k bytes most

/**
 * Read halfword/2-byte from address, address should be an even number
*/
uint16_t AIRFLASH_ReadHalfWord(uint32_t addr)
{
    return *(volatile uint16_t *)addr;
}

/**
 * Read size of halfword/2-bytes from address, address should be an even number
*/
void AIRFLASH_Read(uint32_t addr, uint16_t *pBuf, uint16_t size)
{
    uint16_t i;
    for (i = 0; i < size; i++)
    {
        pBuf[i] = *(volatile uint16_t *)addr; // Read 2 bytes
        addr += 2;
    }
}

#if AIR32F103_FLASH_WREN

void Test_Write(uint32_t addr, uint16_t data)
{
    AIRFLASH_Write(addr, &data, 1);
}

/**
 * Write to flash without check
*/
void AIRFLASH_Write_NoCheck(uint32_t addr, uint16_t *pBuf, uint16_t size)
{
    uint16_t i;
    for (i = 0; i < size; i++)
    {
        FLASH_ProgramHalfWord(addr, pBuf[i]);
        addr += 2;
    }
}

/**
 * Write to flash
 * addr: address should be an even number
 * size: size of halfword data
*/
void AIRFLASH_Write(uint32_t addr, uint16_t *pBuf, uint16_t size)
{
    uint32_t relativeAddr;      // Address relative to 0X08000000 (in byte)
    uint32_t pages;             // Page address
    uint16_t unitPosInPage;     // Address in page (by halfword)
    uint16_t unitsToWrite;      // Number of units to be written in current page (by halfword)
    uint16_t i;

    if (addr < AIR32F103_FLASH_BASE || (addr >= (AIR32F103_FLASH_BASE + 1024 * AIR32F103_FLASH_SIZE)))
    {
        // Skip invalid address
        return;
    }
    // Unlock
    FLASH_Unlock();

    relativeAddr = addr - AIR32F103_FLASH_BASE;
    pages = relativeAddr / AIR32F103_PAGE_BYTES;
    unitPosInPage = (relativeAddr % AIR32F103_PAGE_BYTES) / 2;
    // How many units left in current page
    unitsToWrite = AIR32F103_PAGE_BYTES / 2 - unitPosInPage;
    if (size <= unitsToWrite)
    {
        // If there is enough room in current page
        unitsToWrite = size;
    }

    while (1)
    {
        // Read out all data of this page
        AIRFLASH_Read(pages * AIR32F103_PAGE_BYTES + AIR32F103_FLASH_BASE, AIRFLASH_BUF, AIR32F103_PAGE_BYTES / 2);
        // If it contains non-0xff, erase the sector
        for (i = 0; i < unitsToWrite; i++)
        {
            if (AIRFLASH_BUF[unitPosInPage + i] != 0XFFFF) break;
        }
        if (i < unitsToWrite)
        {
            // Erase this page
            FLASH_ErasePage(pages * AIR32F103_PAGE_BYTES + AIR32F103_FLASH_BASE);
            // Prepare data
            for (i = 0; i < unitsToWrite; i++)
            {
                AIRFLASH_BUF[unitPosInPage + i] = pBuf[i];
            }
            // Write back to whole page
            AIRFLASH_Write_NoCheck(pages * AIR32F103_PAGE_BYTES + AIR32F103_FLASH_BASE, AIRFLASH_BUF, AIR32F103_PAGE_BYTES / 2);
        }
        else
        {
            // Write without erase
            AIRFLASH_Write_NoCheck(addr, pBuf, unitsToWrite);
        }
        if (size == unitsToWrite)
        {
            // Finished
            break;
        }
        else
        {
            pages++;                    // Increate page number
            unitPosInPage = 0;          // Reset page offset
            pBuf += unitsToWrite;       // Buffer point move forward (by halfword)
            addr += unitsToWrite * 2;   // address move forward (by byte)
            size -= unitsToWrite;       // Remain size (by halfword)
            if (size > (AIR32F103_PAGE_BYTES / 2))
                unitsToWrite = AIR32F103_PAGE_BYTES / 2; // larger than one page
            else
                unitsToWrite = size;
        }
    };

    // Lock flash
    FLASH_Lock();
}
#endif
