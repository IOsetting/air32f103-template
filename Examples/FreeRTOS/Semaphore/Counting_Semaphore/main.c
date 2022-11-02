/**
 * Example of Semaphore, emulate a queue with 3 producers and 2 consumers, each producer
 * produces 100 numbers from x000 to x099, and consumers will print them.
 * 
 * mutex; protect the buf and uart output
 * empty_sema: indicate the number of empty slots, taken by producer and given by consumer
 * filled_sema: indicate the number of filled slots, taken by consumer and given by producer
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

#define QUEUE_SIZE      5
#define PRODUCERS       3
#define CONSUMERS       2
#define LIMIT           100

SemaphoreHandle_t mutex;
SemaphoreHandle_t binary_sema;
SemaphoreHandle_t empty_sema;
SemaphoreHandle_t filled_sema;

const uint16_t ids[] = {1,2,3,4,5};
uint16_t buf[QUEUE_SIZE], from = 0, to = 0;


void taskProducer(void *pvParameters)
{
    uint16_t count, id = *(uint16_t *)pvParameters;
    printf("Producer %d created\n", id);

    for (count = 0; count < LIMIT; count++)
    {
        xSemaphoreTake(empty_sema, portMAX_DELAY);
        xSemaphoreTake(mutex, portMAX_DELAY);
        buf[to] = id * 1000 + count;
        to = (to + 1) % QUEUE_SIZE;
        xSemaphoreGive(mutex);
        xSemaphoreGive(filled_sema);
    }
    vTaskDelete(NULL);
}

void taskConsumer(void *pvParameters)
{
    uint16_t id = *(uint16_t *)pvParameters;
    printf("Consumer %d created\n", id);

    while (1)
    {
        xSemaphoreTake(filled_sema, portMAX_DELAY);
        xSemaphoreTake(mutex, portMAX_DELAY);
        printf("%d: %d\n", id, buf[from]);
        from = (from + 1) % QUEUE_SIZE;
        xSemaphoreGive(mutex);
        xSemaphoreGive(empty_sema);
    }
}

int main(void)
{
    uint8_t i;
    BaseType_t xReturned;

    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    mutex = xSemaphoreCreateMutex();
    binary_sema = xSemaphoreCreateBinary();
    empty_sema = xSemaphoreCreateCounting(QUEUE_SIZE, QUEUE_SIZE);
    filled_sema = xSemaphoreCreateCounting(QUEUE_SIZE, 0);

    memset(buf, 0, QUEUE_SIZE * sizeof(uint16_t));

    for (i = 0; i < PRODUCERS; i++)
    {
        printf("Producer %d\n", i);
        xReturned = xTaskCreate(taskProducer, "taskProducer", configMINIMAL_STACK_SIZE, (void *)&ids[i], 2, NULL); 
        if (xReturned != pdPASS)
        {
            printf("FreeRTOS failed to create producer id:%d\r\n", i);
            while (1);
        }
    }

    for (i = 0; i < CONSUMERS; i++)
    {
        printf("Consumer %d\n", i);
        xReturned = xTaskCreate(taskConsumer, "taskConsumer", configMINIMAL_STACK_SIZE, (void *)&ids[i], 2, NULL); 
        if (xReturned != pdPASS)
        {
            printf("FreeRTOS failed to create consumer id:%d\r\n", i);
            while (1);
        }
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    vTaskStartScheduler();
    return 0;
}