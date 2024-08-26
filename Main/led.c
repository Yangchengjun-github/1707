#include "led.h"
#include "app.h"
#include "stdio.h"
/**@brief       Init LED1 and LED2.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void f_led_show_battery(void);
void f_led_discharge(void);
void f_led_charge(void) ;
void f_led_alloff(void) ;
void f_led_health(void);
void f_led_port_warning(void);
void f_led_port_normal(void);
led_t led =
    {
        .bat.status = LED_ALL_OFF,
        .bat.run_cnt = 0,
        .bat.timer = 0,
        .bat.method.pf_led_charge = f_led_charge,
        .bat.method.pf_led_discharge = f_led_discharge,
        .bat.method.pf_led_show_battery = f_led_show_battery,
        .bat.method.pf_led_health = f_led_health,
		.bat.method.pf_led_alloff = f_led_alloff,
		.port.method.pf_led_normal = f_led_port_normal,
		.port.method.pf_led_warning = f_led_port_warning,
        
};
void led_init(void)
{
    /* Enable the clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    gpio_mode_config(LED1_PORT, LED1_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED2_PORT, LED2_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED3_PORT, LED3_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED4_PORT, LED4_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED5_PORT, LED5_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED6_PORT, LED6_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED7_PORT, LED7_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED8_PORT, LED8_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
	
	gpio_mode_config(LED_PWM_PORT, LED_PWM_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));

    gpio_mode_config(LED_C1_PORT, LED_C1_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(LED_C2_PORT, LED_C2_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));//SWCLK PA14
    gpio_mode_config(LED_A1_PORT, LED_A1_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH)); // SWDIO PA13
    



    LED1_OFF;
    LED2_OFF;
    LED3_OFF;
    LED4_OFF;
    LED5_OFF;
    LED6_OFF;
    LED7_OFF;
    LED8_OFF;
    LED_C1_OFF;
    LED_C2_OFF;
    LED_A1_OFF;   
    LED_PWM_OFF;

	led.bat.breath.cnt = 0;
	led.bat.breath.cnt2 = 0;
    led.bat.breath.duty = 5 ;
    led.bat.breath.cycle = 100;
    led.bat.breath.preiod = 20000;
}



void task_led(void)
{
    led_bat_show(&led);
    led_port_show(&led);
}

void f_led_show_battery(void)  //开机3s电量显示
{
    printf("event %d %s\n", __LINE__, __FILE__);
	led.bat.breath.status = breath_100;
    led.bat.status =  LED_SHOW_BATTERY;
    led.bat.timer = 0;
}

void f_led_discharge(void) //放电显示
{
    printf("event %d %s\n", __LINE__, __FILE__);
    led.bat.status = LED_DISCHARGE;
    led.bat.breath.status = breath_normal;
    led.bat.timer = 0;
}
    
void f_led_charge(void) //充电显示
{
    printf("event %d %s\n", __LINE__, __FILE__);
    led.bat.status = LED_CHARGE;
    led.bat.breath.status = breath_100;
    
    led.bat.timer = 0;
    led.bat.run_cnt= 2;
}


void f_led_alloff(void)  //所有LED关闭
{
    printf("event %d %s\n", __LINE__, __FILE__);
    led.bat.status = LED_ALL_OFF;
    led.bat.timer = 0;
}

void f_led_health(void) //
{
    printf("event %d %s\n",__LINE__,__FILE__);
    led.bat.status = LED_HEALTH;
    led.bat.timer = 0;
}

void f_led_port_normal(void)  
{
    printf("event %d %s\n", __LINE__, __FILE__);
	led.port.status = NORMAL;
}

void f_led_port_warning(void)
{
    printf("event %d %s\n", __LINE__, __FILE__);
	led.port.timer2 = 0;
	led.port.status = WARNING;
}

void led_port_show(led_t *cb)
{
    switch(cb->port.status)
    {
    case WARNING:
        cb->port.timer2++;
        if(cb->port.timer1++ < 500/TIME_TASK_LED_CALL)
        {
            LED_C1_ON;
            LED_C2_ON;
            LED_A1_ON;
        }
        else if (cb->port.timer1 < 1000 / TIME_TASK_LED_CALL)
        {
            LED_C1_OFF;
            LED_C2_OFF;
            LED_A1_OFF;
        }
        else
        {
            cb->port.timer1 = 0;
        }

        if (cb->port.timer2 > 20000/TIME_TASK_LED_CALL)
        {
            cb->port.status = NORMAL;
        }

        break;
    case NORMAL:
        switch(sys.state)
        {
        case STATE_ON:
         
            switch(sys.port.A1_status)
            {
            case A_IDLE:
				LED_A1_OFF;
                break;
            case A_DISCHARGE:
                LED_A1_ON;
                break;
			case A_PROTECT:
				LED_A1_ON;
				break;
            

            }
            
            switch (sys.port.C1_status)
            {
            case C_IDLE:
				LED_C1_OFF;
                break;
            case C_DISCHARGE:
                LED_C1_ON;
                break;
            case C_PROTECT:
                LED_C1_ON;
                break;
            }
            
            switch (sys.port.C2_status)
            {
            case C_IDLE:
				LED_C2_OFF;
                break;
            case C_DISCHARGE:
                LED_C2_ON;
                break;
            case C_PROTECT:
                LED_C2_ON;
                break;
            }
            break;
        case STATE_OFF:
            LED_A1_OFF;
            LED_C1_OFF;
            LED_C2_OFF;
            break;    
        }
        
    }
}

void led_bat_show(led_t *cb)
{

    switch (cb->bat.status)
    {
    case LED_ALL_OFF:
        LED_0_8;
        break;
    case LED_SHOW_BATTERY:
        if(cb->bat.timer++ < 3000/TIME_TASK_LED_CALL)
        {
            switch(sys.bat.cap)
            {
            case 0:
                LED_0_8;
                break;
            case 1:
                LED_1_8;
                break;
            case 2:
                LED_2_8;
                break;
            case 3:
                LED_3_8;
                break;
            case 4:
                LED_4_8;
                break;
            case 5:
                LED_5_8;
                break;
            case 6:
                LED_6_8;
                break;
            case 7:
                LED_7_8;
            case 8:
                LED_8_8;
            default:
                break;
            }
        }
        else
        {
            cb->bat.method.pf_led_alloff();
        }
        break;
    case LED_DISCHARGE:

        switch (sys.bat.cap)
        {
        case 0:
            LED_0_8;
            break;
        case 1:  //红灯闪
			if (cb->bat.timer++ < 500 / TIME_TASK_LED_CALL)
			{
				LED_1_8;
			}
            else if (cb->bat.timer < 1000 / TIME_TASK_LED_CALL)
			{
				LED_0_8;
			}
			else
			{
				cb->bat.timer = 0;
			}
            break;
        case 2:
            LED_2_8;
            break;
        case 3:
            LED_3_8;
            break;
        case 4:
            LED_4_8;
            break;
        case 5:
            LED_5_8;
            break;
        case 6:
            LED_6_8;
            break;
        case 7:
            LED_7_8;
			break;
        case 8:
            LED_8_8;
        default:
            break;
        }
        break;
    case LED_CHARGE:
        if (cb->bat.timer++ > 500 / TIME_TASK_LED_CALL)
        {
            cb->bat.timer = 0;
            
            if (cb->bat.run_cnt++ > sys.bat.cap)
            {
                cb->bat.run_cnt = 2; //!
            }

            switch (cb->bat.run_cnt)
            {
            case 0:

            case 1:

            case 2:
                LED_2_8;
                break;
            case 3:
                LED_3_8;
                break;
            case 4:
                LED_4_8;
                break;
            case 5:
                LED_5_8;
                break;
            case 6:
                LED_6_8;
                break;
            case 7:
                LED_7_8;
            case 8:
                LED_8_8;
            default:
                break;
            }
        }
        break;
    case LED_HEALTH:
        if(cb->bat.timer < 10000/TIME_TASK_LED_CALL)
        {
            switch (sys.bat.health)
            {
            case 0:
                LED_2_8;
                break;
            case 1:
                LED_3_8;
                break;
            case 2:
                LED_4_8;
                break;
            case 3:
                LED_5_8;
                break;
            case 4:
                LED_6_8;
                break;
            case 5:
                LED_7_8;
            case 6:
                LED_8_8;
            default:
                break;
            }
            break;
        }
        else
        {
            cb->bat.status = LED_ALL_OFF;
        }
        
    default:
        break;
    }
}
void led_pwm_control(led_t *cb)
{
    cb->bat.breath.cnt++;
    cb->isr_int++;
    switch (cb->bat.breath.status)
    {
    case breath_0:
        // PWM -> IO
        LED_PWM_OFF;
        break;
    case breath_normal:
        // IO -> PWM
        

        if (cb->bat.breath.cnt2++ % (cb->bat.breath.preiod / cb->bat.breath.cycle) == 0)
        {
            if (cb->bat.breath.dir)
            {
                cb->bat.breath.duty++;
            }
            else
            {
                if (cb->bat.breath.duty)
                    cb->bat.breath.duty--;
            }
        }

        if (cb->bat.breath.cnt2 >= cb->bat.breath.preiod)
        {
            cb->bat.breath.cnt2 = 0;
            cb->bat.breath.dir = !cb->bat.breath.dir;
        }

        if (cb->bat.breath.cnt >= cb->bat.breath.cycle)
        {
            cb->bat.breath.cnt = 0;
        }
        if (cb->bat.breath.cnt < cb->bat.breath.duty)
        {
            LED_PWM_ON;
        }
        else
        {
            LED_PWM_OFF;
        }
        break;
    case breath_100:
        // PWM -> IO
        LED_PWM_ON;
        break;
    }
}


void swd_to_gpio(void)
{
    
}