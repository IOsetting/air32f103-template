/* Includes ------------------------------------------------------------------*/
#include "air32f10x_it.h" 


 
void NMI_Handler(void) {}
 
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1);
}
 
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1);
}

 
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1);
}
 
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1);
}
 
void SVC_Handler(void) {}
 
void DebugMon_Handler(void) {}
 
void PendSV_Handler(void) {}
 
void SysTick_Handler(void) 
{
}


/******************************************************************************/
/*                 MH32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_MH32f10x_xx.s).                                            */
/******************************************************************************/
extern __IO uint32_t TimeDisplay;
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
        /* Clear the RTC Second interrupt */
        RTC_ClearITPendingBit(RTC_IT_SEC);
        /* Enable time update */
        TimeDisplay = 1;
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }
}
