/*
 * @Note Example of Helix MP3 Decoder
 *
 *   AIR32F103                  MAX98357A / PT8211
 *   PB12(SPI1_NSS/I2S_WS)       -> LRC, WS
 *   PB13(SPI1_SCK/I2S_CK)       -> BCLK, BCK
 *   PB15(SPI1_MOSI/I2S_SD)      -> DIN
 *                               GND  -> GND
 *                               VIN  -> 3.3V
 *                               +    -> speaker
 *                               -    -> speaker
 * 
 * - Change I2S standard according to your I2S device: LSB for PT8211, Phillips for MAX98357A
 * - System clock should be above 192MHz, otherwise there will be obvious lags in the sound
*/
#include "air32f10x_spi.h"
#include "air32f10x_dma.h"
#include "debug.h"
#include "mp3dec.h"
#include "mp3_data.h"


static int16_t audio_buffer[2][4096];
volatile uint32_t time_var1, time_var2, audio_buffer_idx = 0, load_flag = 0;
volatile uint16_t samples_size = 4096;
MP3FrameInfo mp3FrameInfo;
HMP3Decoder hMP3Decoder;


void load_next_frame(void)
{
  int offset, err;
  int outOfData = 0;
  static const char *read_ptr = mp3_data;
  static int bytes_left = MP3_DATA_SIZE;

  int16_t *samples = audio_buffer[1 - audio_buffer_idx];
  /* prepare new buffer */
  offset = MP3FindSyncWord((unsigned char *)read_ptr, bytes_left);
  bytes_left -= offset;

  if (bytes_left <= 5000)
  {
    /* start over */
    read_ptr = mp3_data;
    bytes_left = MP3_DATA_SIZE;
    offset = MP3FindSyncWord((unsigned char *)read_ptr, bytes_left);
  }

  /* decocd for one frame */
  read_ptr += offset;
  err = MP3Decode(hMP3Decoder, (unsigned char **)&read_ptr, &bytes_left, samples, 0);

  if (err)
  {
    /* error occurred */
    switch (err)
    {
    case ERR_MP3_INDATA_UNDERFLOW:
      outOfData = 1;
      break;
    case ERR_MP3_MAINDATA_UNDERFLOW:
      /* do nothing - next call to decode will provide more mainData */
      break;
    case ERR_MP3_FREE_BITRATE_SYNC:
    default:
      outOfData = 1;
      break;
    }
  }
  else
  {
    /* no error, read samples size */
    MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
  }

  if (!outOfData)
  {
    samples_size = mp3FrameInfo.outputSamps;
    load_flag = 0;
  }
}

void IIS_Configuration(void)
{
    I2S_InitTypeDef I2S_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    // PB12,PB13,PB15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // PC13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    SPI_I2S_DeInit(SPI2);
    I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
    // PT8211:LSB,  MAX98357A:Phillips
    I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;
    I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_44k;
    I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
    I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
    I2S_Init(SPI2, &I2S_InitStructure);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    I2S_Cmd(SPI2, ENABLE);
}

void DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel5);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)mp3_data;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = samples_size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ClearFlag(DMA1_FLAG_GL5);
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
}

void DMA1_Channel5_IRQHandler(void)
{
    int16_t *samples;

    if (DMA_GetITStatus(DMA1_IT_TC5) == SET)
    {
        audio_buffer_idx = 1 - audio_buffer_idx;
        samples = audio_buffer[audio_buffer_idx];
        // switch to another buffer
        /* Disable DMA Channel */
        DMA1_Channel5->CCR &= (uint16_t)(~DMA_CCR1_EN);
        DMA1_Channel5->CNDTR = samples_size;
        DMA1_Channel5->CMAR = (uint32_t)samples;
        /* Enable DMA Channel */
        DMA1_Channel5->CCR |= DMA_CCR1_EN;
        // set the flag for next frame loading
        load_flag = 1;
    }
    DMA_ClearFlag(DMA1_FLAG_GL5);
}

int main(void)
{
    Delay_Init();

    USART_Printf_Init(115200);
    printf("\nHelix MP3 Demo Start\n");

    Delay_Ms(2000);

    IIS_Configuration();

    DMA_Configuration();

    // Enable DMA transfer
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    hMP3Decoder = MP3InitDecoder();

    while (1)
    {
        if (load_flag == 1)
        {
            load_next_frame();
        }
    }
}
