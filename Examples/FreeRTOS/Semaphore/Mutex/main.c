/**
 * Example of Semaphore Mutex
 */
#include <stdlib.h>
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include <string.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

SemaphoreHandle_t mutex;
volatile uint16_t counter;

void taskCount(void *pvParameters)
{
    uint8_t id;
    id = (uint8_t)pvParameters;

    while (1)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        counter++;
        printf("%d: %d\n", id, counter);
        xSemaphoreGive(mutex);
    }
}

int main(void)
{
    BaseType_t xReturned;

    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    // Create mutex
    mutex = xSemaphoreCreateMutex();

    xReturned = xTaskCreate(taskCount, "task 1", configMINIMAL_STACK_SIZE, (void *)1, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task\r\n");
        while (1);
    }
    xReturned = xTaskCreate(taskCount, "task 2", configMINIMAL_STACK_SIZE, (void *)2, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task\r\n");
        while (1);
    }
    xReturned = xTaskCreate(taskCount, "task 3", configMINIMAL_STACK_SIZE, (void *)3, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    vTaskStartScheduler();


    return 0;
}