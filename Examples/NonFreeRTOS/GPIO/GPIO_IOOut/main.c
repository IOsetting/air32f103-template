#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "air32f10x.h"


#define GPIO_GROUP_TEST GPIOB
#define GPIO_MODE_TEST GPIO_Mode_Out_PP
#define GPIO_SPEED_TEST GPIO_Speed_50MHz
#define GPIO_PIN1_TEST GPIO_Pin_2
#define GPIO_PIN2_TEST GPIO_Pin_10
#define GPIO_PIN3_TEST GPIO_Pin_11

USART_TypeDef *USART_TEST = USART1;

void UART_Configuration(uint32_t bound);
void GPIO_Configuration(void);
/********************************************************************************/
// GPIO输出测试，开发板自带的3个灯依次亮起并熄灭实现跑马灯效果，日志通过串口1发送，波特率为115200
/********************************************************************************/
int main(void)
{
	RCC_ClocksTypeDef clocks;
	Delay_Init();
	USART_Printf_Init(115200);
	RCC_GetClocksFreq(&clocks);

	printf("\n");
	printf("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
			   (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
			   (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, (float)clocks.ADCCLK_Frequency / 1000000);
	printf("AIR32F103 GPIO Output Test\n");
	GPIO_Configuration();

	while (1)
	{
		GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN1_TEST);
		Delay_Ms(200);
		GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN2_TEST);
		Delay_Ms(200);
		GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN3_TEST);
		Delay_Ms(200);
		GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN1_TEST);
		Delay_Ms(200);
		GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN2_TEST);
		Delay_Ms(200);
		GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN3_TEST);
		Delay_Ms(200);
	}
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOB时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN1_TEST | GPIO_PIN2_TEST | GPIO_PIN3_TEST;
	GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_TEST; //速度50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_TEST;	 //输出模式
	GPIO_Init(GPIO_GROUP_TEST, &GPIO_InitStructure); //初始化GPIOB.2,10,11
}
