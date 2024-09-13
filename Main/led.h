#ifndef _LED_H
#define _LED_H
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"

#define TIME_TASK_LED_CALL (100)


#define LED1_PORT GPIOA
#define LED1_PIN  GPIO_PIN_11
#define LED2_PORT GPIOA
#define LED2_PIN GPIO_PIN_10
#define LED3_PORT GPIOA
#define LED3_PIN GPIO_PIN_9
#define LED4_PORT GPIOA
#define LED4_PIN GPIO_PIN_8
#define LED5_PORT GPIOB
#define LED5_PIN GPIO_PIN_15
#define LED6_PORT GPIOB
#define LED6_PIN GPIO_PIN_14
#define LED7_PORT GPIOB
#define LED7_PIN GPIO_PIN_13
#define LED8_PORT GPIOB
#define LED8_PIN GPIO_PIN_12

// #define LED1_PORT GPIOB
// #define LED1_PIN GPIO_PIN_12
// #define LED2_PORT GPIOB
// #define LED2_PIN GPIO_PIN_13
// #define LED3_PORT GPIOB
// #define LED3_PIN GPIO_PIN_14
// #define LED4_PORT GPIOB
// #define LED4_PIN GPIO_PIN_15
// #define LED5_PORT GPIOA
// #define LED5_PIN GPIO_PIN_8
// #define LED6_PORT GPIOA
// #define LED6_PIN GPIO_PIN_9
// #define LED7_PORT GPIOA
// #define LED7_PIN GPIO_PIN_10
// #define LED8_PORT GPIOA
// #define LED8_PIN GPIO_PIN_11

#define LED_C1_PORT GPIOB
#define LED_C1_PIN GPIO_PIN_8
#define LED_C2_PORT GPIOA
#define LED_C2_PIN GPIO_PIN_14
#define LED_A1_PORT GPIOA
#define LED_A1_PIN GPIO_PIN_13

#define LED_PWM_PORT GPIOA
#define LED_PWM_PIN GPIO_PIN_12

#define LED_PWM_ON      __GPIO_PIN_SET(LED_PWM_PORT, LED_PWM_PIN)
#define LED_PWM_OFF     __GPIO_PIN_RESET(LED_PWM_PORT, LED_PWM_PIN)

#define LED1_ON         __GPIO_PIN_SET(LED1_PORT, LED1_PIN)
#define LED1_OFF        __GPIO_PIN_RESET(LED1_PORT, LED1_PIN)
#define LED2_ON         __GPIO_PIN_SET(LED2_PORT, LED2_PIN)
#define LED2_OFF        __GPIO_PIN_RESET(LED2_PORT, LED2_PIN)
#define LED3_ON         __GPIO_PIN_SET(LED3_PORT, LED3_PIN)
#define LED3_OFF        __GPIO_PIN_RESET(LED3_PORT, LED3_PIN)
#define LED4_ON         __GPIO_PIN_SET(LED4_PORT, LED4_PIN)
#define LED4_OFF        __GPIO_PIN_RESET(LED4_PORT, LED4_PIN)
#define LED5_ON         __GPIO_PIN_SET(LED5_PORT, LED5_PIN)
#define LED5_OFF        __GPIO_PIN_RESET(LED5_PORT, LED5_PIN)
#define LED6_ON         __GPIO_PIN_SET(LED6_PORT, LED6_PIN)
#define LED6_OFF        __GPIO_PIN_RESET(LED6_PORT, LED6_PIN)
#define LED7_ON         __GPIO_PIN_SET(LED7_PORT, LED7_PIN)
#define LED7_OFF        __GPIO_PIN_RESET(LED7_PORT, LED7_PIN)
#define LED8_ON         __GPIO_PIN_SET(LED8_PORT, LED8_PIN)
#define LED8_OFF        __GPIO_PIN_RESET(LED8_PORT, LED8_PIN)
    
