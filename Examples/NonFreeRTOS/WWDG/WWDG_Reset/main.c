#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"


int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

    printf("AIR32F103 WWDG Test.\n");
    
    WWDG_SetPrescaler(WWDG_Prescaler_8);
    WWDG_SetWindowValue(0x5F);
    WWDG_Enable(0x7f); 
    
    while(1);
}
