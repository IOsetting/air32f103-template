/**
 * Example of Binary Semaphore
 */
#include <inttypes.h>
#include <stdlib.h>
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include <air32f10x_tim.h>
#include <misc.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

SemaphoreHandle_t xBinarySemaphore;
RCC_ClocksTypeDef clocks;
volatile uint32_t val;

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // Set counter limit to 10000
    TIM_TimeBaseStructure.TIM_Period = 9999;
    /**
     * Clock source of TIM2,3,4,5,6,7: if(APB1 prescaler =1) then PCLK1 x1, else PCLK1 x2
     * Set TIM3 clock to 1KHz
     */
    if (clocks.HCLK_Frequency == clocks.PCLK1_Frequency)
    {
        // clock source is PCLK1. 
        // Note: TIM_Prescaler is 16bit, [0, 65535], given PCLK1 is 36MHz, divider should > 550
        TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency / 10000 - 1;
    }
    else
    {
        // clock source is PCLK1 x2, so prescaler should be doubled
        TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency / 5000 - 1;
    }
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // Enable interrupt from 'TIM update'
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // NVIC config
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        // Clear interrupt flag
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        // Increase the value
        val++;
        // Give the semaphore so that taskUART and take and proceed
        xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void taskUART(void *pvParameters)
{
    (void)(pvParameters);

    while (1)
    {
        // Block till being notified by TIM3 Interrupt
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        printf("Notified, Val = %"PRIu32"\n", val);
    }
}

int main(void)
{
    BaseType_t xReturned;

    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);
    RCC_GetClocksFreq(&clocks);

    xBinarySemaphore = xSemaphoreCreateBinary();
    if (xBinarySemaphore == NULL)
    {
        printf("FreeRTOS failed to create binary semaphore\r\n");
        while (1);
    }

    TIM_Configuration();

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