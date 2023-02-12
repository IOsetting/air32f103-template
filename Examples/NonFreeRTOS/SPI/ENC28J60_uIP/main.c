/*
| ENC28J60  | STM32    | USB2TTL      | 
| --------- | -------- | ------------ | 
| GND       | GND      | GND          | 
| VCC       | 3.3V     |              | 
| RESET     | PB8      |              | 
| CS        | PA4      |              | 
| SCK       | PA5      |              | 
| SI        | PA7      |              | 
| SO        | PA6      |              | 
| WCL       |          |              | 
| INT       |          |              | 
| CLKOUT    |          |              | 
|           | PA9      | RX           | 
|           | PA10     | TX           | 
*/
#include <stdio.h>
#include "air32f10x_conf.h"
#include "debug.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
#include "main.h"


#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

const uint16_t IP_ADDR[4] = {192,168,  1, 253};
const uint16_t IP_MASK[4] = {255,255,255,  0};
const uint16_t GW_ADDR[4] = {192,168,  1,  1};

static struct timer periodic_timer, arp_timer;

void SPI1_Init(void);


void UipPro(void)
{
    uint8_t i;

    uip_len = etherdev_read(uip_buf);
    if(uip_len > 0)
    {
        if(BUF->type == htons(UIP_ETHTYPE_IP))
        {
            uip_arp_ipin();
            uip_input();
            if (uip_len > 0)
            {
                uip_arp_out();
                etherdev_send(uip_buf, uip_len);
            }
        }
        else if (BUF->type == htons(UIP_ETHTYPE_ARP))
        {
            uip_arp_arpin();
            if (uip_len > 0)
            {
                etherdev_send(uip_buf, uip_len);
            }
        }
    }
    else if(timer_expired(&periodic_timer)) // every 0.5 second
    {
        timer_reset(&periodic_timer);

        for(i = 0; i < UIP_CONNS; i++)
        {
            uip_periodic(i);
            if(uip_len > 0)
            {
                uip_arp_out();
                etherdev_send(uip_buf, uip_len);
            }
        }

    #if UIP_UDP
        for(i = 0; i < UIP_UDP_CONNS; i++)
        {
            uip_udp_periodic(i); 
            /* If the above function invocation resulted in data that
            should be sent out on the network, the global variable
            uip_len is set to a value > 0. */
            if(uip_len > 0)
            {
            uip_arp_out();
            etherdev_send(uip_buf, uip_len);
            }
        }
    #endif /* UIP_UDP */

        if (timer_expired(&arp_timer))
        {
            timer_reset(&arp_timer);
            uip_arp_timer();
        }
    }
}

int main(void)
{
    RCC_ClocksTypeDef clocks;
    Delay_Init();
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf("\r\nSYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\r\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);
    printf("AIR32F103 uIP/ENC28J60 Demo\r\n");

    SPI1_Init();
    TIM1_Init();

    etherdev_init();
    uip_init();

    uip_ipaddr_t ipaddr;
    // uip_hostaddr (host address, chip address)
    printf("IP Address: %d.%d.%d.%d\r\n", IP_ADDR[0], IP_ADDR[1], IP_ADDR[2], IP_ADDR[3]);
    uip_ipaddr(ipaddr, IP_ADDR[0], IP_ADDR[1], IP_ADDR[2], IP_ADDR[3]);
    uip_sethostaddr(ipaddr);

    // uip_netmask: IP network mask
    printf("IP Network Mask: %d.%d.%d.%d\r\n", IP_MASK[0], IP_MASK[1], IP_MASK[2], IP_MASK[3]);
    uip_ipaddr(ipaddr, IP_MASK[0], IP_MASK[1], IP_MASK[2], IP_MASK[3]);
    uip_setnetmask(ipaddr);

    // uip_draddr: default route (default gateway)
    printf("GW Address: %d.%d.%d.%d\r\n", GW_ADDR[0], GW_ADDR[1], GW_ADDR[2], GW_ADDR[3]);
    uip_ipaddr(ipaddr, GW_ADDR[0], GW_ADDR[1], GW_ADDR[2], GW_ADDR[3]);
    uip_setdraddr(ipaddr);

    uip_listen(HTONS(1200));

    timer_set(&periodic_timer, CLOCK_SECOND / 2);  // 0.5 second timer
    timer_set(&arp_timer, CLOCK_SECOND * 10);      // 10 second timer

    while (1)
    {
        UipPro();
    }
}


void SPI1_Init(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  /* SCK,MISO MOSI */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* CS */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA, GPIO_Pin_4);

  SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);
  SPI_Cmd(SPI1, ENABLE); 

  GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

unsigned char	SPI1_ReadWrite(unsigned char writedat)
{
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1, writedat);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  return SPI_I2S_ReceiveData(SPI1);
}
