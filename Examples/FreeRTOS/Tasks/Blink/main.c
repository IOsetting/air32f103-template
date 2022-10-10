/**
 * Example of xTaskCreate(), vTaskDelay() and vTaskStartScheduler()
 * 3 leads blink in different rates
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
        vTaskDelay(299);
    }
}

void task2(void *pvParameters)
{
    (void)(pvParameters);

    while (1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_10);
        vTaskDelay(1);
        GPIO_ResetBits(GPIOB, GPIO_Pin_10);
        vTaskDelay(499);
    }
}

void task3(void *pvParameters)
{
    (void)(pvParameters);

    while (1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_11);
        vTaskDelay(1);
        GPIO_ResetBits(GPIOB, GPIO_Pin_11);
        vTaskDelay(999);
    }
}

void task4(void *pvParameters)
{
    (void)(pvParameters);   // Suppress "unused parameter" warning

    while (1)
    {
        printf("Task4\r\n");
		vTaskDelay(1000);
    }
}

int main(void)
{
    BaseType_t xReturned;

    GPIO_Configuration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    xReturned = xTaskCreate(
        task1,                      // Task function point
        "Task1",                    // Task name
        configMINIMAL_STACK_SIZE,   // Use the minimum stack size, each take 4 bytes(32bit)
        NULL,                       // Parameters
        2,                          // Priority
        NULL);                      // Task handler

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

    xReturned = xTaskCreate(task4, "Task4", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 4\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was not enough heap space to create the idle task. */
    return 0;
}