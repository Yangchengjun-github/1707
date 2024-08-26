/***************************************************************************//**
 * @file        cs32f10x_conf.h
 * @version     V2.0.1
 * @author      Software Development
 * @brief       This file contains all the functions prototypes for the RCU firmware library.
 *              
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/


/* Define to prevent recursive inclusion */
#ifndef __CS32F10x_CONF_H__
#define __CS32F10x_CONF_H__

/* Run Time Environment will set specific #define for each selected module below */
#include "RTE_Components.h"


/************************************* Assert Selection **************************************/
/**
  * @brief Uncomment the line below to expanse the "ASSERT" macro in the 
  *        HAL drivers code
  */

/* #define USE_FULL_ASSERT 1 */


#ifdef  USE_FULL_ASSERT

/**@brief       The ASSERT macro is used for function's parameters check.
 *
 * @param[in]   expr: expr: If expr is false, it calls assert_failed function which reports
 *              the name of the source file and the source line number of the call
 *              that failed. If expr is true, it returns no value.
 *
 * @return      None
 */
/* Exported functions ------------------------------------------------------- */
void assert_failed(uint8_t* file, uint32_t line);
#define ASSERT(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

  
#else

  #define ASSERT(expr) ((void)0)
  
#endif 

#endif /* end of group __CS32F10x_CONF_H__ */

/*** (C) COPYRIGHT 2021 Software Development . ***/

