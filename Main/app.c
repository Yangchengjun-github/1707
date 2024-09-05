#include "app.h"
#include "string.h"
#include "stdint.h"
#include "key.h"
#include "init.h"
#include "adc.h"
#include "led.h"
#include "bms.h"
#include "debug.h"
#include "cs32f10x_pmu.h"
#include "communication.h"
//#include "communication.h"
static uint8_t portA_plug_check(void);

void key_fast_switch(uint8_t key_level);

void sys_switch_check(void);

void sys_pow_on_off_deal(void);

void led_app(void);

void usba_app(void);

void f_uaba_open(void);

void f_uaba_close(void);

void f_uaba_fault_ov(void);

void f_uaba_fault_oc(void);

void eta_control(void);


void power_rank_contorl(void);

void temperature_protect(void);

void power_sw_contorl(void);

void sleep_control(void);

sys_t sys =
{
    .port.method.usbaClose = f_uaba_close,
    .port.method.usbaOpen = f_uaba_open,
	.port.method.usbaFault = f_uaba_fault_ov,
};
xbms_t xbms;


void app_init(void)
{
    memset(&sys, 0, sizeof(sys));
}


void task_app(void)
{
    
    sys_switch_check();
    sys_pow_on_off_deal();
    key_fast_switch(KEY2_IO_LEVEL);
    led_app();
    usba_app();
	power_rank_contorl();
    power_sw_contorl();
    temperature_protect();
    sleep_control();

}


static uint8_t portA_plug_check(void)
{
//    static  uint8_t cnt = 0;
//    //if(IS_PORTA_PLUG)
//    if(sys.port.a_exit)
//    {
//        sys.port.a_exit = 0;
//        //return 1;
//        cnt++;
//        if(cnt > 10)
//        {
//            cnt = 0;
//            return 1;
//        }
//        else
//        {
//            return 0;
//        }
//    }
//    else
//    {
//        cnt = 0;
//        return 0;
//    }



	static uint8_t cnt = 0;
	if(__GPIO_INPUT_PIN_GET(WAKE_A_PORT,WAKE_A_PIN))
	{
		cnt++;
		if(cnt++ > 5)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		cnt = 0;
		return 0;
	}
}


static uint8_t portA_current_check(void)
{
    static uint8_t cnt = 0,cnt1=0;
    if(sys.adc.value[CH_A_I] > 1000)
    {
        cnt1++;
        if(cnt1 > 3)
        {
            return 2;
        }

    }
    else
    {
        cnt1 = 0;
    }


    if(sys.adc.value[CH_A_I] < 20)
    {
        cnt++;
        if(cnt > 3)
        {
            return 1;
        }
    }
    else
    {
        cnt = 0;
        return 0;
    }

    return 0;
}


static uint8_t portA_voltage_check(void)
{
    static uint8_t cnt = 0;
    if(sys.adc.value[CH_A_V] > 1000)
    {
        cnt++;
        if(cnt > 3)
        {
            return 1;
        }
    }
    else
    {
        cnt = 0;
        return 0;
    }

    return 0;
}


void key_fast_switch(uint8_t key_level)
{
    static uint8_t last_level = 0;
    static uint8_t num = 0;
    static uint16_t timer = 0;
    if(last_level != key_level )
    {
        timer = 0;
        if(key_level == RESET)
        {
            num++;
        }
        
    }
    if (timer++ > 5000 / TIME_TASK_ADC_CALL)
    {
        num = 0;
    }
    //printf("num:%d\n",num);
    if(num > 5)
    {
        //printf("----------------------\n");
        sys.flag.health_trig = 1;
        num = 0;
    }   
    last_level = key_level;
}

void sys_switch_check(void)
{
    static uint8_t off_cnt = 0;
    static uint8_t key2_high_cnt = 0;
    static uint8_t key2_low_cnt = 0;

    if (KEY2_IO_LEVEL == SET) // 关机
    {
        key2_high_cnt++;
        if (key2_high_cnt > 3)
        {
            sys.cmd.powOFF = 1;
        }
    }
    else
    {
        key2_high_cnt = 0;
    }

    if (KEY2_IO_LEVEL == RESET) // 开机
    {
        key2_low_cnt++;
        if (key2_low_cnt > 3)
        {

            sys.cmd.powON = 1;
        }
    }
    else
    {
        key2_low_cnt = 0;
    }
}

