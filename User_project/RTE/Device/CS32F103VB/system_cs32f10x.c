/***************************************************************************//**
 * @file        system_cs32f10x.c
 * @version     V2.0.1
 * @author      Software Development
 * @brief       This file contains system clock init functions.
 *              
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/


/*****************************************************************************
 * @includes 
*****************************************************************************/

#include "cs32f10x.h"
#include "cs32f10x_rcu.h"

/****************************************************************************
 * @definitions 
*****************************************************************************/

/**
 * @brief User choose specified value for system clock
 */
//#define SYSCLK_FREQ_HXT    HXT_VALUE 
//#define SYSCLK_FREQ_24MHz  24000000  
//#define SYSCLK_FREQ_36MHz  36000000 
//#define SYSCLK_FREQ_48MHz  48000000 
//#define SYSCLK_FREQ_56MHz  56000000 
#define SYSCLK_FREQ_72MHz  72000000

//#define VECT_TAB_SRAM
#define VECT_TAB_OFFSET  0x0 /*!< Vector Table base offset field. 
                                  This value must be a multiple of 0x200. */

/*****************************************************************************
 * @variables
*****************************************************************************/

/** @addtogroup System_Global_Variables
  * @{
  */

#ifdef SYSCLK_FREQ_HXT
  uint32_t SystemCoreClock         = SYSCLK_FREQ_HXT;
#elif defined SYSCLK_FREQ_24MHz
  uint32_t SystemCoreClock         = SYSCLK_FREQ_24MHz;
#elif defined SYSCLK_FREQ_36MHz
  uint32_t SystemCoreClock         = SYSCLK_FREQ_36MHz;
#elif defined SYSCLK_FREQ_48MHz
  uint32_t SystemCoreClock         = SYSCLK_FREQ_48MHz;
#elif defined SYSCLK_FREQ_56MHz
  uint32_t SystemCoreClock         = SYSCLK_FREQ_56MHz;
#elif defined SYSCLK_FREQ_72MHz
  uint32_t SystemCoreClock         = SYSCLK_FREQ_72MHz;
#else
  uint32_t SystemCoreClock         = HRC_VALUE;
#endif

static uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
/**
  * @}
  */


/*****************************************************************************
 * @functions
*****************************************************************************/


/** @addtogroup System_FunctionPrototypes
  * @{
  */

static void SetSysClock(void);

#ifdef SYSCLK_FREQ_HXT
  static void set_sysclk_hxt(void);
#elif defined SYSCLK_FREQ_24MHz
  static void set_sysclk_freq_24M(void);
#elif defined SYSCLK_FREQ_36MHz
  static void set_sysclk_freq_36M(void);
#elif defined SYSCLK_FREQ_48MHz
  static void set_sysclk_freq_48M(void);
#elif defined SYSCLK_FREQ_56MHz
  static void set_sysclk_freq_56M(void);  
#elif defined SYSCLK_FREQ_72MHz
  static void set_sysclk_freq_72M(void);
#endif


/**
  * @}
  */

/** @addtogroup System_Private_Functions
  * @{
  */


/**@brief       Initialize the global clock configuration. Including FLASH and PLL.
 *              This function should be called only after reset.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void SystemInit (void)
{
    /* Set HRCEN bit */
    RCU->CTR |= (uint32_t)0x00000001;

    /* Reset SYSSW, HCLKPDIV, PCLKPDIV, HPCLKPDIV, ADCPDIV and CKOSEL bits */
    RCU->CFG &= (uint32_t)0xF8FF0000;

    /* Reset HXTEN, HXTME and PLLEN bits */
    RCU->CTR &= (uint32_t)0xFEF6FFFF;

    /* Reset HXTBPS bit */
    RCU->CTR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSEL, PLLHXTPDIV, PLLMUF and USBPDIV bits */
    RCU->CFG &= (uint32_t)0xFF80FFFF;

    /* Disable all interrupts and clear pending bits  */
    RCU->INTR = 0x009F0000;

    SetSysClock();
    
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif 
}

