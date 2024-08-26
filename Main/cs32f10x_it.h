/***************************************************************************//**
 * @file        cs32f10x_it.h
 * @version     V2.0.1
 * @author      Software Development
 * @brief       This file provides all the interrupt handle functions.
 *              
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CS32F10X_IT_H__
#define __CS32F10X_IT_H__

#ifdef __cplusplus
 extern "C" {
#endif 


#include "cs32f10x.h"


void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2021 ChipSea *****END OF FILE****/