void sys_pow_on_off_deal(void)
{
    if (sys.cmd.powOFF)
    {
        sys.cmd.powOFF = 0;
        if (sys.state == STATE_ON)
        {
            sys.state = STATE_OFF;
            sys.eta_en = 0;
            DCDC_OFF;
            USBA_OFF;
            G020_OFF;
        }
    }

    if (sys.cmd.powON)
    {

        sys.cmd.powON = 0;
        if (sys.state == STATE_OFF)
        {
            if (led.bat.status != LED_HEALTH && led.bat.status != LED_SHOW_BATTERY)
            {
                led.bat.method.pf_led_show_battery();
                
            }
           // bq76942_reset();
            sys.state = STATE_ON;
            G020_ON;
        }
    }
}
inline void led_app()
{
    //bat led
    if(sys.bat.cap == 0)
    {

    }
    else if (sys.port.PG_status == PG_CHARGE)
    {
        if (led.bat.status != LED_CHARGE)
        {


            led.bat.method.pf_led_charge();
        }
    }
    else if (sys.port.C1_status == C_DISCHARGE || sys.port.C2_status == C_DISCHARGE || sys.port.A1_status == C_DISCHARGE)
    {
        if (led.bat.status != LED_DISCHARGE)
        {

            led.bat.method.pf_led_discharge();
        }
    }
    else if (sys.flag.health_trig && sys.state == STATE_ON)
    {
        sys.flag.health_trig = 0;
        led.bat.method.pf_led_health();
    }
    else

    {
        if (led.bat.status != LED_ALL_OFF && led.bat.status != LED_SHOW_BATTERY && led.bat.status != LED_HEALTH)
        {

            led.bat.method.pf_led_alloff();
            printf("%d\n", __LINE__);
        }
    }

    if(sys.state == STATE_OFF)
    {
        if(led.bat.status != LED_ALL_OFF )
        {
            led.bat.method.pf_led_alloff();
            printf("%d\n", __LINE__);
        }
            
    }

	//port led
	if (sys.port.C1_status == C_PROTECT || sys.port.C2_status == C_PROTECT ||           \
            sys.port.PG_status == PG_PROTECT || sys.port.A1_status == A_PROTECT  ||     \
            sys.temp_err.charge_otp || sys.temp_err.charge_utp||                        \
            sys.temp_err.discharge_otp || sys.temp_err.discharge_utp                    \
        )
	{
		if(led.port.status != WARNING)
			led.port.method.pf_led_warning();
	}
	else
	{
		if(led.port.status != NORMAL)
			led.port.method.pf_led_normal();
	}
}


