#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

/**
 * PA2为中断输入，下降沿触发
 */
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1); // GPIOA1为外部中断1的触发源

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;		  // EXTI0中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器

	EXTI_InitStructure.EXTI_Line = EXTI_Line1;				// EXTI1中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// EXTI1中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//使能EXTI1中断
	EXTI_Init(&EXTI_InitStructure);							//根据指定的参数初始化EXTI寄存器
}

void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) == SET) //检测EXTI1上的中断
	{
		Delay_Ms(10);
		if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == Bit_SET) //检测GPIOA上的GPIO_Pin_2引脚的电平
		{
			printf("The key is pressed\n");
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line1); //清除EXTI1上的中断标志位
}

int main(void)
{
	Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%ld\r\n", SystemCoreClock);

	printf("AIR32F103 GPIO EXTI Input Test.\n");

	GPIO_Configuration();

	while (1);
}
