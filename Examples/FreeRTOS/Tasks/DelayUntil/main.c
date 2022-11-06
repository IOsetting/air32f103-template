/**
 * Example of vTaskDelayUntil()
 */
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    // PC13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void task1(void *pvParameters)
{
    const TickType_t xBlockTime = pdMS_TO_TICKS(200);
    (void)(pvParameters);

    while (1)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        vTaskDelay(xBlockTime);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        vTaskDelay(xBlockTime);
    }
}

void task2(void *pvParameters)
{
    const TickType_t xBlockTime = pdMS_TO_TICKS(1000);
    TickType_t xLastWakeTime;
    (void)(pvParameters);

    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        printf("Task 2\r\n");
        // Execute every 1000 milliseconds exactly
        // xLastWakeTime is automatically updated within vTaskDelayUntil()
        vTaskDelayUntil(&xLastWakeTime, xBlockTime);
    }
}

int main(void)
{
    BaseType_t xReturned;

    GPIO_Configuration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    taskENTER_CRITICAL();

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

    taskEXIT_CRITICAL();

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was not enough heap space to create the idle task. */
    return 0;
}