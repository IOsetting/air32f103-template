/**
 * FreeRTOS & LVGL Demo
 * 
 * WIRING:
 *   AIR32      ST7735
 * - PA2        BL
 * - PA3        CS
 * - PA4        DC(Data/Command)
 * - PA5        SCK/SCL
 * - PA6        RES
 * - PA7        SI/SDA
 * - GND        GND
 * - 3.3V       VCC
 * BOARD: AIR32F103CCT6 / AIR32F103RPT6
 * LCD:   ST7735 128x160 RGB LCD
 */
#include <air32f10x.h>
#include <air32f10x_gpio.h>
#include <air32f10x_rcc.h>
#include "debug.h"
#include "st7735.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lvgl.h"
#include "demos/lv_demos.h"

#define LVGL_TASK_STACK_SIZE 4096
#define LVGL_TASK_PRORITY 4

static RCC_ClocksTypeDef clocks;

static lv_disp_draw_buf_t draw_buf;
// Declare a buffer for 1/10 screen size
static lv_color_t buf1[ST7735_WIDTH * ST7735_HEIGHT / 10];
// Descriptor of a display driver
static lv_disp_drv_t disp_drv;

void APP_GPIO_Config(void);
void APP_SPI_Config(void);
void TIM3_Configuration(void);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t len;
  DMA_InitTypeDef initStructure;

  len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) * 2;

  ST7735_CS_LOW;
  ST7735_SetAddrWindow(area->x1, area->y1, area->x2, area->y2);
  /* DMA1 Channel3 (triggered by SPI1 Tx event) Config */
  DMA_DeInit(DMA1_Channel3);
  initStructure.DMA_BufferSize = len;
  initStructure.DMA_M2M = DMA_M2M_Disable;
  initStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  initStructure.DMA_MemoryBaseAddr = (uint32_t)color_p;
  initStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  initStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  initStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
  initStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  initStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  initStructure.DMA_Priority = DMA_Priority_High;
  initStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_Init(DMA1_Channel3, &initStructure);
  // Start transfer
  DMA_Cmd(DMA1_Channel3, ENABLE);
  while (!DMA_GetFlagStatus(DMA1_FLAG_TC3));
  DMA_ClearFlag(DMA1_FLAG_TC3);
  DMA_Cmd(DMA1_Channel3, DISABLE);
  ST7735_CS_HIGH;

  // Indicate you are ready with the flushing
  lv_disp_flush_ready(disp);
}

static void lvglTaskHandler(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(10);
  (void)(pvParameters); // Suppress "unused parameter" warning

  ST7735_Init();

  lv_init();

  // Initialize the display buffer.
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, ST7735_WIDTH * ST7735_HEIGHT / 10);

  lv_disp_drv_init(&disp_drv);                /*Basic initialization*/
  disp_drv.flush_cb = my_disp_flush;          /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;              /*Assign the buffer to the display*/
  disp_drv.hor_res = ST7735_WIDTH;            /*Set the horizontal resolution of the display*/
  disp_drv.ver_res = ST7735_HEIGHT;           /*Set the vertical resolution of the display*/
  lv_disp_drv_register(&disp_drv);            /*Finally register the driver*/

  lv_demo_benchmark();

  while (1)
  {
    lv_timer_handler();
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

int main(void)
{
  USART_Printf_Init(115200);
  RCC_GetClocksFreq(&clocks);

  printf("SYSCLK: %ld, HCLK: %ld, PCLK1: %ld, PCLK2: %ld, ADCCLK: %ld\n",
         clocks.SYSCLK_Frequency, clocks.HCLK_Frequency,
         clocks.PCLK1_Frequency, clocks.PCLK2_Frequency, clocks.ADCCLK_Frequency);
  printf("AIR32F103 FreeRTOS LVGL Demo.\r\n");

  APP_GPIO_Config();
  APP_SPI_Config();

  xTaskCreate(
        lvglTaskHandler,              // Task function point
        "LVGL Task",                  // Task name
        LVGL_TASK_STACK_SIZE / sizeof(StackType_t),   // Use the minimum stack size, each take 4 bytes(32bit)
        NULL,                         // Parameters
        LVGL_TASK_PRORITY,            // Priority
        NULL);                        // Task handler

  TIM3_Configuration();

  printf("FreeRTOS Scheduler starting...\r\n");
  /* Start the scheduler. */
  vTaskStartScheduler();

  /* Will only get here if there was not enough heap space to create the idle task. */
  return 0;
}

void APP_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6);
}

void APP_SPI_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_7);

  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 0;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* Enable SPI1 DMA TX request */
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_Cmd(SPI1, ENABLE);
}

/**
 * TIM3 for lv_tick_inc
*/
void TIM3_Configuration(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  // Set counter limit to 100 -- interval will be 1ms
  TIM_TimeBaseStructure.TIM_Period = 100 - 1;
  /**
   * Clock source of TIM2,3,4,5,6,7: if(APB1 prescaler =1) then PCLK1 x1, else PCLK1 x2
   * */
  if (clocks.HCLK_Frequency == clocks.PCLK1_Frequency)
  {
    // clock source is PCLK1 x1.
    // Note: TIM_Prescaler is 16bit, [0, 65535]
    TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency / 100000 - 1;
  }
  else
  {
    // clock source is PCLK1 x2
    TIM_TimeBaseStructure.TIM_Prescaler = clocks.PCLK1_Frequency * 2 / 100000 - 1;
  }
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TDTS = Tck_tim
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  // Enable interrupt from 'TIM update'
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

  // NVIC config
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    // Clear INT flag
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    // Required for the internal timing of LVGL
    lv_tick_inc(1);
  }
}