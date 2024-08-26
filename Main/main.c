/***************************************************************************//**
 * @file        GPIO/GPIO_TOGGLE/main.c
 * @version     V2.0.1
 * @author      Software Development
 * @brief       Main program body.
 * @copyright   Copyright (C) Software Development. All rights reserved.
 ****************************************************************************/

#include "cs32f10x_rcu.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_exti.h"

#include "init.h"
#include "led.h"
#include "key.h"
#include "adc.h"
#include "task.h"
#include "app.h"
#include "iic.h"
#include "bms.h"


#define LED_DELAY 0x8FFFF


void nvic_config(void);
void led_init(void);
void led1_toggle(void);
void led2_toggle(void);
void delay(__IO uint32_t count);


/**@brief       Before enter main function,the chip clock setted already in function SystemInit
 *              User could release clock frequency definition to modify frequency.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
int main(void)
{
	delay(UINT32_MAX/5000);

    tick_init();  //任务调度用
    
    uart_init(); //通讯用 usart3
	log_init();   //DEBUG用 usart2
    key_init();
    adc_init_();
    tim_init();  //PWM呼吸用
	i2c_init_2();
    other_io_init();
    while(bms_init() == 0 || xbms.nack_cnt != 0)
    {
        xbms.nack_cnt = 0;
        xbms.ack_total = 0;
        printf("bms_init fail\n");
    };
    printf("bms_init OK\n");
     //其他IO控制
    led_init();       //
    exti_init(); // A口外部中断    
    nvic_configuration(); 
	
	gpio_pin_remap_config(GPIO_REMAP_SWJ_DISABLE,ENABLE);  //SWD---->GPIO
	 
	//TEST
    //led.port.status = WARNING;
	sys.port.method.usbaClose();
    sys.bat.cap = 5;
    sys.bat.health = 6;
    
	//TEST

    while(1)
    {
		
       Task_Pro_Handler_Callback();
    }
}



/**@brief       Toggle the LED.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void led1_toggle(void)
{
    GPIOB->DO ^= GPIO_PIN_7;
}

void led2_toggle(void)
{
    GPIOB->DO ^= GPIO_PIN_14;
}

/**@brief       Software delay.
 *
 * @param[in]   count: the delay time length
 *
 * @return      None.
 */
void delay(__IO uint32_t count)
{
  for(; count!= 0; count--);
}


#ifdef  USE_FULL_ASSERT

/**@brief       Report the assert error.
 *
 * @param[in]   file: pointer to the source file name.
 *
 * @param[in]   line: error line source number.
 *
 * @return      None.
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    while(1);
}

#endif


