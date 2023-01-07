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

  static const uint32_t multi[] = {
          RCC_CFGR_PLLMULL2,  RCC_CFGR_PLLMULL3,  RCC_CFGR_PLLMULL4,  RCC_CFGR_PLLMULL5,  RCC_CFGR_PLLMULL6,  RCC_CFGR_PLLMULL7,
          RCC_CFGR_PLLMULL8,  RCC_CFGR_PLLMULL9,  RCC_CFGR_PLLMULL10, RCC_CFGR_PLLMULL11, RCC_CFGR_PLLMULL12, RCC_CFGR_PLLMULL13,
          RCC_CFGR_PLLMULL14, RCC_CFGR_PLLMULL15, RCC_CFGR_PLLMULL16, RCC_CFGR_PLLMULL17, RCC_CFGR_PLLMULL18, RCC_CFGR_PLLMULL19,
          RCC_CFGR_PLLMULL20, RCC_CFGR_PLLMULL21, RCC_CFGR_PLLMULL22, RCC_CFGR_PLLMULL23, RCC_CFGR_PLLMULL24, RCC_CFGR_PLLMULL25,
          RCC_CFGR_PLLMULL26, RCC_CFGR_PLLMULL27, RCC_CFGR_PLLMULL28, RCC_CFGR_PLLMULL29, RCC_CFGR_PLLMULL30, RCC_CFGR_PLLMULL31,
          RCC_CFGR_PLLMULL32,
  };
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
static void SetSysClockToHSE(void);

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
      pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;

      if (RCC->CFGR >> 28 & 0x01)
      {
        pllmull = (0x01 << 4 | (pllmull >> 18)) + 1;
      }
      else
      {
        pllmull = (pllmull >> 18) + 2;
      }
      
      if (pllsource == 0x00)
      {
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
    uint8_t high_speed = SYSCLK_HSE > CLK_MHz(106);

    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/

    RCC->CR |= ((uint32_t)RCC_CR_HSEON);                                        /* Enable HSE */
    while((RCC->CR&RCC_CR_HSERDY) == 0){                                        /* Wait till HSE is ready */
        if(++StartUpCounter > HSE_STARTUP_TIMEOUT)
            clockFailure();                                                     /* Check timeout */
    }

    FLASH->ACR |= FLASH_ACR_PRFTBE;                                             /* Enable Prefetch Buffer */
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);                     /* Reset Flash wait state */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;                                  /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;                                 /* PCLK2 = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;                                 /* PCLK1 = HCLK */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));

    if(PLL_M < 2){                                                              /* No multiplier, use HSE */
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSE;                                 /* Select HSE as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x04)          /* Wait till HSE is used as system clock source */
        {
        }
    }
    else{                                                                       /* If multiplier defined */
        if(PLL_M > 31)
            PLL_M = 31;                                                         /* Check limits. Real value is multiplier+1 */
        if(high_speed){                                                         /* 0 wait states up to 106MHz */
            FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_1;                        /* 1 wait states >106MHz */
            AIR_SysFreq_Set(multi[PLL_M], FLASH_Div_2, 0, 1);                   /* Set PLL */
        }
        else
            AIR_SysFreq_Set(multi[PLL_M], FLASH_Div_0, 0, 1);

        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                                            RCC_CFGR_PLLMULL));
        RCC->CFGR |= RCC_CFGR_PLLSRC_HSE | multi[PLL_M];
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