/**@brief       Update SystemCoreClock according to Clock Register Values
 *              The SystemCoreClock variable contains the core clock (HCLK), it can
 *              be used by the user application to setup the SysTick timer or configure
 *              other parameters.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void SystemCoreClockUpdate (void)
{
    uint32_t tmp = 0, pllmull = 0, pllsource = 0;

    /* Get SYSCLK source */
    tmp = RCU->CFG & RCU_CFG_SYSSS;

    switch (tmp)
    {
        case 0x00:                          /* HRC used as system clock */
            SystemCoreClock = HRC_VALUE;
        break;
        case 0x04:                          /* HXT used as system clock */
            SystemCoreClock = HXT_VALUE;
        break;
        case 0x08:                          /* PLL used as system clock */
            /* Get PLL clock source and multiplication factor */
            pllmull = RCU->CFG & RCU_CFG_PLLMUF;
            pllsource = RCU->CFG & RCU_CFG_PLLSEL;

            pllmull = ( pllmull >> 18) + 2;

            if (pllsource == 0x00)
            {
                /* HRC oscillator clock divided by 2 selected as PLL clock entry */
                SystemCoreClock = (HRC_VALUE >> 1) * pllmull;
            }
            else
            {
                /* HXT selected as PLL clock entry */
                if ((RCU->CFG & RCU_CFG_PLLHXTPDIV) != (uint32_t)RESET)
                {
                    /* HXT oscillator clock divided by 2 */
                    SystemCoreClock = (HXT_VALUE >> 1) * pllmull;
                }
                else
                {
                    SystemCoreClock = HXT_VALUE * pllmull;
                }
            }
        break;

        default:
            SystemCoreClock = HRC_VALUE;
        break;
    }
  
    /* Compute HCLK clock frequency */
    /* Get HCLK pre-divider */
    tmp = AHBPrescTable[((RCU->CFG & RCU_CFG_HCLKPDIV) >> 4)];
    /* HCLK clock frequency */
    SystemCoreClock >>= tmp;  
}

