/***************************************************************************//**
 * @file        cs32f10x_it.c
 * @version     V2.0.1
 * @author      Software Development
 * @brief       This file provides all the interrupt handle functions.
 *              
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/

#include "cs32f10x_it.h"
#include "cs32f10x_tim.h"
#include "cs32f10x_usart.h"
#include "cs32f10x_exti.h"
#include "queue.h"
#include "task.h"
#include "led.h"
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**@brief       This function handles NMI exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void NMI_Handler(void)
{
    /* User code */
}

/**@brief       This function handles Hard Fault exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void HardFault_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles Memory Manage exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void MemManage_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles Bus Fault exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void BusFault_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles Usage Fault exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void UsageFault_Handler(void)
{
  while (1)
  {
      /* User code */
  }
}

/**@brief       This function handles SVCall exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void SVC_Handler(void)
{
    /* User code */
}

/**@brief       This function handles Debug Monitor exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void DebugMon_Handler(void)
{
    /* User code */
}

/**@brief       This function handles PendSVC exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void PendSV_Handler(void)
{
    /* User code */
}

/**@brief       This function handles SysTick exception.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void SysTick_Handler(void)
{
    /* User code */
    Task_Marks_Handler_Callback();
}

/**
 * @brief  This function handles USART3 interrupt request.
 * @param  None
 * @retval None
 */
void USART3_IRQHandler(void)
{
    if (__USART_INTF_STATUS_GET(USART3, RXNE) == SET)
    {

        circ_buffer_push(&rxBuffer, (uint8_t)__USART_DATA_RECV(USART3));
    }
}

/**@brief       This function handles TIM1 interrupt request.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void TIM1_UP_IRQHandler(void)
{
   // printf("TIM1 delay 1s\r\n");

   led_pwm_control(&led);//TODO
    __TIM_FLAG_CLEAR(TIM1, TIM_FLAG_UPDATE);
}

/**@brief       This function handles exti interrupt request.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void EXTI4_IRQHandler(void)
{
    if (__EXTI_FLAG_STATUS_GET(EXTI_LINE_4) != RESET)
    {
        __EXTI_FLAG_CLEAR(EXTI_LINE_4);
        if(sys.port.a_exit == 0)
        {
            sys.port.a_exit = 1;
            //printf("exit_irq");
        }
    }
}

/******************************************************************************/
/*                 CS32F10x Peripherals Interrupt Handlers                   */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2021 ChipSea *****END OF FILE****/
