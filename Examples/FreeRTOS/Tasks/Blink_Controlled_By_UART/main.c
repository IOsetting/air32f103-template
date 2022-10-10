/**
 * Example of xTaskCreate(), vTaskDelay() and vTaskStartScheduler()
 * 3 leads blink in different rates
 */
#include <inttypes.h>
#include <stdlib.h>
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

volatile uint32_t delay;

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // PB2, PB10, PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void task1(void *pvParameters)
{
    (void)(pvParameters); // Suppress "unused parameter" warning

    while (1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_2);
        vTaskDelay(1);
        GPIO_ResetBits(GPIOB, GPIO_Pin_2);
        vTaskDelay(delay);
    }
}

void task2(void *pvParameters)
{
    uint8_t in[10], c, pos = 0;
    int tmp;
    (void)(pvParameters);

    while (1)
    {
        // If RX is not empty, read the number and update delay
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
        {
            c = (uint8_t)USART_ReceiveData(USART1);
            in[pos++] = c;
            pos %= 10;
            if (c == '\n')
            {
                tmp = atoi((const char *)in);
                if (tmp > 0)
                {
                    delay = tmp;
                }
                pos = 0;
            }
        }
    }
}

void task3(void *pvParameters)
{
    (void)(pvParameters);

    while (1)
    {
        printf("delay = %ld\r\n", delay);
		vTaskDelay(1000);
    }
}

int main(void)
{
    BaseType_t xReturned;

    GPIO_Configuration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    delay = 500;

    xReturned = xTaskCreate(task1, "Task1", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 

    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 1\r\n");
        while (1);
    }

    xReturned = xTaskCreate(task2, "Task2", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 2\r\n");
        while (1);
    }

    xReturned = xTaskCreate(task3, "Task3", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 3\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was not enough heap space to create the idle task. */
    return 0;
}