/**@brief       Configures the System clock frequency, HCLK, PCLK and HPCLK prescalers.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void SetSysClock(void)
{
#ifdef SYSCLK_FREQ_HXT
  set_sysclk_hxt();
#elif defined SYSCLK_FREQ_24MHz
  set_sysclk_freq_24M();
#elif defined SYSCLK_FREQ_36MHz
  set_sysclk_freq_36M();
#elif defined SYSCLK_FREQ_48MHz
  set_sysclk_freq_48M();
#elif defined SYSCLK_FREQ_56MHz
  set_sysclk_freq_56M();  
#elif defined SYSCLK_FREQ_72MHz
  set_sysclk_freq_72M();
#endif
}

#ifdef SYSCLK_FREQ_HXT
/**@brief       Configures HXT as System clock frequency and configure HCLK, HPCLK,PCLK pre-divider.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void set_sysclk_hxt(void)
{
  __IO uint32_t startup_counter = 0, HXT_Status = 0;
  
  /* Enable HXT */    
  RCU->CTR |= ((uint32_t)RCU_CTR_HXTEN);
 
  /* Wait till HXT is ready and if Time out is reached exit */
  do
  {
    HXT_Status = RCU->CTR & RCU_CTR_HXTSTAB;
    startup_counter++;  
  } while((HXT_Status == 0) && (startup_counter != HXT_STARTUP_TIMEOUT));

  if ((RCU->CTR & RCU_CTR_HXTSTAB) != RESET)
  {
    HXT_Status = (uint32_t)0x01;
  }
  else
  {
    HXT_Status = (uint32_t)0x00;
  }  

  if (HXT_Status == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->WCR |= FMC_WCR_WE;

    /* Flash 0 wait state */
    FLASH->WCR &= (uint32_t)((uint32_t)~FMC_WCR_WCNT);

    FLASH->WCR |= (uint32_t)FMC_WCR_WCNT_0;
 
    /* HCLK = SYSCLK */
    RCU->CFG |= (uint32_t)RCU_SYSCLK_DIV1;
      
    /* PCLK2 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* PCLK1 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* Select HXT as system clock source */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_SYSSW));
    RCU->CFG |= (uint32_t)RCU_SYSCLK_SEL_HXT;    

    /* Wait till HXT is used as system clock source */
    while ((RCU->CFG & (uint32_t)RCU_CFG_SYSSS) != (uint32_t)0x04)
    {
    }
  }
  else
  { /* If HXT fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
  }  
}
#elif defined SYSCLK_FREQ_24MHz
/**@brief       Configures System clock frequency to 24MHz and configure HCLK, HPCLK,PCLK pre-divider.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void set_sysclk_freq_24M(void)
{
  __IO uint32_t startup_counter = 0, HXT_Status = 0;
  
  /* Enable HXT */    
  RCU->CTR |= ((uint32_t)RCU_CTR_HXTEN);
 
  /* Wait till HXT is ready and if Time out is reached exit */
  do
  {
    HXT_Status = RCU->CTR & RCU_CTR_HXTSTAB;
    startup_counter++;  
  } while((HXT_Status == 0) && (startup_counter != HXT_STARTUP_TIMEOUT));

  if ((RCU->CTR & RCU_CTR_HXTSTAB) != RESET)
  {
    HXT_Status = (uint32_t)0x01;
  }
  else
  {
    HXT_Status = (uint32_t)0x00;
  }  

  if (HXT_Status == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->WCR |= FMC_WCR_WE;

    /* Flash 0 wait state */
    FLASH->WCR &= (uint32_t)((uint32_t)~FMC_WCR_WCNT);
    FLASH->WCR |= (uint32_t)FMC_WCR_WCNT_0;    
 
    /* HCLK = SYSCLK */
    RCU->CFG |= (uint32_t)RCU_SYSCLK_DIV1;
      
    /* PCLK2 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* PCLK1 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;

    /*  PLL = (HXT / 2) * 6 = 24 MHz */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_PLLSEL | RCU_CFG_PLLHXTPDIV | RCU_CFG_PLLMUF));
    RCU->CFG |= (uint32_t)(RCU_PLL_SOURCE_HXT_DIV2 | RCU_PLL_MULTI_6);

    /* Enable PLL */
    RCU->CTR |= RCU_CTR_PLLEN;

    /* Wait till PLL is ready */
    while((RCU->CTR & RCU_CTR_PLLSTAB) == 0)
    {
    }

    /* Select PLL as system clock source */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_SYSSW));
    RCU->CFG |= (uint32_t)RCU_SYSCLK_SEL_PLL;

    /* Wait till PLL is used as system clock source */
    while ((RCU->CFG & (uint32_t)RCU_CFG_SYSSS) != (uint32_t)0x08)
    {
    }
  }
  else
  { /* If HXT fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
  } 
}
#elif defined SYSCLK_FREQ_36MHz
/**@brief       Configures System clock frequency to 36MHz and configure HCLK, HPCLK,PCLK pre-divider.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void set_sysclk_freq_36M(void)
{
  __IO uint32_t startup_counter = 0, HXT_Status = 0;
  
  /* Enable HXT */    
  RCU->CTR |= ((uint32_t)RCU_CTR_HXTEN);
 
  /* Wait till HXT is ready and if Time out is reached exit */
  do
  {
    HXT_Status = RCU->CTR & RCU_CTR_HXTSTAB;
    startup_counter++;  
  } while((HXT_Status == 0) && (startup_counter != HXT_STARTUP_TIMEOUT));

  if ((RCU->CTR & RCU_CTR_HXTSTAB) != RESET)
  {
    HXT_Status = (uint32_t)0x01;
  }
  else
  {
    HXT_Status = (uint32_t)0x00;
  }  

  if (HXT_Status == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->WCR |= FMC_WCR_WE;

    /* Flash 1 wait state */
    FLASH->WCR &= (uint32_t)((uint32_t)~FMC_WCR_WCNT);
    FLASH->WCR |= (uint32_t)FMC_WCR_WCNT_1;    
 
    /* HCLK = SYSCLK */
    RCU->CFG |= (uint32_t)RCU_SYSCLK_DIV1;
      
    /* PCLK2 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* PCLK1 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;

    /* PLLCLK = (HXT / 2) * 9 = 36 MHz */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_PLLSEL | RCU_CFG_PLLHXTPDIV | RCU_CFG_PLLMUF));
    RCU->CFG |= (uint32_t)(RCU_PLL_SOURCE_HXT_DIV2 | RCU_PLL_MULTI_9);

    /* Enable PLL */
    RCU->CTR |= RCU_CTR_PLLEN;

    /* Wait till PLL is ready */
    while((RCU->CTR & RCU_CTR_PLLSTAB) == 0)
    {
    }

    /* Select PLL as system clock source */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_SYSSW));
    RCU->CFG |= (uint32_t)RCU_SYSCLK_SEL_PLL;    

    /* Wait till PLL is used as system clock source */
    while ((RCU->CFG & (uint32_t)RCU_CFG_SYSSS) != (uint32_t)0x08)
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
  } 
}
#elif defined SYSCLK_FREQ_48MHz
/**@brief       Configures System clock frequency to 48MHz and configure HCLK, HPCLK,PCLK pre-divider.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void set_sysclk_freq_48M(void)
{
  __IO uint32_t startup_counter = 0, HXT_Status = 0;
  
  /* Enable HXT */    
  RCU->CTR |= ((uint32_t)RCU_CTR_HXTEN);
 
  /* Wait till HXT is ready and if Time out is reached exit */
  do
  {
    HXT_Status = RCU->CTR & RCU_CTR_HXTSTAB;
    startup_counter++;  
  } while((HXT_Status == 0) && (startup_counter != HXT_STARTUP_TIMEOUT));

  if ((RCU->CTR & RCU_CTR_HXTSTAB) != RESET)
  {
    HXT_Status = (uint32_t)0x01;
  }
  else
  {
    HXT_Status = (uint32_t)0x00;
  }  

  if (HXT_Status == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->WCR |= FMC_WCR_WE;

    /* Flash 1 wait state */
    FLASH->WCR &= (uint32_t)((uint32_t)~FMC_WCR_WCNT);
    FLASH->WCR |= (uint32_t)FMC_WCR_WCNT_1;    
 
    /* HCLK = SYSCLK */
    RCU->CFG |= (uint32_t)RCU_SYSCLK_DIV1;
      
    /* PCLK2 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* PCLK1 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_2;

    /* PLLCLK = HXT * 6 = 48 MHz */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_PLLSEL | RCU_CFG_PLLHXTPDIV | RCU_CFG_PLLMUF));
    RCU->CFG |= (uint32_t)(RCU_PLL_SOURCE_HXT_DIV1 | RCU_PLL_MULTI_6);

    /* Enable PLL */
    RCU->CTR |= RCU_CTR_PLLEN;

    /* Wait till PLL is ready */
    while((RCU->CTR & RCU_CTR_PLLSTAB) == 0)
    {
    }

    /* Select PLL as system clock source */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_SYSSW));
    RCU->CFG |= (uint32_t)RCU_SYSCLK_SEL_PLL;    

    /* Wait till PLL is used as system clock source */
    while ((RCU->CFG & (uint32_t)RCU_CFG_SYSSS) != (uint32_t)0x08)
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
  } 
}

