#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define GPIO_GROUP_TEST GPIOB
#define GPIO_MODE_TEST GPIO_Mode_Out_PP
#define GPIO_SPEED_TEST GPIO_Speed_50MHz
#define GPIO_PIN1_TEST GPIO_Pin_2
#define GPIO_PIN2_TEST GPIO_Pin_10
#define GPIO_PIN3_TEST GPIO_Pin_11

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOB时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN1_TEST | GPIO_PIN2_TEST | GPIO_PIN3_TEST;
	GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_TEST; //速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_TEST;	 //输出模式
	GPIO_Init(GPIO_GROUP_TEST, &GPIO_InitStructure); //初始化GPIOB.2,10,11
}

void vTaskFunction(void *pvParameters)
{
    (void)(pvParameters);   // Suppress "unused parameter" warning

    while (1)
    {
        GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN1_TEST);
		vTaskDelay(200);
		GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN2_TEST);
		vTaskDelay(200);
		GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN3_TEST);
		vTaskDelay(200);
		GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN1_TEST);
		vTaskDelay(200);
		GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN2_TEST);
		vTaskDelay(200);
		GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN3_TEST);
		vTaskDelay(200);
    }
}

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
     * Don't use Delay_Init(), which conflict with FreeRTOS tick handling
     */
    GPIO_Configuration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    xReturned = xTaskCreate(
        vTaskFunction,              // Task function point
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
        vTaskFunction2,             // Task function point
        "TaskUART",                 // Task name
        configMINIMAL_STACK_SIZE,   // Use the minimum stack size, each take 4 bytes(32bit)
        NULL,                       // Parameters
        2,                          // Priority
        NULL);                      // Task handler

    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create task 2\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was not enough heap space to create the idle task. */
    return 0;
}