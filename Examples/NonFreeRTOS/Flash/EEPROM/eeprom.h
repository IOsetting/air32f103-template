#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "air32f10x.h"

#define AIR32F103_PAGE_BYTES 1024

uint16_t    AIRFLASH_ReadHalfWord(uint32_t addr);
void        AIRFLASH_Read(uint32_t addr, uint16_t *pBuf, uint16_t size);
void        AIRFLASH_Write(uint32_t addr, uint16_t *pBuf, uint16_t size);
void        AIRFLASH_EraseByPage(uint32_t addr);

#endif
