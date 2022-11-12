/**
 * Example of Queue working with notification
 */
#include <stdlib.h>
#include <air32f10x.h>
#include <air32f10x_adc.h>
#include <air32f10x_dma.h>
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

#define BUFF_SIZE 8000

#define QUEUE_SIZE      5
#define MSG_SIZE        RING_BUFFER_SIZE

typedef struct Message {
    char msg[MSG_SIZE];
    int16_t data;
} Message;

volatile uint8_t rx_flag = 0;
uint8_t *data_ptr;
uint16_t dma_buf[BUFF_SIZE];

TaskHandle_t taskDmaComplete_handler;
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
            resp.data = cmd.data;
            xQueueSend(message_queue, (void *)&resp, 10);
            vTaskDelay(500);
            strcpy(resp.msg, cmd.msg);
            resp.data = dma_buf[BUFF_SIZE - 1];
            xQueueSend(message_queue, (void *)&resp, 10);
        }
    }
}

void taskDmaComplete(void *pvParameters)
{
    Message resp;
    uint32_t ulNotifiedValue;

    (void)(pvParameters);

    while (1)
    {
        // Block till be notified by DMA1 IRQ
        ulNotifiedValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        if( ulNotifiedValue > 0 )
        {
            strcpy(resp.msg, "DMA interrupt");
            resp.data = dma_buf[BUFF_SIZE - 1];
            xQueueSend(message_queue, (void *)&resp, 10);
        }
    }
}

void RCC_Configuration(void);
void TIM_Configuration(void);
void GPIO_Configuration(void);
void ADC_Configuration(void);
void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, uint32_t ppadr, uint32_t memadr, uint16_t bufsize);
void NVIC_Configuration(void);

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

    xReturned = xTaskCreate(taskDmaComplete, "taskDmaComplete", configMINIMAL_STACK_SIZE, NULL, 2, &taskDmaComplete_handler); 
    if (xReturned != pdPASS)
    {
        printf("FreeRTOS failed to create taskDmaComplete\r\n");
        while (1);
    }

    RCC_Configuration();
    GPIO_Configuration();
    ADC_Configuration();
    DMA_Tx_Init(DMA1_Channel1, (uint32_t)&ADC1->DR, (uint32_t)dma_buf, BUFF_SIZE);
    NVIC_Configuration();
    TIM_Configuration();

    printf("FreeRTOS Scheduler starting...\r\n");
    /* Start the scheduler. */
    vTaskStartScheduler();

    return 0;
}

/**
 * Peripheral configurations
*/

void RCC_Configuration(void)
{
    // Enable periphal clock
    RCC_ADCCLKConfig(RCC_PCLK2_Div36);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    // ADC clock =  PCLK2 / 8
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    // Set TIM3 period to 9000, clock at 72MHz
    TIM_TimeBaseStructure.TIM_Period = 9 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // Enable TIM3 'TIM update' trigger output
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    /*
     * Don't set timer interrupt and NVIC 
    */
    TIM_Cmd(TIM3, ENABLE);
}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // GPIO: A2 as analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void ADC_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    // Reset ADC1
    ADC_DeInit(ADC1);
    // Independent mode, continuous
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    // Select TIM3 trigger output as external trigger
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);

    // Enable ADC1 external trigger
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);

    // Enable ADC1 with DMA
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    // Calibration
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

/*******************************************************************************
* Function Name  : DMA_Tx_Init
* Description    : Initializes the DMA Channelx configuration.
* Input          : DMA_CHx:
*                    x can be 1 to 7.
*                  ppadr: Peripheral base address.
*                  memadr: Memory base address.
*                  bufsize: DMA channel buffer size.
* Return         : None
*******************************************************************************/
void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, uint32_t ppadr, uint32_t memadr, uint16_t bufsize)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_DeInit(DMA_CHx);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    // Addresss increase - peripheral:no, memory:yes
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    // Data unit size: 16bit
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    // Memory to memory: no
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA_CHx, &DMA_InitStructure);
    // Enable 'Transfer complete' interrupt
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    // Enable DMA
    DMA_Cmd(DMA_CHx, ENABLE);
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void DMA1_Channel1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL1);
        //printf("%d %d\r\n", dma_buf[BUFF_SIZE - 2], dma_buf[BUFF_SIZE - 1]);
        // Notify taskTimer3 to proceed
        vTaskNotifyGiveFromISR(taskDmaComplete_handler, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
