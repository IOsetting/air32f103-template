/**
 * Example of Queue working with notification
 */
#include <stdlib.h>
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include <air32f10x_tim.h>
#include <misc.h>
#include <string.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ring_buffer.h"

#define QUEUE_SIZE      5
#define MSG_SIZE        RING_BUFFER_SIZE

typedef struct Message {
    char msg[MSG_SIZE];
    int16_t data;
} Message;

volatile uint8_t rx_flag = 0;
uint8_t *data_ptr;

TaskHandle_t taskTimer3_handler;
QueueHandle_t message_queue;
QueueHandle_t command_queue;

void taskTxRx(void *pvParameters)
{
    Message received, cmd;
    uint8_t c;
    uint16_t len;
    (void)(pvParameters);

    ring_buffer_reset();

    while (1)
    {
        // Print messages from message queue
        if (xQueueReceive(message_queue, (void *)&received, 0) == pdTRUE)
        {
            printf("Msg: %s, data: %d\r\n", received.msg, received.data);
        }
        // Send receive command to command queue
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
        {
            c = (uint8_t)USART_ReceiveData(USART1);
            if (c == '\r' || c == '\n')
            {
                USART_Print(USART1, '\r');
                USART_Print(USART1, '\n');
                c = '\0';
                ring_buffer_push(c);
            }
            else if (c == 0x7F)
            {
                ring_buffer_pop(&c);
                USART_Print(USART1, '\b');
                USART_Print(USART1, ' ');
                USART_Print(USART1, '\b');
            }
            else
            {
                ring_buffer_push(c);
                USART_Print(USART1, c);
            }

            if (c == '\0')
            {
                len = ring_buffer_size();
                if (len > 1)
                {
                    ring_buffer_read((uint8_t *)cmd.msg);
                    cmd.data = len;
                    xQueueSend(command_queue, (void *)&cmd, 10);
                }
                // Clear the buffer
                ring_buffer_reset();
            }
        }
    }
}

void taskExeCmd(void *pvParameters)
{
    Message cmd, resp;
    (void)(pvParameters);

    while (1)
    {
        // Update delay value from command queue
        if (xQueueReceive(command_queue, (void *)&cmd, 0) == pdTRUE)
        {
            strcpy(resp.msg, "Command received");
            xQueueSend(message_queue, (void *)&resp, 10);
            vTaskDelay(500);
            strcpy(resp.msg, cmd.msg);
            resp.data = 500;
            xQueueSend(message_queue, (void *)&resp, 10);
        }
    }
}

void taskTimer3(void *pvParameters)
{
    Message resp;
    uint32_t ulNotifiedValue;

    (void)(pvParameters);

    while (1)
    {
        // Block till be notified by TIM3 IRQ
        ulNotifiedValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        if( ulNotifiedValue > 0 )
        {
            strcpy(resp.msg, "Timer interrupt");
            xQueueSend(message_queue, (void *)&resp, 10);
        }
    }
}

void TIM_Configuration(void);

int main(void)
{
    BaseType_t xReturned;

    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    // Create two queues for message delivering
    message_queue = xQueueCreate(QUEUE_SIZE, sizeof(Message));
    command_queue = xQueueCreate(QUEUE_SIZE, sizeof(Message));

    xReturned = xTaskCreate(taskTxRx, "taskTxRx", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskTxRx\r\n");
        while (1);
    }

    xReturned = xTaskCreate(taskExeCmd, "taskExeCmd", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskExeCmd\r\n");
        while (1);
    }

    xReturned = xTaskCreate(taskTimer3, "taskTimer3", configMINIMAL_STACK_SIZE, NULL, 2, &taskTimer3_handler); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskTimer3\r\n");
        while (1);
    }

    // Start timer after xTaskCreate to avoid null task handler
    TIM_Configuration();

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    return 0;
}

void TIM_Configuration(void)
{
    RCC_ClocksTypeDef clocks;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // Set counter limit to 10000
    TIM_TimeBaseStructure.TIM_Period = 9999;
    /**
     * Clock source of TIM2,3,4,5,6,7: if(APB1 prescaler =1) then PCLK1 x1, else PCLK1 x2
     * Set TIM3 clock to 1KHz
     */
    RCC_GetClocksFreq(&clocks);
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
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        // Clear interrupt flag
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        // Notify taskTimer3 to proceed
        vTaskNotifyGiveFromISR(taskTimer3_handler, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}