#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "air32f10x.h"
#include "air32f10x_exti.h"
#include "air32f10x_rcc_ex.h"
#include "misc.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "task.h"


#define EVENT_KEY1  (0x01 << 0)
#define EVENT_KEY2  (0x01 << 1)


static EventGroupHandle_t eventGroupHandle = NULL;

void taskUART(void *pvParameters)
{
    EventBits_t bits;
    (void)(pvParameters);

    while (1)
    {
        bits = xEventGroupWaitBits(
            eventGroupHandle,           // The event group in which the bits are set
            EVENT_KEY1 | EVENT_KEY2,    // A bitwise value that indicates the bits inside the event group
            pdTRUE,                     // Clear event bits in the event group before xEventGroupWaitBits() returns
            pdTRUE,                     // Wait for any bits or timeout
            portMAX_DELAY);             // Wait forever, or set timeout using pdMS_TO_TICKS(ms)

        // Test the return value to know which bits were set
        if ((bits & (EVENT_KEY1 | EVENT_KEY2)) == (EVENT_KEY1 | EVENT_KEY2))
        {
            printf("Key1 & Key2\r\n");
        }
        else if (bits == EVENT_KEY1)
        {
            printf("Key1\r\n");
        }
        else if (bits == EVENT_KEY2)
        {
            printf("Key2\r\n");
        }
        else
        {
            printf("Mismatch\r\n");
        }
    }
    
}

void RCC_ClkConfiguration(void);
void GPIO_Configuration(void);

int main(void)
{
    BaseType_t xReturned;

    RCC_ClkConfiguration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    eventGroupHandle = xEventGroupCreate();
    configASSERT(eventGroupHandle);

    xReturned = xTaskCreate(taskUART, "taskUART", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskUART\r\n");
        while (1);
    }

    GPIO_Configuration();

    printf("FreeRTOS Scheduler starting...\r\n");
    vTaskStartScheduler();
    return 0;
}

/**
 * Set system clock to 216MHz
*/
void RCC_ClkConfiguration(void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

    RCC_PLLCmd(DISABLE);
    AIR_RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_27, 1);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    SystemCoreClockUpdate();
}

/**
 * PA1 as input, falling edge trigger
 */
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_Init(&EXTI_InitStructure);
}

void EXTI1_IRQHandler(void)
{
    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line1) == SET)
    {
        xResult = xEventGroupSetBitsFromISR(eventGroupHandle, EVENT_KEY1, &xHigherPriorityTaskWoken);
        EXTI_ClearITPendingBit(EXTI_Line1);
        if (xResult != pdFAIL)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void EXTI2_IRQHandler(void)
{
    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line2) == SET)
    {
        xResult = xEventGroupSetBitsFromISR(eventGroupHandle, EVENT_KEY2, &xHigherPriorityTaskWoken);
        EXTI_ClearITPendingBit(EXTI_Line2);
        if (xResult != pdFAIL)
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
