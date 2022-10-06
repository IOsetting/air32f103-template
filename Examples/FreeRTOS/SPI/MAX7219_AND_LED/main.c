/**
 * FreeRTOS Tasks:
 * - Built-in LED blinking 
 * - MAX7219 LED matrix
 * - UART1 printf
 * 
 * MAX7219 8x8LED connection
 * - PA4 -> CS
 * - PA5 -> SCK
 * - PA7 -> DIN
 * 
 */
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include "FreeRTOS.h"
#include "debug.h"
#include "led.h"
#include "max7219.h"

#include "task.h"
#include "queue.h"



void vTaskFunction2(void *pvParameters)
{
    (void)(pvParameters);   // Suppress "unused parameter" warning

    while (1)
    {
        printf("echo\r\n");
		vTaskDelay(1000);
    }
}

int main(void)
{
    BaseType_t xReturned;

    /*
     * Don't use Delay_Init(), which conflicts with FreeRTOS tick handling
     */
    LED_Configuration();
    MAX7219_SPI_Init();
    MAX7219_Init();

    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    xReturned = xTaskCreate(
        LED_TaskFunction,           // Task function point
        "TaskLED",                  // Task name
        configMINIMAL_STACK_SIZE,   // Use the minimum stack size, each take 4 bytes(32bit)
        NULL,                       // Parameters
        2,                          // Priority
        NULL);                      // Task handler

    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 1\r\n");
        while (1);
    }

    xReturned = xTaskCreate(
        MAX7219_TaskFunction,       // Task function point
        "TaskMAX7219",              // Task name
        configMINIMAL_STACK_SIZE,   // Use the minimum stack size, each take 4 bytes(32bit)
        NULL,                       // Parameters
        2,                          // Priority
        NULL);                      // Task handler

    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 2\r\n");
        while (1);
    }

    xReturned = xTaskCreate(
        vTaskFunction2,             // Task function point
        "TaskUART",                 // Task name
        configMINIMAL_STACK_SIZE,   // Use the minimum stack size, each take 4 bytes(32bit)
        NULL,                       // Parameters
        2,                          // Priority
        NULL);                      // Task handler

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