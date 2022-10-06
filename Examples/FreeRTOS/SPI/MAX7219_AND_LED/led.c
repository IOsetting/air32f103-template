#include "led.h"


void LED_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOB时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN1_TEST | GPIO_PIN2_TEST | GPIO_PIN3_TEST;
    GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_TEST; //速度50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_TEST;   //输出模式
    GPIO_Init(GPIO_GROUP_TEST, &GPIO_InitStructure); //初始化GPIOB.2,10,11
}


void LED_TaskFunction(void *pvParameters)
{
    (void)(pvParameters);   // Suppress "unused parameter" warning

    while (1)
    {
        GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN1_TEST);
        vTaskDelay(200);
        GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN2_TEST);
        vTaskDelay(200);
        GPIO_SetBits(GPIO_GROUP_TEST, GPIO_PIN3_TEST);
        vTaskDelay(200);
        GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN1_TEST);
        vTaskDelay(200);
        GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN2_TEST);
        vTaskDelay(200);
        GPIO_ResetBits(GPIO_GROUP_TEST, GPIO_PIN3_TEST);
        vTaskDelay(200);
    }
}
