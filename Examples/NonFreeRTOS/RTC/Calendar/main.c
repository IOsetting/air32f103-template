#include <inttypes.h>
#include <air32f10x_conf.h>
#include "debug.h"

void NVIC_Configuration(void);
void RTC_Configuration(void);
uint32_t Time_Regulate(void);

__IO uint32_t TimeDisplay = 0;

int main(void)
{
    uint32_t ts, thh = 0, tmm = 0, tss = 0;
    RCC_ClocksTypeDef clocks;

    Delay_Init();
    USART_Printf_Init(115200);
    printf("RTC Calendar.\n");
    RCC_GetClocksFreq(&clocks);

    printf("SYSCLK: %"PRIu32"Khz, HCLK: %"PRIu32"Khz, PCLK1: %"PRIu32"Khz, PCLK2: %"PRIu32"Khz, ADCCLK: %"PRIu32"Khz\n", \
    clocks.SYSCLK_Frequency/1000, clocks.HCLK_Frequency/1000, \
    clocks.PCLK1_Frequency/1000, clocks.PCLK2_Frequency / 1000, clocks.ADCCLK_Frequency / 1000);

    NVIC_Configuration();
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        printf("\rRTC not yet configured....\n");
        RTC_Configuration();

        printf("RTC configured....\n");
        
        RTC_WaitForLastTask();
        RTC_SetCounter(Time_Regulate());
        RTC_WaitForLastTask();
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    else
    {
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
          printf("Power On Reset occurred....\n");
        }
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
          printf("External Reset occurred....\n");
        }

        printf("No need to configure RTC....\n");
        RTC_WaitForSynchro();

        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        RTC_WaitForLastTask();
    }

    RCC_ClearFlag();

    while (1)
    {
        if (TimeDisplay == 1)
        {
            ts = RTC_GetCounter();
            if (ts == 0x0001517F)
            {
                RTC_SetCounter(0x0);
                RTC_WaitForLastTask();
            }
            thh = ts / 3600;
            tmm = (ts % 3600) / 60;
            tss = (ts % 3600) % 60;
            printf("Time: %02"PRIu32":%02"PRIu32":%02"PRIu32"\n", thh, tmm, tss);
            TimeDisplay = 0;
        }
    }
}

void RTC_Configuration(void)
{
    unsigned char errCount = 0;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_DeInit();

    RCC_LSEConfig(RCC_LSE_ON);
    Delay_Ms(1000); // Important!
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && errCount < 250)
    {
        errCount++;
        Delay_Ms(10);
    }
    if (errCount >= 250)
    {
        // Return fail
        return;
    }
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    // RCC_LSICmd(ENABLE);
    // while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
    // RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();
    RTC_WaitForLastTask();

    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();

    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
    RTC_WaitForLastTask();
}

/**
 * Note: each input should be two numbers + LF
 */
uint32_t Time_Regulate(void)
{
    uint32_t in, hour = 0xFF, minutes = 0xFF, seconds = 0xFF;

    printf("\n==============Time Settings================\n");

    while(1)
    {
        printf("  Please set hours: ");
        scanf("%"SCNu32, &in);
        printf("%"PRIu32, in);
        hour = in;
        if (hour < 24)
        {
            printf(". Hours set to %" PRIu32, hour);
            break;
        }
    }

    while(1)
    {
        printf("\n  Please set minutes: ");
        scanf("%"SCNu32, &in);
        printf("%"PRIu32, in);
        minutes = in;
        if (minutes < 60) 
        {
            printf(". Minutes set to: %" PRIu32, minutes);
            break;
        }
    }

    while(1)
    {
        printf("\n  Please set seconds: ");
        scanf("%"SCNu32, &in);
        printf("%"PRIu32, in);
        seconds = in;
        if (seconds < 60) 
        {
            printf(". Seconds set to: %" PRIu32, seconds);
            break;
        }
    }
    
    printf("\n");
    return((hour * 3600 + minutes * 60 + seconds));
}

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /* Enable the RTC Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}