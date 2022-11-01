/**
 * Example of pvPortMalloc() and vPortFree()
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

volatile uint8_t rx_flag = 0;
uint8_t *data_ptr, in[BUF_SIZE];

/**
 * CR: 0x0D, \r, this will be ignored
 * LF: 0x0A, \n, this will trigger the UART1 print
*/
void taskRX(void *pvParameters)
{
    uint8_t c, pos = 0;
    (void)(pvParameters);

    while (1)
    {
        // If RX is not empty
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
        {
            c = (uint8_t)USART_ReceiveData(USART1);
            if (c == '\r')
            {
                continue;
            }
            in[pos++] = c;

            if (pos == BUF_SIZE || c == '\n')
            {
                in[pos - 1] = '\0';
                /* For debug */
                //printf("%x %s\n", pos, in);
                if (pos > 0 && rx_flag == 0 && data_ptr == NULL)
                {
                    data_ptr = (uint8_t *)pvPortMalloc((pos + 1) * sizeof(uint8_t));
                    configASSERT(data_ptr);
                    memcpy(data_ptr, in, pos + 1);
                    rx_flag = 1;
                }
                pos = 0;
            }
        }
    }
}

void taskPrintf(void *pvParameters)
{
    (void)(pvParameters);

    while (1) 
    {
        if (rx_flag == 1)
        {
            printf("Received: %s\n", data_ptr);
            printf("Free(bytes): %d\n", xPortGetFreeHeapSize());
            // Free buffer, set pointer to null, and clear flag
            vPortFree(data_ptr);
            data_ptr = NULL;
            rx_flag = 0;
        }
        /*
         * Add a small delay,or it will take too much cpu time and cause RX data lost
         * Set this task a lower priority doesn't help.
         */ 
        vTaskDelay(1);
    }
}

int main(void)
{
    BaseType_t xReturned;

    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    xReturned = xTaskCreate(taskRX, "TaskRX", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create TaskRX\r\n");
        while (1);
    }

    xReturned = xTaskCreate(taskPrintf, "TaskPrintf", configMINIMAL_STACK_SIZE, NULL, 2, NULL); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create TaskPrintf\r\n");
        while (1);
    }

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    return 0;
}