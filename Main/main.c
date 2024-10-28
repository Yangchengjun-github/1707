/***************************************************************************/ /**
                                                                               * @file        GPIO/GPIO_TOGGLE/main.c
                                                                               * @version     V2.0.1
                                                                               * @author      Software Development
                                                                               * @brief       Main program body.
                                                                               * @copyright   Copyright (C) Software Development. All rights reserved.
                                                                               ****************************************************************************/

#include "adc.h"
#include "app.h"
#include "bathealth.h"
#include "bms.h"
#include "coulomp.h"
#include "cs32f10x_exti.h"
#include "cs32f10x_fwdt.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_pmu.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_rtc.h"
#include "iic.h"
#include "init.h"
#include "key.h"
#include "led.h"
#include "task.h"
#include "define.h"
#define LED_DELAY 0x8FFFF

void nvic_config(void);
void led_init(void);
void led1_toggle(void);
void led2_toggle(void);
void delay(__IO uint32_t count);

__IO uint8_t second_flag = 0;

/**@brief       Before enter main function,the chip clock setted already in function SystemInit
 *              User could release clock frequency definition to modify frequency.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
extern uint32_t cnt_value;
int main(void)
{   
    tick_init(); // 任务调度用

    rcu_clock_t clock = {0};
    // io_sleep_conf(STATE_ON);
    // pmu_stop_mode_enter(PMU_LDO_ON, PMU_DSM_ENTRY_WFI);
    rcu_clk_freq_get(&clock);

    tick_delay(1000);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
       __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    gpio_pin_remap_config(GPIO_REMAP_SWJ_DISABLE, ENABLE); //复用 用于bms控制
    gpio_mode_config(CFETOFF_PORT, CFETOFF_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH)); // --io
    __GPIO_PIN_RESET(CFETOFF_PORT, CFETOFF_PIN);
    gpio_mode_config(DFETOFF_PORT, DFETOFF_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH)); // --io
    __GPIO_PIN_RESET(DFETOFF_PORT, DFETOFF_PIN);


    uart_init(); // 通讯用 usart3
    log_init();  // DEBUG用 usart2
    printf("sys reset\n");

    key_init();

    adc_init_(1);

    tim_init(); // PWM呼吸用
    i2c_init_2();
    other_io_init();

#if (BME_EN)
    // printf("sysclk_freq = %d, hclk_freq = %d, pclk1_freq = %d pclk2_freq = %d adc_clk_freq = %d \r\n",
    //  clock.sysclk_freq,clock.hclk_freq,clock.pclk1_freq,clock.pclk2_freq,clock.adc_clk_freq);
    bq76942_reset(); // afe
    tick_delay(1000); // 其他IO控制
    coulomp_init();   // 库仑计
#endif
    health_init(); // 电池健康
    led_init();    //
#if (WDG_EN)
    rtc_config();
    fwdt_init(); //看门狗
#endif
#if ACTIVE_NEED
    sys.flag.bms_active = 0;
#else
    sys.flag.bms_active = 1;
#endif
        //         // TEST
        //         // led.port.status = WARNING;
        //         // sys.port.method.usbaClose();
        //         // sys.port.method.usbaOpen();
        sys.eta_en = 0;
#if 1


    

#endif

    while (1)
    {
#if (WDG_EN)
        fwdt_reload_counter();
#endif
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
    for (; count != 0; count--)
        ;
}

#ifdef USE_FULL_ASSERT

/**@brief       Report the assert error.
 *
 * @param[in]   file: pointer to the source file name.
 *
 * @param[in]   line: error line source number.
 *
 * @return      None.
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1)
        ;
}

#endif
