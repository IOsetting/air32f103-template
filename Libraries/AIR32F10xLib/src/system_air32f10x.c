/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup air32f10x_system
  * @{
  */  
  

#include "air32f10x.h"


/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */ 
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x0 /*!< Vector Table base offset field. 
                                  This value must be a multiple of 0x200. */

#ifdef SYSCLK_HSE
#define SYSCLK_FREQ_HSE         SYSCLK_HSE
#endif

/**
  * @}
  */

/** @addtogroup air32f10x_System_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @addtogroup air32f10x_System_Private_Variables
  * @{
  */

/*******************************************************************************
*  Clock Definitions
*******************************************************************************/
#ifdef SYSCLK_HSE                                               /*!< HSE Selected as System Clock source */
  uint32_t SystemCoreClock         = SYSCLK_FREQ_HSE;
#else                                                           /*!< HSI Selected as System Clock source */
  uint32_t SystemCoreClock         = HSI_VALUE;
#endif

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};


/**
  * @}
  */

/* Provide a weak function to handle system clock failure*/
__attribute__((weak))  void clockFailure(void){
  NVIC_SystemReset();     /* Timeout reached, oscillator not working */
}

/** @addtogroup air32f10x_System_Private_FunctionPrototypes
  * @{
  */

static void SetSysClock(void);
#ifdef SYSCLK_FREQ_HSE
static void SetSysClockToHSE(void);
#endif

#ifdef DATA_IN_ExtSRAM
  static void SystemInit_ExtMemCtl(void); 
#endif /* DATA_IN_ExtSRAM */

/**
  * @}
  */

/** @addtogroup air32f10x_System_Private_Functions
  * @{
  */

/**
  * @brief  Setup the microcontroller system
  *         Initialize the Embedded Flash Interface, the PLL and update the 
  *         SystemCoreClock variable.
  * @note   This function should be used only after reset.
  * @param  None
  * @retval None
  */
void SystemInit (void)
{
  /* Reset the RCC clock configuration to the default reset state(for debug purpose) */

  RCC->CR |= (uint32_t)0x00000001;                          /* Set HSION bit */
  RCC->CFGR &= (uint32_t)0xF8FF0000;                        /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;                          /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFFFBFFFF;                          /* Reset HSEBYP bit */
  RCC->CFGR &= (uint32_t)0xFF80FFFF;                        /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CIR = 0x009F0000;                                    /* Disable all interrupts and clear pending bits  */
  SetSysClock();                                            /* Configure System clocks, enable prefetch buffer */
  SystemCoreClockUpdate();
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET;                  /* Vector Table Relocation in Internal SRAM. */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;                 /* Vector Table Relocation in Internal FLASH. */
#endif 
}

/**
  * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *           
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.         
  *     
  * @note   - The system frequency computed by this function is not the real 
  *           frequency in the chip. It is calculated based on the predefined 
  *           constant and the selected clock source:
  *             
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *                                              
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *                          
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**) 
  *             or HSI_VALUE(*) multiplied by the PLL factors.
  *         
  *         (*) HSI_VALUE is a constant defined in air32f1xx.h file (default value
  *             8 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.   
  *    
  *         (**) HSE_VALUE is a constant defined in air32f1xx.h file (default value
  *              8 MHz or 25 MHz, depedning on the product used), user has to ensure
  *              that HSE_VALUE is same as the real frequency of the crystal used.
  *              Otherwise, this function may have wrong result.
  *                
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate (void)
{
  uint32_t tmp = 0, pllmull = 0, pllsource = 0;
    
  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;
  
  switch (tmp)
  {
    case 0x00:  /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
    case 0x04:  /* HSE used as system clock */
      SystemCoreClock = HSE_VALUE;
      break;
    case 0x08:  /* PLL used as system clock */
      /* Get PLL clock source and multiplication factor ----------------------*/
      pllmull = (RCC->CFGR & RCC_CFGR_PLLMULL) >> 18;
      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;

      if (RCC->CFGR & 1<<28)                               /* High speed PLL bit */
          pllmull += 17;
      else
          pllmull += 2;                                    /* Normal PLL */
      
      if (pllsource == 0x00){
        /* HSI oscillator clock divided by 2 selected as PLL clock entry */
        SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
      }
      else
      {
        /* HSE selected as PLL clock entry */
        if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) != (uint32_t)RESET)
        {/* HSE oscillator clock divided by 2 */
          SystemCoreClock = (HSE_VALUE >> 1) * pllmull;
        }
        else
        {
          SystemCoreClock = HSE_VALUE * pllmull;
        }

      }
      break;

    default:
      SystemCoreClock = HSI_VALUE;
      break;
  }
  
  /* Compute HCLK clock frequency ----------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;  
}

/**
  * @brief  Configures the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers.
  * @param  None
  * @retval None
  */
static void SetSysClock(void)
{
#ifdef SYSCLK_FREQ_HSE
    SetSysClockToHSE();
#endif
 
 /* If none of the define above is enabled, the HSI is used as System clock
    source (default after reset) */ 
}
#ifdef SYSCLK_FREQ_HSE
static void SetSysClockToHSE(void)
{
    __IO uint32_t StartUpCounter = 0;
    uint32_t PLL_M = SYSCLK_HSE/HSE_VALUE;                                     /* Calculate multiplier  */

    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/

    RCC->CR |= ((uint32_t)RCC_CR_HSEON);                                        /* Enable HSE */
    while((RCC->CR&RCC_CR_HSERDY) == 0){                                        /* Wait till HSE is ready */
        if(++StartUpCounter > HSE_STARTUP_TIMEOUT)
            clockFailure();                                                     /* Check timeout */
    }

    FLASH->ACR |= FLASH_ACR_PRFTBE;                                             /* Enable Prefetch Buffer */
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);                     /* Reset Flash wait state */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;                              /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;                             /* PCLK2 = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;                             /* PCLK1 = HCLK */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

    if(PLL_M < 2){                                                              /* No multiplier, use HSE */
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSE;                                 /* Select HSE as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x04)          /* Wait till HSE is used as system clock source */
        {
        }
    }
    else{                                                                       /* If multiplier defined */
        uint8_t flash_div;

        if(SYSCLK_HSE <= CLK_MHz(106))
            flash_div = FLASH_Div_0;                                            /* 0 wait states up to 106MHz */
        else
            flash_div = FLASH_Div_2;                                            /* 1 wait states over 106MHz */

        if(PLL_M > 32)
            PLL_M = 32;                                                         /* Check limits */

        if(PLL_M<17)
            PLL_M = (PLL_M-2)<<18;                                              /* Low speed PLL setting*/
        else
            PLL_M = ((PLL_M-17) | 1<<10 )<<18;                                  /* High speed PLL setting*/

        AIR_SysFreq_Set(PLL_M, flash_div, 0, 1);                                /* Internal ROM hack to enable fast PLL */
     
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                                                RCC_CFGR_PLLMULL));             /* PLL configuration */
        RCC->CFGR |= RCC_CFGR_PLLSRC_HSE | PLL_M;
        RCC->CR |= RCC_CR_PLLON;                                                /* Enable PLL */
        while((RCC->CR & RCC_CR_PLLRDY) == 0)                                   /* Wait till PLL is ready */
        {
        }
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;                                 /* Select PLL as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)          /* Wait till PLL is used as system clock source */
        {
        }
    }
}
#endif
/**
  * @}
  */

/**
  * @}
  */
  
/**
  * @}
  */
