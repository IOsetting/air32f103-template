#include <stdio.h>
#include "debug.h"
#include "nrf24l01.h"

uint8_t RX_ADDRESS[NRF24L01_ADDR_WIDTH] = {0x32,0x4E,0x6F,0x64,0x65};
uint8_t TX_ADDRESS[NRF24L01_ADDR_WIDTH] = {0x32,0x4E,0x6F,0x64,0x22};

/*-----------------------------------------------------*/
/* Here control sending:1 or receiving:0 mode */
uint8_t Mode = 1;
/*-----------------------------------------------------*/
extern uint8_t RX_BUF[];
extern uint8_t TX_BUF[];

int main(void)
{
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

    NRF24L01_Init();
    printf("nRF24L01 initialized\r\n");

    while (NRF24L01_Check() != 0)
    {
        printf("nRF24L01 check failed\r\n");
        Delay_Ms(2000);
    }
    printf("nRF24L01 check succeeded\r\n");

    if (Mode == 1)
    {
        printf("nRF24L01 in SEND mode\r\n");
        NRF24L01_TX_Mode(RX_ADDRESS, TX_ADDRESS);
    }
    else if (Mode == 0)
    {
        printf("nRF24L01 in RECEIVE mode\r\n");
        NRF24L01_RX_Mode(RX_ADDRESS, TX_ADDRESS);
    }
    while (1)
    {
        NRF24L01_DumpConfig();

        if (Mode == 1)
        {
            uint8_t tmp[] = {0x1f,
                        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                        0x21, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x28,
                        0x31, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x38,
                        0x41, 0x12, 0x13, 0x14, 0x15, 0x16, 0x47, 0x48};
            NRF24L01_TxPacket(tmp, 32);
            Delay_Ms(3000);
        }
        else if (Mode == 0)
        {
            NRF24L01_RxPacket(RX_BUF);
        }
    }
}
