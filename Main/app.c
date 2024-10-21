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

// #include "communication.h"

extern __IO uint8_t second_flag;
static uint8_t portA_plug_check(void);

void key_fast_switch(uint8_t key_level);

void sys_switch_check(void);

void sys_pow_on_off_deal(void);

void app_led_control(void);

void app_usba_control_protect(void);

void f_uaba_open(void);

void f_uaba_close(void);

void f_uaba_fault_ov(void);

void f_uaba_fault_oc(void);

void eta_driver(void);

void app_power_rank_contorl(void);

void app_temperature_protect(void);

void app_power_sw_contorl(void);

void app_sleep(uint8_t);

void app_bms_comm_recover(void);

void app_eta_control(void);

void app_bms_charge_to_active(void);

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
    sys_switch_check();
    sys_pow_on_off_deal();
    app_rtc();
    key_fast_switch(KEY2_IO_LEVEL);
    app_led_control();
    app_usba_control_protect();
    app_power_rank_contorl(); // control g020
    app_power_sw_contorl();   // control usba
    app_temperature_protect();
    app_eta_control();

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

    static uint8_t cnt = 0;
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
    if (num > 5)
    {
        // printf("----------------------\n");
        sys.flag.health_trig = 1;
        num = 0;
    }
    last_level = key_level;
}

void sys_switch_check(void)
{
   
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
        if (sys.state == STATE_ON) // 关机
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
        if (sys.state == STATE_OFF) // 开机
        {

            led.bat.method.pf_led_show_battery(NULL);
            sys.state = STATE_ON;
            G020_ON;
        }
    }
}
inline void app_led_control()
{
    uint16_t warnig_time = 0;
    uint8_t err_cnt = 0;
    // bat led
    static uint8_t err_mode = 0;
    switch (sys.state)
    {
    case STATE_OFF:

        led.bat.method.pf_led_alloff(NULL);

        break;
    case STATE_ON:
        if (sys.flag.iic_err == 1) // IIC 通讯错误
        {

            err_mode = 0;
            led.bat.method.pf_led_err(&err_mode);
        }

        else if (sys.flag.bms_active == 0) // 锂保未激活
        {

            err_mode = 1;
            led.bat.method.pf_led_err(&err_mode);
        }
        else if ((sys.temp_err.charge_otp || sys.temp_err.charge_utp || sys.temp_err.discharge_otp || sys.temp_err.discharge_utp)) // 温度异常
        {

            led.bat.method.pf_led_warning(&warnig_time);
            printf("%d,%d,%d,%d\n", sys.temp_err.charge_otp, sys.temp_err.charge_utp, sys.temp_err.discharge_otp, sys.temp_err.discharge_utp);
        }

        else if (sys.port.PG_status == PG_CHARGE) // 充电
        {

            led.bat.method.pf_led_charge(NULL);
        }
        else if (sys.port.C1_status == C_DISCHARGE || sys.port.C2_status == C_DISCHARGE || sys.port.A1_status == A_DISCHARGE) // 放电
        {
            if (sys.bat.cap == 0)
            {
            }

            led.bat.method.pf_led_discharge(NULL);
        }
        else if (sys.flag.health_trig) // 电池健康
        {
            sys.flag.health_trig = 0;
            led.bat.method.pf_led_health(NULL);
        }
        else
        {
            if (led.bat.status == LED_CHARGE) // 充电拔除
            {
                led.bat.method.pf_led_show_battery(NULL);
            }
            else if (led.bat.status != LED_SHOW_BATTERY && led.bat.status != LED_HEALTH) // 熄灭
            {

                led.bat.method.pf_led_alloff(NULL);
            }
        }
        break;
    default:
        break;
    }

    // port led
    if (sys.port.C1_status == C_PROTECT || sys.port.C2_status == C_PROTECT ||
        sys.port.PG_status == PG_PROTECT || sys.port.A1_status == A_PROTECT ||
        sys.temp_err.charge_otp || sys.temp_err.charge_utp ||
        sys.temp_err.discharge_otp || sys.temp_err.discharge_utp)
    {
        err_cnt++;
        if (err_cnt > 300 / TIME_TASK_APP_CALL)
        {
            err_cnt = 0;

            printf("!!!!!!!!!!!!!!!!!!!!!!%d\n", __LINE__);
            led.port.method.pf_led_warning(NULL);
        }
    }
    else
    {
        err_cnt = 0;

        led.port.method.pf_led_normal(NULL);
    }
}