#define LED_C1_ON     	__GPIO_PIN_SET(LED_C1_PORT, LED_C1_PIN)
#define LED_C1_OFF      __GPIO_PIN_RESET(LED_C1_PORT, LED_C1_PIN)
#define LED_C2_ON       __GPIO_PIN_SET(LED_C2_PORT, LED_C2_PIN)
#define LED_C2_OFF      __GPIO_PIN_RESET(LED_C2_PORT, LED_C2_PIN)
#define LED_A1_ON       __GPIO_PIN_SET(LED_A1_PORT, LED_A1_PIN)
#define LED_A1_OFF      __GPIO_PIN_RESET(LED_A1_PORT, LED_A1_PIN)


// #define LED_0_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF;LED8_OFF          //全关
// #define LED_1_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF;LED8_ON           //红灯闪
// #define LED_2_8  LED1_ON;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF; LED8_OFF          //
// #define LED_3_8  LED1_ON;LED2_ON;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF;LED8_OFF            //
// #define LED_4_8  LED1_ON;LED2_ON;LED3_ON;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF;LED8_OFF
// #define LED_5_8  LED1_ON;LED2_ON;LED3_ON;LED4_ON;LED5_OFF;LED6_OFF;LED7_OFF;LED8_OFF
// #define LED_6_8  LED1_ON;LED2_ON;LED3_ON;LED4_ON;LED5_ON;LED6_OFF;LED7_OFF;LED8_OFF
// #define LED_7_8  LED1_ON;LED2_ON;LED3_ON;LED4_ON;LED5_ON;LED6_ON;LED7_OFF;LED8_OFF
// #define LED_8_8  LED1_ON;LED2_ON;LED3_ON;LED4_ON;LED5_ON;LED6_ON;LED7_ON;LED8_OFF


#define LED_0_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF;LED8_OFF          //全关
#define LED_1_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_OFF;LED8_ON           //红灯闪
#define LED_2_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_OFF;LED7_ON;LED8_OFF         //
#define LED_3_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;LED6_ON;LED7_ON;LED8_OFF            //
#define LED_4_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_ON;LED6_ON;LED7_ON;LED8_OFF 
#define LED_5_8  LED1_OFF;LED2_OFF;LED3_OFF;LED4_ON;LED5_ON;LED6_ON;LED7_ON;LED8_OFF
#define LED_6_8  LED1_OFF;LED2_OFF;LED3_ON;LED4_ON;LED5_ON;LED6_ON;LED7_ON;LED8_OFF
#define LED_7_8  LED1_OFF;LED2_ON;LED3_ON;LED4_ON;LED5_ON;LED6_ON;LED7_ON;LED8_OFF
#define LED_8_8  LED1_ON;LED2_ON;LED3_ON;LED4_ON;LED5_ON;LED6_ON;LED7_ON;LED8_OFF

typedef void (*pfun)(void *arg);

typedef struct
{
    enum
    {
        breath_0,
        breath_normal,
        breath_100,
    } status;
    uint16_t cnt;
    uint16_t cnt2;
    uint16_t duty;
    uint16_t cycle;
    uint16_t preiod;
    uint8_t dir;
} breath_t;
typedef struct 
{
    struct 
    {
        struct
        {
            pfun pf_led_show_battery;
            pfun pf_led_discharge;
            pfun pf_led_charge;
            pfun pf_led_health;
			pfun pf_led_alloff;
            pfun pf_led_warning;
            pfun pf_led_err;
        } method;
        enum
        {
            LED_ALL_OFF = 0,  // 全灭
            LED_SHOW_BATTERY, // 开机显示,
            LED_DISCHARGE,    // 放电显示
            LED_CHARGE,       // 充电显示
            LED_HEALTH,       // 电池健康显示
            LED_WARNING,       //异常  
            LED_ERR,  //错误
        } status;

        breath_t breath;


        uint16_t timer;
        uint16_t timer1;
        uint16_t timer2;
        uint8_t run_cnt;
    }bat;
    struct
    {
        enum
        {
            
            NORMAL = 0,      // 正常显示
			WARNING , // 异常显示
        } status;
        uint16_t timer1;
        uint16_t timer2;
        uint8_t run_cnt;
		struct
        {
            pfun pf_led_normal;
            pfun pf_led_warning;
            
        } method;
    }port;
    uint16_t isr_int;
   

    
}led_t;

extern  led_t led;

void led_init(void);

void task_led(void);

void led_port_show(led_t *cb);

void led_bat_show(led_t *cb);

void led_pwm_control(led_t *cb);

#endif
