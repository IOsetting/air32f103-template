/**
 * Example of Queue operations
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

#define BUF_SIZE        50
#define QUEUE_SIZE      5
#define BLINK_PERIOD    100

typedef struct Message {
    char msg[20];
    int16_t data;
} Message;

const char  cmd_delay[]     = "delay ";
uint8_t     cmd_delay_len   = strlen(cmd_delay);

QueueHandle_t echo_queue;
QueueHandle_t cmd_queue;
volatile uint8_t rx_flag = 0;
uint8_t *data_ptr, cmd_buf[BUF_SIZE];


void taskTxRx(void *pvParameters)
{
    Message received;
    uint8_t c, pos = 0;
    int16_t delay;
    (void)(pvParameters);

    memset(cmd_buf, 0, BUF_SIZE);

    while (1)
    {
        // Print echos from blink task
        if (xQueueReceive(echo_queue, (void *)&received, 0) == pdTRUE)
        {
            printf("Echo: %s, data: %d\n", received.msg, received.data);
        }
        // Read input into buffer, if RX is not empty
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
        {
            c = (uint8_t)USART_ReceiveData(USART1);
            cmd_buf[pos++] = c;
            // Start over if length exceeds buffer size
            pos %= BUF_SIZE;

            if (c == '\n' || c == '\r')
            {
                // Ignore empty input
                if (pos > 1)
                {
                    // Check if the buffer starts with "delay "
                    if (memcmp(cmd_buf, cmd_delay, cmd_delay_len) == 0)
                    {
                        // Convert last part to positive integer
                        uint8_t *tail = cmd_buf + cmd_delay_len;
                        delay = atoi((char *)tail);
                        delay = abs(delay);
                        // Send new delay value to command queue
                        if (xQueueSend(cmd_queue, (void *)&delay, 10) != pdTRUE) 
                        {
                            printf("error in cmd queue\n");
                        }
                    }
                }
                pos = 0;
                printf("\n");
            }
            else
            {
                printf("%c", c);
            }
        }
    }
}

void taskBlink(void *pvParameters)
{
    Message echo;
    uint8_t counter;
    int16_t delay = 500;

    (void)(pvParameters);

    while (1)
    {
        // Update delay value from command queue
        if (xQueueReceive(cmd_queue, (void *)&delay, 0) == pdTRUE) 
        {
            strcpy(echo.msg, "Cmd received");
            echo.data = delay;
            xQueueSend(echo_queue, (void *)&echo, 10);
        }
        // Blink
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        vTaskDelay(delay);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        vTaskDelay(delay);
        // Increase counter and echo
        counter++;
        if (counter >= BLINK_PERIOD)
        {
            strcpy(echo.msg, "Counter event");
            echo.data = counter;
            xQueueSend(echo_queue, (void *)&echo, 10);
            counter = 0;
        }
    }
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

int main(void)
{
    BaseType_t xReturned;

    GPIO_Configuration();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    // Create two queues for message delivering
    echo_queue = xQueueCreate(QUEUE_SIZE, sizeof(Message));
    cmd_queue = xQueueCreate(QUEUE_SIZE, sizeof(int16_t));

    xReturned = xTaskCreate(taskTxRx, "taskTxRx", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskTxRx\r\n");
        while (1);
    }

    xReturned = xTaskCreate(taskBlink, "taskBlink", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskBlink\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    vTaskStartScheduler();
    return 0;
}