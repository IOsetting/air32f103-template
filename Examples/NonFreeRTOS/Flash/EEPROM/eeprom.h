#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "air32f10x.h"

#define AIR32F103_FLASH_SIZE 512
#define AIR32F103_FLASH_WREN 1
#define AIR32F103_FLASH_BASE 0x08000000

uint16_t    AIRFLASH_ReadHalfWord(uint32_t addr);
void        AIRFLASH_Read(uint32_t addr, uint16_t *pBuf, uint16_t size);

#if AIR32F103_FLASH_WREN
void        Test_Write(uint32_t addr, uint16_t data);
void        AIRFLASH_Write(uint32_t addr, uint16_t *pBuf, uint16_t size);
void        AIRFLASH_Write_NoCheck(uint32_t addr, uint16_t *pBuf, uint16_t size);
#endif

#endif
