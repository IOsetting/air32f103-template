#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

void NVIC_Configuration(void);

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

    printf("AIR32F103 WWDG Int.\n");
    printf("Interrupt Feed Dog\n");

    WWDG_SetPrescaler(WWDG_Prescaler_8);
    WWDG_SetWindowValue(0x5F);
    WWDG_Enable(0x7f); 

    WWDG_ClearFlag();
    NVIC_Configuration();
    WWDG_EnableIT();

    while(1);
}

void WWDG_IRQHandler(void)
{
    WWDG_SetCounter(0x7f);
    WWDG_ClearFlag();     
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;   
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;   
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;    
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
}