#elif defined SYSCLK_FREQ_56MHz
/**@brief       Configures System clock frequency to 56MHz and configure HCLK, HPCLK,PCLK pre-divider.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void set_sysclk_freq_56M(void)
{
  __IO uint32_t startup_counter = 0, HXT_Status = 0;
  
  /* Enable HXT */    
  RCU->CTR |= ((uint32_t)RCU_CTR_HXTEN);
 
  /* Wait till HXT is ready and if Time out is reached exit */
  do
  {
    HXT_Status = RCU->CTR & RCU_CTR_HXTSTAB;
    startup_counter++;  
  } while((HXT_Status == 0) && (startup_counter != HXT_STARTUP_TIMEOUT));

  if ((RCU->CTR & RCU_CTR_HXTSTAB) != RESET)
  {
    HXT_Status = (uint32_t)0x01;
  }
  else
  {
    HXT_Status = (uint32_t)0x00;
  }  

  if (HXT_Status == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->WCR |= FMC_WCR_WE;

    /* Flash 2 wait state */
    FLASH->WCR &= (uint32_t)((uint32_t)~FMC_WCR_WCNT);
    FLASH->WCR |= (uint32_t)FMC_WCR_WCNT_2;    
 
    /* HCLK = SYSCLK */
    RCU->CFG |= (uint32_t)RCU_SYSCLK_DIV1;
      
    /* PCLK2 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* PCLK1 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_2;

    /* PLLCLK = HXT * 7 = 56 MHz */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_PLLSEL | RCU_CFG_PLLHXTPDIV | RCU_CFG_PLLMUF));
    RCU->CFG |= (uint32_t)(RCU_PLL_SOURCE_HXT_DIV1 | RCU_PLL_MULTI_7);

    /* Enable PLL */
    RCU->CTR |= RCU_CTR_PLLEN;

    /* Wait till PLL is ready */
    while((RCU->CTR & RCU_CTR_PLLSTAB) == 0)
    {
    }

    /* Select PLL as system clock source */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_SYSSW));
    RCU->CFG |= (uint32_t)RCU_SYSCLK_SEL_PLL;    

    /* Wait till PLL is used as system clock source */
    while ((RCU->CFG & (uint32_t)RCU_CFG_SYSSS) != (uint32_t)0x08)
    {
    }
  }
  else
  { /* If HXT fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
  } 
}

#elif defined SYSCLK_FREQ_72MHz
/**@brief       Configures System clock frequency to 72MHz and configure HCLK, HPCLK,PCLK pre-divider.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
static void set_sysclk_freq_72M(void)
{
  __IO uint32_t startup_counter = 0, HXT_Status = 0;
  
  /* Enable HXT */    
  RCU->CTR |= ((uint32_t)RCU_CTR_HXTEN);
 
  /* Wait till HXT is ready and if Time out is reached exit */
  do
  {
    HXT_Status = RCU->CTR & RCU_CTR_HXTSTAB;
    startup_counter++;  
  } while((HXT_Status == 0) && (startup_counter != HXT_STARTUP_TIMEOUT));

  if ((RCU->CTR & RCU_CTR_HXTSTAB) != RESET)
  {
    HXT_Status = (uint32_t)0x01;
  }
  else
  {
    HXT_Status = (uint32_t)0x00;
  }  

  if (HXT_Status == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->WCR |= FMC_WCR_WE;

    /* Flash 2 wait state */
    FLASH->WCR &= (uint32_t)((uint32_t)~FMC_WCR_WCNT);
    FLASH->WCR |= (uint32_t)FMC_WCR_WCNT_2;    

 
    /* HCLK = SYSCLK */
    RCU->CFG |= (uint32_t)RCU_SYSCLK_DIV1;
      
    /* PCLK2 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_1;
    
    /* PCLK1 = HCLK */
    RCU->CFG |= (uint32_t)RCU_HCLK_DIV_2;

    /* PLLCLK = HXT * 9 = 72 MHz */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_PLLSEL | RCU_CFG_PLLHXTPDIV |RCU_CFG_PLLMUF));
    RCU->CFG |= (uint32_t)(RCU_PLL_SOURCE_HXT_DIV1 | RCU_PLL_MULTI_9);

    /* Enable PLL */
    RCU->CTR |= RCU_CTR_PLLEN;

    /* Wait till PLL is ready */
    while((RCU->CTR & RCU_CTR_PLLSTAB) == 0)
    {
    }
    
    /* Select PLL as system clock source */
    RCU->CFG &= (uint32_t)((uint32_t)~(RCU_CFG_SYSSW));
    RCU->CFG |= (uint32_t)RCU_SYSCLK_SEL_PLL;    

    /* Wait till PLL is used as system clock source */
    while ((RCU->CFG & (uint32_t)RCU_CFG_SYSSS) != (uint32_t)0x08)
    {
    }
  }
  else
  { /* If HXT fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */
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
/******************* (C) COPYRIGHT 2021 ChipSea *****END OF FILE****/