void usba_app(void)
{
    static uint32_t small_cur_cnt = 0;
	static uint8_t oc_cnt = 0;
	static uint8_t oc_cnt2 = 0;
	static uint16_t recover_cnt = 0;
    switch (sys.state)
    {
    case STATE_OFF:
		if(sys.port.A1_status != A_IDLE)
        sys.port.method.usbaClose();

        break;
    case STATE_ON:
		if(sys.port.A1_status == A_PROTECT)
		{
			recover_cnt++;
			
			if(sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
			
			if(recover_cnt > 3000/TIME_TASK_APP_CALL)
			{
				sys.port.porta_fault.oc = 0;
				sys.port.porta_fault.ov = 0;
				recover_cnt = 0;
				sys.port.method.usbaFault();
			}
			else
			{
				return ;
			}
		}
		else
		{
			recover_cnt = 0;
		}
		
		if(sys.port.PG_status == PG_CHARGE || sys.port.PG_status == PG_PROTECT)  //充电或者错误关闭A口
		{ 
			if(sys.port.A1_status != A_IDLE)
                    sys.port.method.usbaClose();
		}
		
        else if (portA_plug_check() && !sys.port.dis_output) // 端口A 插入负载
        {
            if(sys.port.A1_status != A_DISCHARGE)
                sys.port.method.usbaOpen();
            
            // printf("PortA plug in\r\n");
        }
        else
        {
            
        }
		
		if(sys.adc.conver[CH_A_I]> 2800 && sys.adc.conver[CH_A_V] < 7000) //过流保护1
		{
			oc_cnt++;
			if(oc_cnt > 100/TIME_TASK_APP_CALL)
			{
				oc_cnt = 0;
				sys.port.porta_fault.oc = 1;
				if(sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
			}
		}
		else
		{
			oc_cnt = 0;
		}
		
		if(sys.adc.conver[CH_A_I] > 1800 && sys.adc.conver[CH_A_V] >  10000)  //过流保护2
		{
			oc_cnt++;
			if(oc_cnt > 100/TIME_TASK_APP_CALL)
			{
				oc_cnt = 0;
				sys.port.porta_fault.oc = 1;
				if(sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
			}
		}
		else
		{
			oc_cnt = 0;
		}
		
		if(sys.adc.conver[CH_A_V]> 13500)  //A口过压保护
		{
			sys.port.porta_fault.ov = 1;
			if(sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
		}
		
        if(sys.port.dis_output)
        {
            if (sys.port.A1_status != A_IDLE)
                sys.port.method.usbaClose();
        }

        if ( (sys.adc.conver[CH_A_I] < 20) && (sys.port.A1_status == A_DISCHARGE)) //小电流
        {
            small_cur_cnt++;
            if(small_cur_cnt > 30000ul/TIME_TASK_APP_CALL)
            {
				small_cur_cnt = 0;
                if(sys.port.A1_status != A_IDLE)
                    sys.port.method.usbaClose();
            }
        }
        else 
        {
            small_cur_cnt = 0;
			
        }
		
		
        break;
    default:
        break;
    }
}

void f_uaba_open(void)
{
  LOG(LOG_LEVEL_INFO,"\n");
    USBA_ON;
    DCDC_ON;
    sys.port.A1_status = A_DISCHARGE;
}
void f_uaba_close(void)
{
  LOG(LOG_LEVEL_INFO,"\n");
    USBA_OFF;
    DCDC_OFF;
    sys.port.A1_status = A_IDLE;
}

void f_uaba_fault_ov(void)
{
	printf("event %d %s %s\n", __LINE__, __FILE__, __func__);
    USBA_OFF;
    DCDC_OFF;
    sys.port.A1_status = A_PROTECT;
}

void f_uaba_fault_oc(void)
{
	printf("event %d %s %s\n", __LINE__, __FILE__, __func__);
    USBA_OFF;
    DCDC_OFF;
    sys.port.A1_status = A_PROTECT;
}


void eta_control(void)
{
    if(sys.eta_en)
    {
        EN_ETA_PORT->DO ^= EN_ETA_PIN;
    }
}




//powerDerating
//充电降额
//放电降额
static int charge_powdown_point[] = {};
static int discha_powdown_point[] = {};
void power_rank_contorl(void)
{
    static uint8_t cnt = 0;
    
    if (bms_tmp1 > TEMPERATURE_TH1_K || bms_tmp2 > TEMPERATURE_TH1_K || bms_tmp3 > TEMPERATURE_TH1_K || bms_tmp4 > TEMPERATURE_TH1_K)
    {
        cnt++;
        if(cnt > 1000/TIME_TASK_APP_CALL)
        {
            cnt = 0;
            sys.port.charge_powerdowm = LEVEL_1;
            sys.port.discharge_powerdown = LEVEL_1;
        }
    }
    else
    {
        sys.port.charge_powerdowm = OFF;
        sys.port.discharge_powerdown = OFF;
        cnt = 0;
    }
}

void temperature_protect(void)
{
    if(sys.flag.temp_scan == 0)
        return;
// chage otp
    if (bms_tmp1 > CHA_OTP_PROTECT || bms_tmp2 > CHA_OTP_PROTECT || bms_tmp3 > CHA_OTP_PROTECT )
    {
        sys.temp_err.charge_otp = 1;
    }

    if (bms_tmp1 < CHA_OTP_RECOVER && bms_tmp2 < CHA_OTP_RECOVER && bms_tmp3 < CHA_OTP_RECOVER )
    {
        sys.temp_err.charge_otp = 0;
    }
// charge utp
    if (bms_tmp1 < CHA_UTP_PROTECT || bms_tmp2 < CHA_UTP_PROTECT || bms_tmp3 < CHA_UTP_PROTECT)
    {
        sys.temp_err.charge_utp = 1;
    }

    if (bms_tmp1 > CHA_UTP_RECOVER && bms_tmp2 > CHA_UTP_RECOVER && bms_tmp3 > CHA_UTP_RECOVER)
    {
        sys.temp_err.charge_utp = 0;
    }
 // dis otp
    if (bms_tmp1 > DISC_OTP_PROTECT || bms_tmp2 > DISC_OTP_PROTECT || bms_tmp3 > DISC_OTP_PROTECT)
    {
        sys.temp_err.discharge_otp = 1;
    }

    if( bms_tmp1 < DISC_OTP_RECOVER && bms_tmp2 < DISC_OTP_RECOVER && bms_tmp3 < DISC_OTP_RECOVER)
    {
        sys.temp_err.discharge_otp = 0;
    }
//dis utp
    if (bms_tmp1 < DISC_UTP_PROTECT || bms_tmp2 < DISC_UTP_PROTECT || bms_tmp3 < DISC_UTP_PROTECT)
    {
        sys.temp_err.discharge_utp = 1;
    }

    if (bms_tmp1 > DISC_UTP_RECOVER && bms_tmp2 > DISC_UTP_RECOVER && bms_tmp3 > DISC_UTP_RECOVER)
    {
        sys.temp_err.discharge_utp = 0;
    }




}
void power_sw_contorl(void)
{
    switch (sys.state)
    {
    case STATE_OFF:

        break;
    case STATE_ON:
        if ((sys.temp_err.charge_otp || sys.temp_err.charge_utp) && (sys.temp_err.discharge_otp || sys.temp_err.discharge_utp))
        {
            cmd_g020_write(DIS_CHARGE_DIS_DISCHAR);
#if (BME_EN)
            sys.port.dis_output = 1;
#endif
        }
        else if (sys.temp_err.charge_otp || sys.temp_err.charge_utp)
        {
            cmd_g020_write(DIS_CHARGE_EN_DISCHAR);
        }
        else if ((sys.temp_err.discharge_otp || sys.temp_err.discharge_utp) || sys.bat.cap == 0)
        {
            cmd_g020_write(EN_CHARGE_DIS_DISCHAR);
#if (BME_EN)
            sys.port.dis_output = 1;
#endif
        }
        else
        {
            cmd_g020_write(EN_CHARGE_EN_DISCHAR);
            sys.port.dis_output = 0;
        }
        break;
    default:
        break;
    }

}
void sleep_control(void)
{
	int32_t ret;
    static uint16_t delay;
    switch (sys.state)
    {
    case STATE_OFF :
        if(delay++ > 2000/TIME_TASK_APP_CALL)
        {
            delay = 0;
            printf("sleep\n");
            pmu_stop_mode_enter(PMU_LDO_ON, PMU_DSM_ENTRY_WFI);
			/*Configure system clock after wakeup from stop mode*/
			__RCU_FUNC_ENABLE(HXT_CLK);
			ret = rcu_hxt_stabilization_wait();
			if(ret == SUCCESS)
			{
				/*Enable PLL clock*/
				__RCU_FUNC_ENABLE(PLL_CLK);
				while(rcu_clkready_reset_flag_get(RCU_FLAG_PLL_STABLE) == RESET);
				/*PLL clock as system clock soure*/
				rcu_sysclk_config(RCU_SYSCLK_SEL_PLL);
				while(rcu_sysclk_src_get() != 0x02);
			}
            printf("wakeup\n");
        }
        
        break;
    case  STATE_ON:
        delay = 0;
        break;
    default:
        break;
    }
}
