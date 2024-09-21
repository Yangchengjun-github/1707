#ifndef _INIT_H
#define _INIT_H



/*****************************************************************************
 * @includes 
*****************************************************************************/
#include "cs32f10x_rcu.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_usart.h"
#include <stdio.h>
//#include <string.h>


#ifdef __cplusplus
 extern "C" {
#endif






#define EN_USBA_PORT GPIOA 
#define EN_USBA_PIN  GPIO_PIN_5

#define EN_DC_PORT GPIOA
#define EN_DC_PIN  GPIO_PIN_6

#define EN_G020_PORT GPIOB
#define EN_G020_PIN  GPIO_PIN_1

#define EN_ETA_PORT GPIOB
#define EN_ETA_PIN  GPIO_PIN_9

#define CFETOFF_PORT GPIOB
#define CFETOFF_PIN  GPIO_PIN_3

#define DFETOFF_PORT GPIOA
#define DFETOFF_PIN  GPIO_PIN_15

#define SCL_B //BMS iic
#define SDA_B //BMS iic
#define INT_B_PORT GPIOB
#define INT_B_PIN  GPIO_PIN_4

#define RST_SHUT_PORT GPIOB
#define RST_SHUT_PIN GPIO_PIN_5

#define WAKE_A_PORT GPIOA
#define WAKE_A_PIN  GPIO_PIN_4

#define PDA

#define IS_PORTA_PLUG    __GPIO_INPUT_PIN_GET(WAKE_A_PORT, WAKE_A_PIN)
#define USBA_ON          __GPIO_PIN_SET(EN_USBA_PORT, EN_USBA_PIN)
#define USBA_OFF         __GPIO_PIN_RESET(EN_USBA_PORT, EN_USBA_PIN)
#define DCDC_ON          __GPIO_PIN_SET(EN_DC_PORT, EN_DC_PIN)
#define DCDC_OFF         __GPIO_PIN_RESET(EN_DC_PORT, EN_DC_PIN)
#define G020_ON          __GPIO_PIN_SET(EN_G020_PORT, EN_G020_PIN)
#define G020_OFF         __GPIO_PIN_RESET(EN_G020_PORT, EN_G020_PIN)
     void log_init(void);

     void uart_init(void);

     void other_io_init(void);

     void io_sleep_conf(void);

     void tick_init(void);

     void nvic_configuration(void);

     void tim_init(void);

     void exti_init(void);

     void fwdt_init(void);

     void rtc_config(void);

     void deinit_befor_sleep(void);

     void init_after_sleep(void);

#ifdef __cplusplus
}
#endif


#endif

