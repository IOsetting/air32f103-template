/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AIR32F10x_IT_H
#define __AIR32F10x_IT_H


#include "air32f10x.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#endif /* __AIR32F10x_IT_H */
