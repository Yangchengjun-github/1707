#include "led.h"
#include "app.h"
#include "stdio.h"
#include "debug.h"
/**@brief       Init LED1 and LED2.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void f_led_show_battery(void *p);
void f_led_discharge(void *p);
void f_led_charge(void *p) ;
void f_led_alloff(void *p) ;
void f_led_health(void *p);
void f_led_warning(warn_cb_t*p);
void f_led_port_warning(warn_cb_t *p);
void f_led_port_normal(void *p);
void f_led_err(void*p);
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
        .bat.method.pf_led_warning = f_led_warning,
        .bat.method.pf_led_err = f_led_err,
		.port.method.pf_led_normal = f_led_port_normal,
		.port.method.pf_led_warning = f_led_port_warning,
        
        
};

void ledBreath_init(breath_t *p, uint8_t dir, uint16_t cnt, uint16_t cnt2, uint16_t duty, uint16_t cycle, uint16_t preiod)
{
    p->dir = dir;
    p->cnt = cnt;
    p->cnt2 = cnt2;
    p->duty = duty;
    p->cycle = cycle;
    p->preiod = preiod;
}
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

    ledBreath_init(&led.bat.breath, 1, 0, 0, 5, 100, 20000);
}



void task_led(void)
{
    led_bat_show(&led);
    led_port_show(&led);
}

void f_led_show_battery(void *p)  //开机3s电量显示
{
    if(led.bat.status != LED_SHOW_BATTERY)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.bat.breath.status = breath_100;
        led.bat.status = LED_SHOW_BATTERY;
        led.bat.timer = 0;
        led.bat.is_run = 1;
    }
 
}



void f_led_discharge(void *p) //放电显示
{
    if (led.bat.status != LED_DISCHARGE)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.bat.status = LED_DISCHARGE;
        led.bat.breath.status = breath_100;
        led.bat.timer = 0;
        // PWM init

    }
        
}
    
void f_led_charge(void *p) //充电显示
{
    if (led.bat.status != LED_CHARGE)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.bat.status = LED_CHARGE;
        led.bat.breath.status = breath_100;

        led.bat.timer = 0;
        led.bat.run_cnt =  sys.bat.soc_level != 0  ? sys.bat.soc_level - 1 : 0;
    }

        
}


void f_led_alloff(void *p)  //所有LED关闭
{
    if (led.bat.status != LED_ALL_OFF)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.bat.breath.status = breath_100;
        led.bat.status = LED_ALL_OFF;
        led.bat.timer = 0;
        led.bat.is_run = 0;
    }

       
}

void f_led_health(void *p) //
{
    if (led.bat.status != LED_HEALTH)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.bat.breath.status = breath_normal;
        led.bat.status = LED_HEALTH;
        led.bat.timer = 0;
        led.bat.timer1 = 0;
        led.bat.health_mode = *((uint8_t *) p);
        led.bat.is_run = 1;
        ledBreath_init(&led.bat.breath, 1, 0, 0, 5, 100, 20000);
    }

       
}

void f_led_warning(warn_cb_t *p) //
{
    if (led.bat.status != LED_WARNING)
    {
        LOG(LOG_LEVEL_INFO, "\n");

        led.bat.breath.status = breath_100;
        led.bat.status = LED_WARNING;
        led.bat.timer = 0;
        led.bat.timer1 = 0;

        led.bat.warning_time = p->disp_time;
        led.bat.warn_mode = p->mode;
    }


}

void f_led_err(void *p)
{
    if (led.bat.status != LED_ERR || led.bat.err_mode != *((uint8_t *)p))
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.bat.breath.status = breath_100;
        led.bat.status = LED_ERR;
        led.bat.timer = 0;
        led.bat.timer1 = 0;
        led.bat.err_mode = *((uint8_t *)p);
    }
        
}

void f_led_port_normal(void *p)  
{
    if (led.port.status != NORMAL)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        led.port.status = NORMAL;
    }
        
}

void f_led_port_warning(warn_cb_t *p)
{
    if(led.port.status != WARNING)
    {
        LOG(LOG_LEVEL_INFO, "\n");
        printf("error cotp:%d,cutp:%d,dotp:%d,dutp:%d\n", sys.temp_err.charge_otp, sys.temp_err.charge_utp, sys.temp_err.discharge_otp, sys.temp_err.discharge_utp);
        printf("error usba oc%d,ov%d\n", sys.port.porta_fault.oc, sys.port.porta_fault.ov);
        printf("error c1:%d,c2:%d,a1:%d,pg:%d\n", sys.port.C1_status == C_PROTECT, sys.port.C2_status == C_PROTECT, sys.port.A1_status == A_PROTECT, sys.port.PG_status == PG_PROTECT);
        led.port.timer2 = 0;
        led.port.timer1 = 0;
        led.port.status = WARNING;
        led.port.warning_time = p->disp_time;
        led.port.warn_mode = p->mode;
    }
}



void led_port_show(led_t *cb)
{
    switch(cb->port.status)
    {
    case WARNING:
        switch (cb->port.warn_mode)
        {
        case WARNING_MODE_A:
            cb->port.timer2++;
            if (cb->port.timer1++ < 500 / TIME_TASK_LED_CALL)
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

            if (cb->port.timer2 > cb->port.warning_time / TIME_TASK_LED_CALL)
            {
                if (!sys.temp_err.charge_otp && !sys.temp_err.charge_utp && !sys.temp_err.discharge_otp && !sys.temp_err.discharge_utp)
                {
                    cb->port.status = NORMAL;
                }
            }
            break;
        case WARNING_MODE_B:
            break;
        default:
            break;
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
                if (sys.flag.aPort_low_current == 1)
                {
                    LED_A1_OFF;
                }
                else
                {
                    LED_A1_ON;
                }
                    
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
        {
            uint8_t i = 0;
            i = cb->bat.timer++ / (150 / TIME_TASK_LED_CALL);
            if(i <= 10)
            {
                switch (i)
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
                    break;
                case 6:
                    LED_8_8;
                    break;
                case 8:
                    
                case 9:
                    
                    LED_0_8;
                    break;
                default:
                    break;
                }
            }
            else
            {
                if (cb->bat.timer < 4000 / TIME_TASK_LED_CALL)
                {
                    switch (sys.bat.soc_level)
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
                        break;
                    case 8:
                        LED_8_8;
                    case 9:
                        LED_8_8;
                    default:
                        break;
                    }
                }
                else
                {
                    cb->bat.method.pf_led_alloff(NULL);
                }
            }
        }


      
        break;
    case LED_DISCHARGE:
        if(sys.port.C1_status != C_DISCHARGE && sys.port.C2_status != C_DISCHARGE && sys.port.A1_status == A_DISCHARGE &&  sys.flag.aPort_low_current == 1)  //A口 小电流 led灯关闭
        {
            LED_0_8;
            break;
        }
        switch (sys.bat.soc_level)
        {
        case 0:
           // LED_0_8; //过放显示（不放电只显示 ）

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
            break;
        case 9:
            LED_8_8;
            break;
        default:
            break;
        }
        break;
    case LED_CHARGE:
    {
        static uint8_t flag = 0;
        if (cb->bat.timer++ >  300 / TIME_TASK_LED_CALL)
        {
            cb->bat.timer = 0;

            if (++cb->bat.run_cnt > sys.bat.soc_level)
            {
                cb->bat.run_cnt = 0; //!
                

                if (sys.bat.soc_level == 0)
                {
                    
                    if(flag)
                    {
                        cb->bat.run_cnt = 1;
                    }
                    else
                    {
                        cb->bat.run_cnt = 0;
                    }
                    flag = !flag;
                }
                else
                {
                    flag = 1;
                }   
            }

            if (sys.bat.soc_level != 0 && cb->bat.run_cnt == 1)
            {
                cb->bat.run_cnt = 2;
            }
            
            if (sys.bat.soc_level == 9) // 满电 长亮
            {
                cb->bat.run_cnt = 9;
            }

            switch (cb->bat.run_cnt)
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
                break;
            case 8:
                LED_8_8;
                break;
            case 9:
                LED_8_8;
                break;
            default:
                break;
            }
        }
        break;
    }
    case LED_HEALTH:
        switch(cb->bat.health_mode)
        {
        case 0:
            if (cb->bat.timer++ < 20*1000 / TIME_TASK_LED_CALL)
            {
                switch (sys.bat.soh_level)
                {
                case 0:
                    LED_1_8;
                    break;
                case 1:
                    LED_2_8;
                    break;
                case 2:
                    LED_3_8;
                    break;
                case 3:
                    LED_4_8;
                    break;
                case 4:
                    LED_5_8;
                    break;
                case 5:
                    LED_6_8;
                    break;
                case 6:
                    LED_7_8;
                    break;
                case 7:
                    LED_8_8;
                    break;
                default:
                    break;
                }
                break;
            }
            else
            {
                cb->bat.status = LED_ALL_OFF;
            }
            break;
        case 1:
            if (cb->bat.timer1++ < 2500 / TIME_TASK_LED_CALL)
            {
                LED_1_8;
            }

            else if (cb->bat.timer1 < 5000 / TIME_TASK_LED_CALL)
            {
                LED_0_8;
            }
            else
            {
                cb->bat.timer1 = 0;
            }
            break;
        default:
            break;
        }
        
        break;
    case LED_WARNING:
        switch (cb->bat.warn_mode)
        {
        case WARNING_MODE_A :
            if (cb->bat.timer1++ < cb->bat.warning_time / TIME_TASK_LED_CALL)
            {
                if (cb->bat.timer++ < 500 / TIME_TASK_LED_CALL)
                {
                    LED_X_8;
                }
                else if (cb->bat.timer < 1000 / TIME_TASK_LED_CALL)
                {
                    LED_0_8;
                }
                else
                {
                    cb->bat.timer = 0;
                }
            }
            else
            {
                cb->bat.status = LED_ALL_OFF;
            }
            
            break;
        case WARNING_MODE_B :
            if (cb->bat.timer1++ < cb->bat.warning_time / TIME_TASK_LED_CALL)
            {
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
            }
            else
            {
                cb->bat.status = LED_ALL_OFF;
            }
            break;
        default:
            break;
        }
        break;
    case LED_ERR:
        switch(led.bat.err_mode)
        {
        case 0:
            
            if (cb->bat.timer++ < 500 / TIME_TASK_LED_CALL)
            {
                LED_ERR1;
            }
            else if (cb->bat.timer < 1000 / TIME_TASK_LED_CALL)
            {
                LED_ERR2;
            }
            else
            {
                cb->bat.timer = 0;
            }
            break;
        case 1:
            if (cb->bat.timer++ < 500 / TIME_TASK_LED_CALL)
            {
                LED_ERR1;
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
            break;
        default :
            break;
        }
        if(cb->bat.timer1 < 10000/TIME_TASK_LED_CALL)
        {
            cb->bat.timer1++;
        }
        else
        {
            cb->bat.method.pf_led_alloff(NULL);
        }
        break;
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
