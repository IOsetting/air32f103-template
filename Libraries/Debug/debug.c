/********************************** (C) COPYRIGHT  *******************************
* File Name          : debug.c
* Description        : This file contains the functions for UART Printf
* 
*******************************************************************************/
#include "debug.h"
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

const char hexes[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void USART_Print_Hex(USART_TypeDef* USARTx, uint16_t ch)
{
    USARTx->DR = hexes[ch >> 8 & 0x0F];
    while((USARTx->SR & USART_FLAG_TC) == (uint16_t)RESET);
    USARTx->DR = hexes[ch >> 4 & 0x0F];
    while((USARTx->SR & USART_FLAG_TC) == (uint16_t)RESET);
    USARTx->DR = hexes[ch & 0x0F];
    while((USARTx->SR & USART_FLAG_TC) == (uint16_t)RESET);
}

void USART_Print(USART_TypeDef* USARTx, uint16_t ch)
{
    USARTx->DR = ((uint16_t)ch & (uint16_t)0x01FF);
    while((USARTx->SR & USART_FLAG_TC) == (uint16_t)RESET);
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
    #define GETCHAR_PROTOTYPE int fgetc(FILE * f)
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined   (  __GNUC__  )
    #define GETCHAR_PROTOTYPE int __io_getchar (void)
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif

PUTCHAR_PROTOTYPE
{
#if (DEBUG_UART == DEBUG_UART1)
    USART1->DR = ((uint16_t)ch & (uint16_t)0x01FF);
    while((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET);
#elif (DEBUG_UART == DEBUG_UART2)
    USART2->DR = ((uint16_t)ch & (uint16_t)0x01FF);
    while((USART2->SR & USART_FLAG_TC) == (uint16_t)RESET);
#elif (DEBUG_UART == DEBUG_UART3)
    USART3->DR = ((uint16_t)ch & (uint16_t)0x01FF);
    while((USART3->SR & USART_FLAG_TC) == (uint16_t)RESET);
#endif
    return ch;
}

GETCHAR_PROTOTYPE
{
    #if (DEBUG_UART == DEBUG_UART1)
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    return (uint8_t)USART_ReceiveData(USART1);
    #elif (DEBUG_UART == DEBUG_UART2)
    while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
    return (uint8_t)USART_ReceiveData(USART2);
    #elif (DEBUG_UART == DEBUG_UART3)
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
    return (uint8_t)USART_ReceiveData(USART3);
    #endif
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
    // nothing
#elif defined   (  __GNUC__  )
__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = __io_getchar();
    }
    return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    (void)file;
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}

__attribute__((weak)) int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;

    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;

    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;

    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    {
        st->st_mode = S_IFCHR;
        return 0;
    }

    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _getpid(void)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((weak)) int _kill(pid_t pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = ENOSYS;
    return -1;
}
#endif


/*******************************************************************************
* Function Name  : USART_Printf_Init
* Description    : Initializes the USARTx peripheral.  
* Input          : baudrate: USART communication baud rate.                                 
* Return         : None
*******************************************************************************/
void USART_Printf_Init(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

#if (DEBUG_UART == DEBUG_UART1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (DEBUG_UART == DEBUG_UART2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

#elif (DEBUG_UART == DEBUG_UART3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

#if (DEBUG_UART == DEBUG_UART1)
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
#elif (DEBUG_UART == DEBUG_UART2)
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
#elif (DEBUG_UART == DEBUG_UART3)
    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);
#endif

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
    // Do nothing
#elif defined   (  __GNUC__  )
    // To avoid io buffer
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
#endif
}

static uint8_t UsNumber = 0;
static uint16_t MsNumber = 0;

void Delay_Init()
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    UsNumber = clocks.HCLK_Frequency / 8 / 1000000;
    /** LOAD is 24-bit, LOAD may overflow(> 16,777,215) if multiple by 1000 */
    MsNumber = (u16)UsNumber * 100;
}

void Delay_Us(u32 nus)
{
    u32 temp;
    SysTick->LOAD = nus * UsNumber;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
}

void Delay_Ms(u16 nms)
{
    u32 temp, round = 10;
    SysTick->LOAD = (u32)nms * MsNumber - 1;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while (round--)
    {
        do
        {
            temp = SysTick->CTRL;
        } while ((temp & 0x01) && !(temp & (1 << 16)));
    }
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
}
void Delay_S(u16 ns)
{
    u16 i;
    while (ns--)
    {
        i = 0;
        while (i++ < 10)
        {
            Delay_Ms(100);
        }
    }
}