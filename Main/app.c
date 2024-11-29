#include "app.h"
#include "iic.h"
#include "adc.h"
#include "bms.h"
#include "communication.h"
#include "coulomp.h"
#include "cs32f10x_fwdt.h"
#include "cs32f10x_pmu.h"
#include "cs32f10x_rtc.h"
#include "debug.h"
#include "init.h"
#include "key.h"
#include "led.h"
#include "stdint.h"
#include "string.h"
#include "define.h"
#include "flash.h"
// #include "communication.h"

extern __IO uint8_t second_flag;
static uint8_t portA_plug_check(void);

void key_fast_switch(uint8_t key_level);

void app_key(void);

void app_sys_toggle_deal(void);

void app_led_control(void);

void app_usba_control(void);

void f_uaba_open(void);

void f_uaba_close(void);

void f_uaba_fault_ov(void);

void f_uaba_fault_oc(void);

void eta_driver(void);

void app_power_rank_contorl(void);

void app_temperature_check(void);

void app_power_sw_contorl(void);

void app_sleep(uint8_t);

void app_bms_comm_recover(void);

void app_eta_control(void);

void app_bms_charge_to_active(void);

void app_shake_check(uint8_t io_state,uint8_t *shake,uint8_t sys_state);

static void sleep_del(uint8_t);
void app_rtc(void);
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
#if (BME_EN)
    app_bms_charge_to_active();
    app_bms_comm_recover();
#endif
    app_key(); //波动开关（开关机，显示健康度）
    app_shake_check(KEY1_IO_LEVEL, &sys.isShake, sys.state); //摇一摇检测（震动开关）
    app_sys_toggle_deal();  //系统开关状态切换处理
    app_rtc(); 
    app_led_control(); //GUI
    
    app_temperature_check(); // 温度检测
  //  app_power_rank_contorl(); // 温度降额
    
    app_power_sw_contorl();    // 充放电使能失能控制
    app_usba_control(); // usbA 功能，及其保护
    app_eta_control(); //电池均衡

    app_sleep(sys.state); // TODO 睡眠需配置外设
}
void app_eta_control(void)
{
    if (sys.port.PG_status == PG_CHARGE)
    {
        sys.eta_en = 1;
    }
    else
    {
        sys.eta_en = 0;
    }
}

