/**
 * Example of Timer Callback
 */
#include <stdlib.h>
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define DELAY 1000

TimerHandle_t timer;

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // PC13
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void dimmerCallback(TimerHandle_t xTimer)
{
    (void)(xTimer); // Suppress "unused parameter" warning
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void taskUART(void *pvParameters)
{
    uint8_t c;
    (void)(pvParameters);

    while (1)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
        {
            c = (uint8_t)USART_ReceiveData(USART1);
            USART_SendData(USART1, c);
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
            xTimerStart(timer, portMAX_DELAY);
        }
        vTaskDelay(1);
    }
}

int main(void)
{
    BaseType_t xReturned;

    GPIO_Configuration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    timer = xTimerCreate(
                      "LED timer",      // Name of timer
                      DELAY,            // Period of timer (in ticks)
                      pdFALSE,          // Auto-reload
                      (void *)0,        // Timer ID
                      dimmerCallback);  // Callback function

    xReturned = xTaskCreate(taskUART, "Task", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    vTaskStartScheduler();

    return 0;
}