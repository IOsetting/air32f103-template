#include <air32f10x_iwdg.h>
#include "debug.h"

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // PA0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t KEY_Pressed(void)
{
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    {
        Delay_Ms(10);
        return 1;
    }
    return 0;
}

void IWDG_Config(uint16_t prer, uint16_t rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prer);
    IWDG_SetReload(rlr);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    Delay_Init();
    KEY_Init();
    printf("IWDG test started\r\n");
    Delay_Ms(1000);
    // Watch dog will reset after around 3.2 seconds if no key-press
    IWDG_Config(IWDG_Prescaler_32, 4000);

    while (1)
    {
        if (KEY_Pressed() == 1)
        {
            printf("Key pressed\r\n");
            IWDG_ReloadCounter();
            Delay_Ms(10);
        }
    }
}