void app_usba_control_protect(void)
{
    static uint32_t small_cur_cnt = 0;
    static uint8_t oc_cnt = 0;

    static uint32_t recover_cnt = 0;
    switch (sys.state)
    {
    case STATE_OFF:
        if (sys.port.A1_status != A_IDLE)
            sys.port.method.usbaClose();

        break;
    case STATE_ON:
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
        }
        else
        {
            recover_cnt = 0;
        }

        if (sys.port.PG_status == PG_CHARGE || sys.port.PG_status == PG_PROTECT) // 充电或者错误关闭A口
        {
            if (sys.port.A1_status != A_IDLE)
                sys.port.method.usbaClose();
        }

        else if (portA_plug_check() && !sys.port.dis_output && sys.flag.bms_active) // 端口A 插入负载
        {
            if (sys.port.A1_status != A_DISCHARGE)
                sys.port.method.usbaOpen();

            // printf("PortA plug in\r\n");
        }
        else
        {
        }
#if 0		
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
#endif
        if (sys.adc.conver[CH_A_I] > 1800 && sys.adc.conver[CH_A_V] > 10000) // 过流保护2
        {
            oc_cnt++;
            if (oc_cnt > 100 / TIME_TASK_APP_CALL)
            {
                oc_cnt = 0;
                sys.port.porta_fault.oc = 1;
                if (sys.port.A1_status != A_PROTECT)
                    sys.port.method.usbaFault();
            }
        }
        else
        {
            oc_cnt = 0;
        }

        if (sys.adc.conver[CH_A_V] > 13500) // A口过压保护
        {
            sys.port.porta_fault.ov = 1;
            if (sys.port.A1_status != A_PROTECT)
                sys.port.method.usbaFault();
        }

        if (sys.port.dis_output)
        {
            if (sys.port.A1_status != A_IDLE)
                sys.port.method.usbaClose();
        }
        if (sys.port.A1_status == A_DISCHARGE)
        {
            if (sys.adc.conver[CH_A_I] < 80) // 小电流  //adc 1 对应 80ma
            {
                sys.flag.aPort_low_current = 1;
                small_cur_cnt++;
                if (small_cur_cnt > 2 * 60 * 60 * 1000ul / TIME_TASK_APP_CALL)
                {
                    small_cur_cnt = 0;
                    if (sys.port.A1_status != A_IDLE)
                        sys.port.method.usbaClose();
                }
            }
            else
            {
                if (sys.flag.aPort_low_current == 1)
                {
                    if (sys.adc.conver[CH_A_I] > 200)
                    {
                        sys.flag.aPort_low_current = 0;
                    }
                }
                small_cur_cnt = 0;
                small_cur_cnt = 0;

                small_cur_cnt = 0;
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
}
void f_uaba_close(void)
{
    LOG(LOG_LEVEL_INFO, "\n");
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

void app_temperature_protect(void)
{
    static uint16_t rep[4] = {0};
    if (sys.flag.temp_scan == 0)
        return;
    // chage otp
    if (bms_tmp1 > CHA_OTP_PROTECT || bms_tmp2 > CHA_OTP_PROTECT || bms_tmp3 > CHA_OTP_PROTECT)
    {
        if (rep[0]++ > 2000 / TIME_TASK_APP_CALL)
        {
            sys.temp_err.charge_otp = 1;
        }
    }
    else
    {
        rep[0] = 0;
        sys.temp_err.charge_otp = 0;
    }

    // charge utp
    if (bms_tmp1 < CHA_UTP_PROTECT || bms_tmp2 < CHA_UTP_PROTECT || bms_tmp3 < CHA_UTP_PROTECT)
    {
        if (bms_tmp1 == 0)
        {
            rep[1] = 0; // todo  bms需要温度测量需要一段时间，未准备好时，读出为0
        }
        if (rep[1]++ > 4000 / TIME_TASK_APP_CALL)
            sys.temp_err.charge_utp = 1;
    }
    else
    {
        rep[1] = 0;
        sys.temp_err.charge_utp = 0;
    }

    // dis otp
    if (bms_tmp1 > DISC_OTP_PROTECT || bms_tmp2 > DISC_OTP_PROTECT || bms_tmp3 > DISC_OTP_PROTECT)
    {
        if (rep[2]++ > 2000 / TIME_TASK_APP_CALL)
            sys.temp_err.discharge_otp = 1;
    }
    else
    {
        rep[2] = 0;
        sys.temp_err.discharge_otp = 0;
    }

    // dis utp
    if (bms_tmp1 < DISC_UTP_PROTECT || bms_tmp2 < DISC_UTP_PROTECT || bms_tmp3 < DISC_UTP_PROTECT)
    {
        if (bms_tmp1 == 0)
        {
            rep[3] = 0; // todo  bms需要温度测量需要一段时间，未准备好时，读出为0
        }
        if (rep[3]++ > 2000 / TIME_TASK_APP_CALL)
            sys.temp_err.discharge_utp = 1;
    }
    else
    {
        rep[3] = 0;
        sys.temp_err.discharge_utp = 0;
    }
}
void app_power_sw_contorl(void)
{
    switch (sys.state)
    {
    case STATE_OFF:

        break;
    case STATE_ON:
        /* -------------------------------------------------------------------------- */
        /* --------------------------------- 正常情况 -------------------------------- */
        /* -------------------------------------------------------------------------- */
        sys.port.dis_output = 0;
        cmd_g020_write(EN_CHARGE_EN_DISCHAR);
        /* -------------------------------------------------------------------------- */
        /* ---------------------------------- 异常情况 ---------------------------------- */
        /* -------------------------------------------------------------------------- */
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
        else if (sys.temp_err.discharge_otp || sys.temp_err.discharge_utp)
        {
            cmd_g020_write(EN_CHARGE_DIS_DISCHAR);
#if (BME_EN)
            sys.port.dis_output = 1;
#endif
        }
        else
        {
            // nothing
        }

        if (sys.bat.cap == 0)
        {
            cmd_g020_write(EN_CHARGE_DIS_DISCHAR);
            sys.port.dis_output = 1;
        }
        if (sys.port.PG_status != PG_IDLE)
        {
            sys.port.dis_output = 1;
        }
        break;
    default:
        break;
    }
}
void app_sleep(uint8_t state)
{

    static uint16_t delay_off;
    static uint16_t delay_on;
    switch (state)
    {
    case STATE_OFF:
        delay_on = 0;
        if (delay_off++ > 3000 / TIME_TASK_APP_CALL) // 需大于开关快速的判断时间
        {
            delay_off = 0;
            printf("sleep1\n");
            sleep_del(state);
            printf("wakeup1\n");
        }

        break;
    case STATE_ON:
        if (sys.uart3_idle_cntdown == 0 && sys.port.C1_status == C_IDLE && sys.port.C2_status == C_IDLE && sys.port.A1_status == A_IDLE && sys.port.PG_status == PG_IDLE)
        {
            if (delay_on++ > 3000 / TIME_TASK_APP_CALL)
            {
                delay_on = 0;
                printf("sleep2\n");
                sleep_del(state);
                printf("wakeup2\n");
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


#define test (0)
#if test
    BQ769x2_RESET_DSG_OFF();
#else
    if (sys.flag.bms_active == 0 && (sys.port.PG_status == PG_CHARGE || JUMP_ACTIVE == 1))
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
    while (sys.flag.wake_aport == 0 && sys.flag.wake_key == 0 && sys.flag.wake_usart == 0) // 如果不是这3个中断唤醒，则继续休眠
    {
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
    init_after_wakeup();
}


void app_rtc(void)
{
    static uint32_t hour = 0;
    static uint32_t min = 0;
    static uint32_t second = 0;



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
        hour = (cnt_value / 3600);
        /* Compute minutes */
        min = ((cnt_value % 3600) / 60);
        /* Compute seconds */
        second = ((cnt_value % 3600) % 60);

        printf("TIME: %0.2d : %0.2d : %0.2d \r\n", hour, min, second);

        second_flag = 0;


    }
}