static uint8_t portA_plug_check(void)
{

    static uint16_t cnt = 0;
    if (__GPIO_INPUT_PIN_GET(WAKE_A_PORT, WAKE_A_PIN))
    {
        cnt++;
        if (cnt++ > 2000 / TIME_TASK_APP_CALL)
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

//static uint8_t portA_current_check(void)
//{
//    static uint8_t cnt = 0, cnt1 = 0;
//    if (sys.adc.value[CH_A_I] > 1000)
//    {
//        cnt1++;
//        if (cnt1 > 3)
//        {
//            return 2;
//        }
//    }
//    else
//    {
//        cnt1 = 0;
//    }

//    if (sys.adc.value[CH_A_I] < 20)
//    {
//        cnt++;
//        if (cnt > 3)
//        {
//            return 1;
//        }
//    }
//    else
//    {
//        cnt = 0;
//        return 0;
//    }

//    return 0;
//}

//static uint8_t portA_voltage_check(void)
//{
//    static uint8_t cnt = 0;
//    if (sys.adc.value[CH_A_V] > 1000)
//    {
//        cnt++;
//        if (cnt > 3)
//        {
//            return 1;
//        }
//    }
//    else
//    {
//        cnt = 0;
//        return 0;
//    }

//    return 0;
//}

void key_fast_switch(uint8_t key_level)
{
    static uint8_t last_level = 0;
    static uint8_t num = 0;
    static uint16_t timer = 0;
    if (last_level != key_level)
    {
        timer = 0;
        if (key_level == RESET)
        {
            num++;
        }
    }
    if (timer++ > 2000 / TIME_TASK_ADC_CALL)
    {
        num = 0;
    }
    // printf("num:%d\n",num);
    if (num >= 5)
    {
        // printf("----------------------\n");
        sys.flag.health_trig = 1;
        num = 0;
    }
    last_level = key_level;
}

void app_key(void)
{
   
    static uint8_t key2_high_cnt = 0;
    static uint8_t key2_low_cnt = 0;
/* ---------------------------------- 开关机功能 --------------------------------- */
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
/* -------------------------------- 触发显示健康度功能 ------------------------------- */
    key_fast_switch(KEY2_IO_LEVEL); // 开关触发健康度
}

void app_sys_toggle_deal(void)
{
    if (sys.cmd.powOFF)
    {
        sys.cmd.powOFF = 0;
        if (sys.state == STATE_ON) // 关机
        {
            sys.state = STATE_OFF;
            sys.eta_en = 0;
            DCDC_OFF;
            USBA_OFF;
            G020_OFF;
            app_flash_save();
        }
    }

    if (sys.cmd.powON)
    {

        sys.cmd.powON = 0;
        if (sys.state == STATE_OFF) // 开机
        {

            led.bat.method.pf_led_show_battery(NULL);
            sys.state = STATE_ON;
            G020_ON;
            sys.flag.Low_current_unload = 0;
        }
    }
}
inline void app_led_control()
{
    static uint8_t err_cnt = 0;
    static uint8_t err_display = 0;
    uint8_t record = 0;
    switch (sys.state)
    {
    case STATE_OFF:

        led.bat.method.pf_led_alloff(NULL);
        led.port.method.pf_led_normal(NULL);
        err_display = 1;
        break;
    case STATE_ON:
        do
        {
            /* ------------------------------- 开机显示电量或者健康 ------------------------------ */
            if (led.bat.is_run)
            {
                
                if (sys.flag.health_trig) // 电池健康
                {
                    sys.flag.health_trig = 0;
                    uint8_t value = 0;
                    led.bat.method.pf_led_health(&value);
                    led.port.method.pf_led_normal(NULL);
                }
                break;
            }
            /* ------------------------------- // IIC 通讯错误 ------------------------------ */
            if (sys.flag.iic_err == 1)
            {
                if (err_display)
                {
                    uint8_t err_mode;
                    err_mode = 0;
                    led.bat.method.pf_led_err(&err_mode);
                    led.port.method.pf_led_normal(NULL);
                    err_display = 0;
                }
                break;
            }
            /* -------------------------------- // 锂保未激活 -------------------------------- */
            if (sys.flag.bms_active == 0)
            {
                if (err_display)
                {
                    uint8_t err_mode;
                    err_mode = 1;
                    led.bat.method.pf_led_err(&err_mode);
                    led.port.method.pf_led_normal(NULL);
                    err_display = 0;
                }

                break;
            }
            /* --------------------------------- // 健康度低 -------------------------------- */
            // if (sys.bat.soh <= HEALTH_WARN_TH)
            // {
            //     uint8_t value = 1;
            //     led.bat.method.pf_led_health(&value);
            //     led.port.method.pf_led_normal(NULL);
            //     break;
            // }
            /* ------------------------------- // 温度异常------------------------------- */
            if (*((uint8_t *)&sys.temp_err))
            {
                warn_cb_t cb;
                cb.disp_time = 5000;
                cb.mode = WARNING_MODE_A;

                led.bat.method.pf_led_warning(&cb);
                led.port.method.pf_led_warning(&cb);
                printf("LED:%d\n",__LINE__);
                break;
            }
            /* ----------------------------------- 过放 ----------------------------------- */
            if (sys.bat.soc == 0 && sys.port.a_pulgin)
            {
                warn_cb_t cb;
                cb.disp_time = 5000;
                cb.mode = WARNING_MODE_A;

                led.bat.method.pf_led_warning(&cb);
                led.port.method.pf_led_warning(&cb);
                printf("LED:%d\n", __LINE__);
                break;
            }
            /* --------------------------- UVP/OVP/OCP/SCP 保护 --------------------------- */
            if (sys.bms_protect)
            {
                warn_cb_t cb;
                cb.disp_time = 5000;
                cb.mode = WARNING_MODE_A;

                led.bat.method.pf_led_warning(&cb);
                led.port.method.pf_led_warning(&cb);
                break;
            }
            /* -------------------------------- // 摇一摇 -------------------------------- */
            if (sys.isShake && sys.bat.soc ==  0)
            {
                warn_cb_t cb;
                cb.disp_time = 20 * 1000;
                cb.mode = WARNING_MODE_B;
                sys.isShake = 0;
                led.bat.method.pf_led_warning(&cb);
                led.port.method.pf_led_warning(&cb);
                break;
            }
            /* --------------------------------- C口，PG保护 -------------------------------- */
            if (sys.port.C1_status == C_PROTECT || sys.port.C2_status == C_PROTECT ||
                sys.port.PG_status == PG_PROTECT || sys.port.A1_status == A_PROTECT) // g020给的异常状态
            {
                record = 1;
                err_cnt++;
                if (err_cnt > 1000 / TIME_TASK_APP_CALL) // 对g020 信号滤波
                {
                    err_cnt = 0;
                    warn_cb_t cb;
                    cb.disp_time = 5000;
                    cb.mode = WARNING_MODE_A;
                    led.bat.method.pf_led_warning(&cb);
                    led.port.method.pf_led_warning(&cb);
                    printf("LED:%d\n", __LINE__);
                }
                break;
            }
            /* ---------------------------------- // 充电 --------------------------------- */
            if (sys.port.PG_status == PG_CHARGE)
            {

                led.bat.method.pf_led_charge(NULL);
                led.port.method.pf_led_normal(NULL);
                break;
            }
            /* ---------------------------------- // 放电 --------------------------------- */
            if (sys.port.C1_status == C_DISCHARGE || sys.port.C2_status == C_DISCHARGE || sys.port.A1_status == A_DISCHARGE)
            {
                led.bat.method.pf_led_discharge(NULL);
                led.port.method.pf_led_normal(NULL);
                break;
            }

            /* --------------------------------- // 充电拔除 -------------------------------- */
            if (led.bat.status == LED_CHARGE)
            {
                led.bat.method.pf_led_show_battery(NULL);
                led.port.method.pf_led_normal(NULL);
                break;
            }
            /* ---------------------------------- led熄灭 --------------------------------- */
            if (led.bat.status != LED_SHOW_BATTERY && led.bat.status != LED_HEALTH && led.bat.status != LED_WARNING) // 息屏状态
            {

                led.bat.method.pf_led_alloff(NULL);
                led.port.method.pf_led_normal(NULL);
                break;
            }

        } while (0);

        if (record == 0) // 对g020信号滤波
        {
            err_cnt = 0;
        }

        break;
    default:
        break;
    }

}

void app_usba_control(void)
{
    static uint32_t small_cur_cnt = 0;
    static uint8_t oc_cnt[3] = {0};
    static uint16_t pulgin_cnt = 0;
    static uint32_t recover_cnt = 0;
    static uint8_t isApulgin = 0;
    static uint16_t noload_cnt = 0;
    static uint16_t nopulgin_cnt = 0;
    switch (sys.state)
    {
    case STATE_OFF:
        if (sys.port.A1_status != A_IDLE)
        {
            sys.port.method.usbaClose();
        }
            

        break;
    case STATE_ON:
        isApulgin =  portA_plug_check();
        if (sys.port.A1_status == A_PROTECT)
        {
            recover_cnt++;

            if (recover_cnt > 3000 / TIME_TASK_APP_CALL)
            {
                sys.port.porta_fault.oc = 0;
                sys.port.porta_fault.ov = 0;
                recover_cnt = 0;
                sys.port.method.usbaClose();
            }
            else
            {
                return;
            }
        }        else
        {
            recover_cnt = 0;
        }

        if (sys.port.PG_status == PG_CHARGE || sys.port.PG_status == PG_PROTECT) // 充电或者错误关闭A口 (充电插入)
        {
            if (sys.port.A1_status != A_IDLE)
            {
                sys.port.method.usbaClose();
            }
                
        }

        else if (isApulgin) // 端口A 插入负载
        {
            if (!sys.port.dis_portA_dsg && sys.flag.bms_active) //放电条件判断
            {
                if (sys.port.A1_status != A_DISCHARGE && sys.flag.Low_current_unload == 0 )
                    sys.port.method.usbaOpen();
            }
            sys.port.a_pulgin = 1;    
        }
        else
        {
        }

        if (isApulgin == 0 && sys.port.a_pulgin ) //插入状态清除
        {
            pulgin_cnt++;

            if(pulgin_cnt > 5000 / TIME_TASK_APP_CALL)
            {
                sys.port.a_pulgin = 0;
                pulgin_cnt = 0;
            }
        }
        else
        {
            pulgin_cnt = 0;
        }

        if (__GPIO_INPUT_PIN_GET(WAKE_A_PORT, WAKE_A_PIN) == 0)
        {
            nopulgin_cnt++;
            if(nopulgin_cnt > 500 / TIME_TASK_APP_CALL)
            {
                sys.flag.Low_current_unload = 0;
                nopulgin_cnt = 0;
            }
        }
        else
        {
            nopulgin_cnt = 0;
        }
/* --------------------------------- 过流保护 --------------------------------- */
		if(sys.adc.conver[CH_A_I]> 3300 && sys.adc.conver[CH_A_V] <= 7000) //过流保护1
		{
			oc_cnt[0]++;
			if(oc_cnt[0] > 100/TIME_TASK_APP_CALL)
			{
                oc_cnt[0] = 0;
                sys.port.porta_fault.oc = 1;
				if(sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
			}
		}
		else
		{
            oc_cnt[0] = 0;
        }

        if (sys.adc.conver[CH_A_I] > 2400 && sys.adc.conver[CH_A_V] > 7000 && sys.adc.conver[CH_A_V]  <=11500) // 过流保护2
        {
            oc_cnt[1]++;
            if (oc_cnt[1] > 100 / TIME_TASK_APP_CALL)
            {
                oc_cnt[1] = 0;
                sys.port.porta_fault.oc = 1;
                if (sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
            }
        }
        else
        {
            oc_cnt[1] = 0;
        }

        if (sys.adc.conver[CH_A_I] > 1800 && sys.adc.conver[CH_A_V] > 11500 && sys.adc.conver[CH_A_V] < 15000) // 过流保护3
        {
            oc_cnt[2]++;
            if (oc_cnt[2] > 100 / TIME_TASK_APP_CALL)
            {
                oc_cnt[2] = 0;
                sys.port.porta_fault.oc = 1;
                if (sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
            }
        }
        else
        {
            oc_cnt[2] = 0;
        }
/* ---------------------------------- 过压保护 ---------------------------------- */
        if (sys.adc.conver[CH_A_V] > 15000) // A口过压保护
        {
            sys.port.porta_fault.ov = 1;
            if (sys.port.A1_status != A_PROTECT)
                sys.port.method.usbaFault();
        }
/* -------------------------- 其他原因禁止放到（过温，soc0，过放） -------------------------- */
        if (sys.port.dis_portA_dsg)
        {
            if (sys.port.A1_status != A_IDLE)
            {
                sys.port.method.usbaClose();
            }
                
        }
/* ---------------------------------- 小电流退载 --------------------------------- */
        if (sys.port.A1_status == A_DISCHARGE)
        {
            if (sys.adc.value[CH_A_I] <= 1) // 小电流  //!adc 2 对应 72ma
            {
                sys.flag.aPort_low_current = 1;
                small_cur_cnt++;
                if (small_cur_cnt > 2 * 60 * 60 * 1000ul / TIME_TASK_APP_CALL)
              //  if (small_cur_cnt >5 * 1000ul / TIME_TASK_APP_CALL)
                {
                    small_cur_cnt = 0;
                    if (sys.port.A1_status != A_IDLE)
                    {
                        sys.port.method.usbaClose();
                        printf("A口小电流退载\n");
                        sys.flag.Low_current_unload = 1;

                    }
                        
                }
            }
            else
            {
                if (sys.flag.aPort_low_current == 1)
                {
                    if (sys.adc.value[CH_A_I] >= 3)//!ad 值
                    {
                        sys.flag.aPort_low_current = 0;
                    }
                }
                small_cur_cnt = 0;
            }
        }
/* -------------------------------- 0电流退载（拔出） ------------------------------- */
        if (sys.port.A1_status == A_DISCHARGE)
        {
            if (sys.adc.value[CH_A_I] == 0) // 无电流  //adc 1 对应 80ma
            {
                noload_cnt++;
                if(noload_cnt > 5 * 1000ul / TIME_TASK_APP_CALL)
                {
                    noload_cnt = 0;

                    sys.port.method.usbaClose();
                }
                
                
            }
            else
            {
                noload_cnt = 0;
            }
        }
        break;
    default:
        break;
    }
}

void f_uaba_open(void)
{
    LOG(LOG_LEVEL_INFO, "\n");
    USBA_ON;
    DCDC_ON;
    sys.port.A1_status = A_DISCHARGE;
    sys.port.A1_status_To_g020 = A_DISCHARGE_2;
}
void f_uaba_close(void)
{
    LOG(LOG_LEVEL_INFO, "\n");
    USBA_OFF;
    DCDC_OFF;
    sys.port.A1_status = A_IDLE;
    sys.port.A1_status_To_g020 = A_IDLE_2;
}

void f_uaba_fault_ov(void)
{
    printf("event %d %s %s\n", __LINE__, __FILE__, __func__);
    USBA_OFF;
    DCDC_OFF;
    sys.port.A1_status = A_PROTECT;
    sys.port.A1_status_To_g020 = A_IDLE_2;
}

void f_uaba_fault_oc(void)
{
    printf("event %d %s %s\n", __LINE__, __FILE__, __func__);
    USBA_OFF;
    DCDC_OFF;
    sys.port.A1_status = A_PROTECT;
    sys.port.A1_status_To_g020 = A_IDLE_2;
}

/**
 * @brief ETA波形产生
 *
 * @details
 */
void eta_driver(void)
{
    if (sys.eta_en)
    {
        EN_ETA_PORT->DO ^= EN_ETA_PIN;
    }
}

// powerDerating
// 充电降额
// 放电降额
//static int charge_powdown_point[] = {0};
//static int discha_powdown_point[] = {0};
void app_power_rank_contorl(void)
{
    static uint8_t cnt = 0;

    if (bms_tmp1 > TEMPERATURE_TH1_K || bms_tmp2 > TEMPERATURE_TH1_K || bms_tmp3 > TEMPERATURE_TH1_K || bms_tmp4 > TEMPERATURE_TH1_K)
    {
        cnt++;
        if (cnt > 1000 / TIME_TASK_APP_CALL)
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

void app_temperature_check(void)
{
    static uint16_t rep[4] = {0};
    if (sys.flag.temp_scan == 0)
        return;
    
    if(sys.port.PG_status == PG_CHARGE)
    {
        sys.temp_err.discharge_otp = 0;
        sys.temp_err.discharge_utp = 0;
        /* ------------------------------ // chage otp ------------------------------ */
        if (bms_tmp1 > CHA_OTP_PROTECT || bms_tmp2 > CHA_OTP_PROTECT || bms_tmp3 > CHA_OTP_PROTECT)
        {
            if (rep[0]++ > 1000 / TIME_TASK_APP_CALL)
            {
                sys.temp_err.charge_otp = 1;
            }
        }
        else
        {
            rep[0] = 0;
        }

        if (sys.temp_err.charge_otp && bms_tmp1 < CHA_OTP_RECOVER && bms_tmp2 < CHA_OTP_RECOVER && bms_tmp3 < CHA_OTP_RECOVER)
        {
            sys.temp_err.charge_otp = 0;
        }

        /* ------------------------------ // charge utp ----------------------------- */
        if (bms_tmp1 < CHA_UTP_PROTECT || bms_tmp2 < CHA_UTP_PROTECT || bms_tmp3 < CHA_UTP_PROTECT)
        {
            if (bms_tmp1 == 0)
            {
                rep[1] = 0; // todo  bms需要温度测量需要一段时间，未准备好时，读出为0
            }
            if (rep[1]++ > 1000 / TIME_TASK_APP_CALL)
                sys.temp_err.charge_utp = 1;
        }
        else
        {
            rep[1] = 0;
        }

        if (sys.temp_err.charge_utp && bms_tmp1 > CHA_UTP_RECOVER && bms_tmp2 > CHA_UTP_RECOVER && bms_tmp3 > CHA_UTP_RECOVER)
        {
            sys.temp_err.charge_utp = 0;
        }
    }
    else
    {
        sys.temp_err.charge_otp = 0;
        sys.temp_err.charge_utp = 0;
        /* ------------------------------- // dis otp ------------------------------- */
        if (bms_tmp1 > DISC_OTP_PROTECT || bms_tmp2 > DISC_OTP_PROTECT || bms_tmp3 > DISC_OTP_PROTECT)
        {
            if (rep[2]++ > 1000 / TIME_TASK_APP_CALL)
                sys.temp_err.discharge_otp = 1;
        }
        else
        {
            rep[2] = 0;
        }

        if (sys.temp_err.discharge_otp && bms_tmp1 < DISC_OTP_RECOVER && bms_tmp2 < DISC_OTP_RECOVER && bms_tmp3 < DISC_OTP_PROTECT)
        {
            sys.temp_err.discharge_otp = 0;
        }
        /* ------------------------------- // dis utp ------------------------------- */
        if (bms_tmp1 < DISC_UTP_PROTECT || bms_tmp2 < DISC_UTP_PROTECT || bms_tmp3 < DISC_UTP_PROTECT)
        {
            if (bms_tmp1 == 0)
            {
                rep[3] = 0; // todo  bms需要温度测量需要一段时间，未准备好时，读出为0
            }
            if (rep[3]++ > 1000 / TIME_TASK_APP_CALL)
                sys.temp_err.discharge_utp = 1;
        }
        else
        {
            rep[3] = 0;
        }

        if (sys.temp_err.discharge_utp && bms_tmp1 > DISC_UTP_RECOVER && bms_tmp2 > DISC_UTP_RECOVER && bms_tmp3 > DISC_UTP_RECOVER)
        {
            sys.temp_err.discharge_utp = 0;
        }
    }


    

}
#pragma pack(1)
typedef struct 
{
    uint8_t temp_fault : 1;
}chag_dis_t;
typedef struct 
{
    uint8_t temp_fault : 1;
    uint8_t soc0 : 1;
}disg_dis_t;

#pragma pack()
void app_power_sw_contorl(void)
{
    sys.port.dis_portA_dsg = 0;
    sys.port.dis_G020_chg = 0;
    sys.port.dis_G020_dsg = 0;
    sys.bms_protect = 0;
    switch (sys.state)
    {
    case STATE_OFF:
        sys.port.dis_G020_chg = 1;
        sys.port.dis_G020_dsg = 1;
        
        break;
    case STATE_ON:


        /* ---------------------------------- 温度保护 ---------------------------------- */
        if (sys.temp_err.charge_otp || sys.temp_err.charge_utp)
        {
            sys.port.dis_G020_chg = 1;
        }
        if (sys.temp_err.discharge_otp || sys.temp_err.discharge_utp)
        {
            sys.port.dis_G020_dsg = 1;
            sys.port.dis_portA_dsg = 1;
        }

        /* ---------------------------- // 电量0  || 锂保放电保护 --------------------------- */
        if (sys.bat.soc_level == 0 || DSG == 0 ) 
        {
            sys.port.dis_G020_dsg = 1;
            sys.port.dis_portA_dsg = 1;
        }
        /* --------------------------------- 充电禁止放电 --------------------------------- */
        if (sys.port.PG_status != PG_IDLE)
        {
            sys.port.dis_G020_dsg = 1;
            sys.port.dis_portA_dsg = 1;
        }
        /* ------------------------ UVP / OVP / OCP / SCP 保护 ------------------------ */
        if (OV_Fault || OCC_Fault)
        {
            sys.port.dis_G020_chg = 1;
            sys.bms_protect = 1;
        }
        if( UV_Fault ||  OCD_Fault || SCD_Fault)
        {
            sys.port.dis_G020_dsg = 1;
            sys.port.dis_portA_dsg = 1;
            sys.bms_protect = 1;
        }

        
        break;
    default:
        break;
    }
}
void app_sleep(uint8_t state)
{

    static uint16_t delay_off;
    static uint32_t delay_on;
    switch (state)
    {
    case STATE_OFF:
        delay_on = 0;
        if (delay_off++ > 10000 / TIME_TASK_APP_CALL) // 需大于开关快速的判断时间
        {
            delay_off = 0;
            printf("sleep(OFF)\n");
            sleep_del(state);
            printf("wakeup(OFF)\n");
        }

        break;
    case STATE_ON:
        if (sys.uart3_idle_cntdown == 0 && sys.port.C1_status == C_IDLE && sys.port.C2_status == C_IDLE && sys.port.A1_status == A_IDLE && sys.port.PG_status == PG_IDLE)
        {
            if (delay_on++ > 10000 / TIME_TASK_APP_CALL)
            {
                delay_on = 0;
                printf("sleep(ON)\n");
                sleep_del(state);
                printf("wakeup(ON)\n");
            }
        }
        else
        {
            delay_on = 0;
        }
        delay_off = 0;
        break;
    default:
        break;
    }
}

void app_bms_comm_recover(void)
{
    static uint16_t cnt = 0;
    if (cnt++ > 2000 / TIME_TASK_APP_CALL)
    {
        cnt = 0;
        if (sys.flag.iic_err)
        {
            sys.flag.iic_err = 0;
            bq76942_reset();

            if (sys.flag.iic_err == 0)
            {
                tick_delay(1000);
                coulomp_init(); // 库仑计
            }

            if (sys.flag.iic_err == 0)
                led.bat.method.pf_led_show_battery(NULL);
        }

    }
}
void app_bms_charge_to_active(void)
{


#define test (1)
#if test
    BQ769x2_RESET_DSG_OFF();
	sys.flag.bms_active = 1;
#else
    if (sys.port.PG_status == PG_CHARGE )
    {
        sys.flag.bms_active = 1;

        BQ769x2_RESET_DSG_OFF();
    }
#endif

    // if(sys.flag.bms_active )      //todo 测试使用，正常功能时注释掉
    // {
    //     cnt++;
    //     if (cnt >= 10000 / TIME_TASK_APP_CALL)
    //     {
    //         cnt = 0;
    //         BQ769x2_DSG_OFF();

    // }
}

void tick_delay(uint16_t ms)
{
    sys.tick = ms;
    while (sys.tick)
    {
    }
}
uint32_t cnt_value = 0;
static void sleep_del(uint8_t state)
{

    deinit_befor_sleep(state);
    sys.flag.wake_aport = 0;
    sys.flag.wake_key = 0;
    sys.flag.wake_usart = 0;
    sys.flag.wake_shake = 0;
  //  while(0)
    while (sys.flag.wake_aport == 0 &&sys.flag.wake_key == 0 &&sys.flag.wake_usart == 0 &&sys.flag.wake_shake == 0) // 如果不是这3个中断唤醒，则继续休眠
    {
        sys.flag.wake_aport = 0;
        sys.flag.wake_key = 0;
        sys.flag.wake_usart = 0;
        sys.flag.wake_shake = 0;
        log_init();
#if (WDG_EN)
        fwdt_reload_counter();
        cnt_value = __RTC_COUNTER_GET();
        rtc_alarm_set(cnt_value + 2);
        while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
            ;
#endif
        printf("sleep————\n");
        tick_delay(1);

        usart_def_init(USART2); // debug串口

        pmu_stop_mode_enter(PMU_LDO_ON, PMU_DSM_ENTRY_WFI);


        __RCU_FUNC_ENABLE(HRC_CLK);

        __RCU_FUNC_ENABLE(PLL_CLK);
        while (rcu_clkready_reset_flag_get(RCU_FLAG_PLL_STABLE) == RESET)
            ;

        rcu_sysclk_config(RCU_SYSCLK_SEL_PLL);
        while (rcu_sysclk_src_get() != 0x02)
        {
            ;
        }

        log_init();

        printf("wakeup————\n");
#if (WDG_EN)
        fwdt_reload_counter();
#endif
    }
    init_after_wakeup(state);
    memset(&sys.temp_err, 0, sizeof(sys.temp_err));
}


void app_rtc(void)
{
   // static uint32_t hour = 0;
  //  static uint32_t min = 0;
   // static uint32_t second = 0;



    if (second_flag == 1)
    {
        /* Time is 23:59:59 */
        if (__RTC_COUNTER_GET() == ((24 * 60 * 60) - 1))
        {
            /* Reset RTC Counter */
            rtc_counter_set(0);
            /* Wait until last write operation on RTC registers has finished */
            while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
                ;
        }

        cnt_value = __RTC_COUNTER_GET();

        /* Compute  hours */
     //   hour = (cnt_value / 3600);
        /* Compute minutes */
      //  min = ((cnt_value % 3600) / 60);
        /* Compute seconds */
     //   second = ((cnt_value % 3600) % 60);

     //   printf("TIME: %0.2d : %0.2d : %0.2d \r\n", hour, min, second);

        second_flag = 0;


    }
}
void app_shake_check(uint8_t io_state,uint8_t *shake,uint8_t sys_state)
{
    static uint8_t cnt = 0;
    static uint16_t clean_timer = 0;
    static uint8_t last_io_state = 0;
    



    switch (sys_state)
    {
    case STATE_ON:
        if (clean_timer < 3000 / TIME_TASK_APP_CALL)
        {
            clean_timer++;
        }
        else
        {
            cnt = 0;
            *shake = 0;
        }

        if (io_state != last_io_state)
        {
            cnt++;
            clean_timer = 0;
        }
        if (cnt > 5)
        {
            *shake = 1;
        }

        break;
    case STATE_OFF:
        last_io_state = 0;
        clean_timer = 0;
        cnt = 0;
        *shake = 0;
        break;
    default:
        break;
    }
  

    last_io_state = io_state;
}

void app_flash_data()
{
    
}

