#include "init.h"
#include "cs32f10x_exti.h"
#include "cs32f10x_fwdt.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_pmu.h"
#include "cs32f10x_rcu.h"
#include "cs32f10x_rtc.h"
#include "cs32f10x_tim.h"
#include "cs32f10x_adc.h"
#include "adc.h"
#include "app.h"
#include "bms.h"
#include "iic.h"
#include "bms.h"
#include "led.h"
void log_init(void)
{
    usart_config_t usart_config_struct;
    
    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_USART2);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA | RCU_APB2_PERI_AFIO);
    
    gpio_mode_config(GPIOA, GPIO_PIN_2, GPIO_MODE_OUT_AFPP(GPIO_SPEED_MEDIUM));      //TX: PA2
    gpio_mode_config(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_PU);                            //RX: PA3
    
    usart_config_struct.baud_rate = 115200;
    usart_config_struct.data_width = USART_DATA_WIDTH_8;
    usart_config_struct.stop_bits = USART_STOP_BIT_1;
    usart_config_struct.parity_check = USART_PARITY_NONE;
    usart_config_struct.flow_control = USART_FLOW_CONTROL_NONE;
    usart_config_struct.transceiver_mode = USART_MODE_TX_RX;
    usart_init(USART2, &usart_config_struct);
    
    __USART_ENABLE(USART2);
}


int fputc(int ch, FILE *f)
{
    (void)f;
    while(__USART_FLAG_STATUS_GET(USART2, TXE) != SET);
    __USART_DATA_SEND(USART2, ch);
    
    return ch;
}


int fgetc(FILE *stream)
{
    uint16_t ch;
    
    (void)stream;
    while(__USART_FLAG_STATUS_GET(USART2, RXNE) == RESET);
    ch = __USART_DATA_RECV(USART2);
    
    return (ch & 0xFF);
}



void uart_init(void)
{
    usart_config_t usart_config_struct;

    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_USART3);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB | RCU_APB2_PERI_AFIO);


    gpio_mode_config(GPIOB, GPIO_PIN_10, GPIO_MODE_OUT_AFPP(GPIO_SPEED_MEDIUM));
    gpio_mode_config(GPIOB, GPIO_PIN_11, GPIO_MODE_IN_PU);

    usart_config_struct.baud_rate = 115200;
    usart_config_struct.data_width = USART_DATA_WIDTH_8;
    usart_config_struct.stop_bits = USART_STOP_BIT_1;
    usart_config_struct.parity_check = USART_PARITY_NONE;
    usart_config_struct.flow_control = USART_FLOW_CONTROL_NONE;
    usart_config_struct.transceiver_mode = USART_MODE_TX_RX;

    usart_init(USART3, &usart_config_struct);
    __USART_INTR_ENABLE(USART3, RXNE);
 //   __USART_INTR_ENABLE(USART3, IDLE);
    __USART_ENABLE(USART3);

    // USART3
    __NVIC_EnableIRQ(IRQn_USART3);
    return;
}

void uart3_to_exit(void)
{
    usart_def_init(USART3);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);

    gpio_mode_config(GPIOB, GPIO_PIN_10, GPIO_MODE_IN_PU);  //tx
    gpio_mode_config(GPIOB, GPIO_PIN_11, GPIO_MODE_IN_PU);  //rx

    /* Config exti line to pin */
    gpio_exti_pin_config(GPIO_EXTI_EVEVT_PORT_GPIOB, GPIO_EXTI_EVENT_PIN11);

    /* Config rising detect */
    __EXTI_EDGE_ENABLE(EXTI_EDGE_RISING, EXTI_LINE_11);

    /* Enable the interrupt */
    __EXTI_INTR_ENABLE(EXTI_LINE_11);

    nvic_init_t nvic_struct = {0};

    nvic_priority_group_config(NVIC_PriorityGroup_2);

    nvic_struct.nvic_irqchannel = IRQn_EXTI15_10;
    nvic_struct.nvic_irq_pre_priority = 0;
    nvic_struct.nvic_irq_sub_priority = 1;
    nvic_struct.nvic_irq_enable = ENABLE;
    nvic_init(&nvic_struct);

    nvic_struct.nvic_irqchannel = IRQn_EXTI15_10;
    nvic_init(&nvic_struct);
}





void other_io_init(void)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);

//输出
    gpio_mode_config(EN_USBA_PORT, EN_USBA_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(EN_DC_PORT, EN_DC_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(EN_G020_PORT, EN_G020_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(EN_ETA_PORT, EN_ETA_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(RST_SHUT_PORT, RST_SHUT_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));

    //bms
    gpio_mode_config(CFETOFF_PORT,CFETOFF_PIN, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH)); // --io
    gpio_mode_config(DFETOFF_PORT, DFETOFF_PIN,GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH)); // --io

	
	
	
	

    // 输入
    gpio_mode_config(WAKE_A_PORT, WAKE_A_PIN, GPIO_MODE_IN_FLOAT);
    gpio_mode_config(INT_B_PORT, INT_B_PIN, GPIO_MODE_IN_FLOAT);
}


void io_sleep_conf(uint8_t state)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);

    gpio_pin_remap_config(GPIO_REMAP_SWJ_DISABLE, ENABLE);

    /* ----------------------------------- led ---------------------------------- */
    gpio_mode_config(GPIOA, GPIO_PIN_8, GPIO_MODE_IN_PD); // led4
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_8);

    gpio_mode_config(GPIOA, GPIO_PIN_9, GPIO_MODE_IN_PD); // led3
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_9);

    gpio_mode_config(GPIOA, GPIO_PIN_10, GPIO_MODE_IN_PD); // led2
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_10);

    gpio_mode_config(GPIOA, GPIO_PIN_11, GPIO_MODE_IN_PD); // led1
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_11);

    gpio_mode_config(GPIOA, GPIO_PIN_12, GPIO_MODE_IN_PD); // led pwm
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_12);

    gpio_mode_config(GPIOA, GPIO_PIN_13, GPIO_MODE_IN_PD); // led A1
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_13);

    gpio_mode_config(GPIOA, GPIO_PIN_14, GPIO_MODE_IN_PD); // led C2
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_14);

    gpio_mode_config(GPIOB, GPIO_PIN_8, GPIO_MODE_IN_PD); // led C1
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_8);

    gpio_mode_config(GPIOB, GPIO_PIN_12, GPIO_MODE_IN_PD); // led8
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_12);

    gpio_mode_config(GPIOB, GPIO_PIN_13, GPIO_MODE_IN_PD); // led7
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_13);

    gpio_mode_config(GPIOB, GPIO_PIN_14, GPIO_MODE_IN_PD); // led6
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_14);

    gpio_mode_config(GPIOB, GPIO_PIN_15, GPIO_MODE_IN_PD); // led5
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_15);

    /* ---------------------------------- g020 ---------------------------------- */
    if(state == STATE_OFF)
    {
        gpio_mode_config(GPIOB, GPIO_PIN_1, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // en g020 //todo
        __GPIO_PIN_RESET(GPIOB, GPIO_PIN_1);

        gpio_mode_config(GPIOB, GPIO_PIN_10, GPIO_MODE_IN_PD); //! todo g020 usart 需要特殊配置

        gpio_mode_config(GPIOB, GPIO_PIN_11, GPIO_MODE_IN_PD); //! todo g020 usart 需要特殊配置

        gpio_mode_config(GPIOB, GPIO_PIN_2, GPIO_MODE_IN_PD); //! pad //todo 与 g020相连
    }
    else
    {
        gpio_mode_config(GPIOB, GPIO_PIN_1, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // en g020 //todo
        __GPIO_PIN_SET(GPIOB, GPIO_PIN_1);

        gpio_mode_config(GPIOB, GPIO_PIN_10, GPIO_MODE_IN_FLOAT); //! todo g020 usart 需要特殊配置

        gpio_mode_config(GPIOB, GPIO_PIN_11, GPIO_MODE_IN_FLOAT); //! todo g020 usart 需要特殊配置

        gpio_mode_config(GPIOB, GPIO_PIN_2, GPIO_MODE_IN_FLOAT); //! pad //todo 与 g020相连
    }

    

    /* ---------------------------------- debug --------------------------------- */
    gpio_mode_config(GPIOA, GPIO_PIN_2, GPIO_MODE_IN_PD); // debug tx//!

    gpio_mode_config(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_PD); // debug rx//!

    /* ----------------------------------- bms ---------------------------------- */
    gpio_mode_config(GPIOB, GPIO_PIN_3, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // bms CEFTOFF

    gpio_mode_config(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // bms INT

    gpio_mode_config(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // bms RST

 //   gpio_mode_config(GPIOA, GPIO_PIN_15, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // bms DEFTOFF

    if (sys.flag.bms_active == 1)
    {
        BQ769x2_RESET_DSG_OFF();
    }
    else
    {
        BQ769x2_DSG_OFF();
    }
    gpio_mode_config(GPIOB, GPIO_PIN_6, GPIO_MODE_IN_PU); // iic

    gpio_mode_config(GPIOB, GPIO_PIN_7, GPIO_MODE_IN_PU); // iic
    /* ----------------------------------- in ----------------------------------- */
    gpio_mode_config(GPIOA, GPIO_PIN_1, GPIO_MODE_IN_FLOAT); // key2  总开关
    gpio_mode_config(GPIOC, GPIO_PIN_13, GPIO_MODE_IN_FLOAT); // key 1  震动开关
    gpio_mode_config(GPIOA, GPIO_PIN_4, GPIO_MODE_IN_FLOAT); // wake_a
    //gpio_mode_config(GPIOA, GPIO_PIN_4, GPIO_MODE_IN_PD);
    /* ----------------------------------- en ----------------------------------- */
    gpio_mode_config(GPIOA, GPIO_PIN_5, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // en_usba
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_5);

    gpio_mode_config(GPIOA, GPIO_PIN_6, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // en_dc
    __GPIO_PIN_RESET(GPIOA, GPIO_PIN_6);

    gpio_mode_config(GPIOB, GPIO_PIN_9, GPIO_MODE_OUT_PP(GPIO_SPEED_LOW)); // en eta
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_9);
    /* ----------------------------------- adc ---------------------------------- */
    gpio_mode_config(GPIOA, GPIO_PIN_7, GPIO_MODE_IN_ANALOG); // adc vbusa_i
    gpio_mode_config(GPIOB, GPIO_PIN_0, GPIO_MODE_IN_ANALOG); // adc ovp
    //gpio_mode_config(GPIOA, GPIO_PIN_7, GPIO_MODE_IN_PD);     // adc vbusa_i
    //gpio_mode_config(GPIOB, GPIO_PIN_0, GPIO_MODE_IN_PD);     // adc ovp
    /* --------------------------------- unused --------------------------------- */

    gpio_mode_config(GPIOA, GPIO_PIN_0, GPIO_MODE_IN_PD);  

    gpio_mode_config(GPIOC, GPIO_PIN_14, GPIO_MODE_IN_PD);                     
    gpio_mode_config(GPIOC, GPIO_PIN_15, GPIO_MODE_IN_PD);                     

    gpio_mode_config(GPIOD, GPIO_PIN_0, GPIO_MODE_IN_PD);
    gpio_mode_config(GPIOD, GPIO_PIN_1, GPIO_MODE_IN_PD);
    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */
    /* -------------------------------------------------------------------------- */
    
}


void tick_init(void)
{
	//systick_clk_src_config(SysTick_CLKSource_HCLK);
    if(SysTick_Config(SystemCoreClock / 1000)) 
    {
        while(1);
    }
}



void tim_init(void)
{
    tim_base_t ptr_time = {0};

    /* PCLK1 = HCLK/2 */
   // rcu_pclk1_config(RCU_HCLK_DIV_2);
    /* PCLK2 = HCLK/2 */
  //  rcu_pclk2_config(RCU_HCLK_DIV_2);

    /* TIM clock enable */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_TIM1);
    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_TIM2);

    /* Time base configuration */
    ptr_time.clk_div = TIM_CLK_DIV1;
    ptr_time.cnt_mode = TIM_CNT_MODE_DOWN;
    ptr_time.period = 100;  
    ptr_time.pre_div = 63;
    tim_base_init(TIM1, &ptr_time);
    //ptr_time.period = 20000;
    //tim_base_init(TIM2, &ptr_time);

    /* TIM interrupt enable */
    __TIM_INTR_ENABLE(TIM1, TIM_INTR_UPDATE);
    //__TIM_INTR_ENABLE(TIM2, TIM_INTR_UPDATE);

    /* TIM enable */
    __TIM_ENABLE(TIM1);
    //__TIM_ENABLE(TIM2);

    nvic_init_t ptr_nvic;

    /* Configure and enable TIM1 interrupt. */
    ptr_nvic.nvic_irqchannel = IRQn_TIM1_UP;
    ptr_nvic.nvic_irq_enable = ENABLE;
    ptr_nvic.nvic_irq_pre_priority = 0;
    ptr_nvic.nvic_irq_sub_priority = 0;
    nvic_init(&ptr_nvic);
}

void exti_init(uint8_t mode)
{

    exti_def_init();
    /* Enable the GPIOC clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    /* Enable the AFIO clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);
    // EXTI
    nvic_init_t nvic_struct = {0};
    nvic_struct.nvic_irq_pre_priority = 0;
    nvic_struct.nvic_irq_sub_priority = 1;
    nvic_struct.nvic_irq_enable = ENABLE;

    /* Config rising detect */
    __EXTI_EDGE_ENABLE(EXTI_EDGE_RISING, EXTI_LINE_17);

    /* Enable the interrupt */
    __EXTI_INTR_ENABLE(EXTI_LINE_17);

    nvic_priority_group_config(NVIC_PriorityGroup_2);
    switch(mode)
    {
    case STATE_ON:

/* --------------------------------- A口插入唤醒 --------------------------------- */
        gpio_mode_config(WAKE_A_PORT, WAKE_A_PIN, GPIO_MODE_IN_FLOAT);
        gpio_exti_pin_config(GPIO_EXTI_EVEVT_PORT_GPIOA, GPIO_EXTI_EVENT_PIN4);
        __EXTI_EDGE_ENABLE(EXTI_EDGE_RISING, EXTI_LINE_4);

        nvic_struct.nvic_irqchannel = IRQn_EXTI4;
        __EXTI_INTR_ENABLE(EXTI_LINE_4);
        nvic_init(&nvic_struct);
        

/* --------------------------------- 震动开关唤醒 --------------------------------- */
        gpio_mode_config(KEYB_PORT, KEYB_PIN, GPIO_MODE_IN_FLOAT);
        gpio_exti_pin_config(GPIO_EXTI_EVEVT_PORT_GPIOC, GPIO_EXTI_EVENT_PIN13);
        __EXTI_EDGE_ENABLE(EXTI_EDGE_FALLING, EXTI_LINE_13);

        nvic_struct.nvic_irqchannel = IRQn_EXTI15_10;
        __EXTI_INTR_ENABLE(EXTI_LINE_13);
        nvic_init(&nvic_struct);
        
        // break; //todo 不写break
    case STATE_OFF:
/* --------------------------------- 系统开关唤醒 --------------------------------- */
        gpio_mode_config(KEYA_PORT, KEYA_PIN, GPIO_MODE_IN_FLOAT);

        gpio_exti_pin_config(GPIO_EXTI_EVEVT_PORT_GPIOA, GPIO_EXTI_EVENT_PIN1);

        __EXTI_EDGE_ENABLE(EXTI_EDGE_FALLING, EXTI_LINE_1);

        __EXTI_INTR_ENABLE(EXTI_LINE_1);

        nvic_struct.nvic_irqchannel = IRQn_EXTI1; //key

        nvic_init(&nvic_struct);
        break;
    }

}

void fwdt_init(void)   //5s
{
    fwdt_write_access_enable_ctrl(FWDT_WRITE_ACCESS_ENABLE);
    fwdt_prescaler_set(FWDT_PRESCALER_256);
    fwdt_reload_set(781);
    fwdt_enable();
}

#define AM_8 (8 * 60 * 60)
#define TIME_RST ((24 * 60 * 60) - 1)

extern uint32_t cnt_value ;
void rtc_config(void)
{
    /* Enable PMU clock */
    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_PMU);

    /* Allow access to RTC Domain */
    pmu_vbat_domain_write_config(ENABLE);

    /* Enable LXT clock */
    // __RCU_FUNC_ENABLE(LXT_CLK);
    (RCU->STS)  |= RCU_STS_LRCEN;
//     /* Wait till LXT is ready */
     while (RESET == rcu_clkready_reset_flag_get(RCU_FLAG_LRC_STABLE))
//   //      ;

    /* Select LXT as RTC Clock Source */
    rcu_rtcclk_config(RCU_RTCCLK_SEL_LRC);

    /* Enable RTC Clock */
    __RCU_RTC_CLK_ENABLE();

    /* Wait for RTC registers synchronization */
    rtc_wait_for_synchronize();
    /* Wait until last write operation on RTC registers has finished */
    while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
        ;

    /* Enable the RTC Second interrupt*/
    __RTC_INTERRUPT_ENABLE(RTC_INTERRUPT_SECOND | RTC_INTERRUPT_ALARM);
    /* Wait until last write operation on RTC registers has finished */
    while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
        ;

    /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
    rtc_prescaler_set(39999);
    /* Wait until last write operation on RTC registers has finished */
    while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
        ;

    /* Time starts at 8 o'clock in the morning */
    rtc_counter_set(AM_8);
    /* Wait until last write operation on RTC registers has finished */
    while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
        ;

    nvic_init_t ptr_nvic = {0};

    /* Configure and enable RTC interrupt. */
    ptr_nvic.nvic_irqchannel = IRQn_RTC;
    ptr_nvic.nvic_irq_enable = ENABLE;
    ptr_nvic.nvic_irq_pre_priority = 0;
    ptr_nvic.nvic_irq_sub_priority = 0;
    nvic_init(&ptr_nvic);

    ptr_nvic.nvic_irqchannel = IRQn_RTCAlarm;
    ptr_nvic.nvic_irq_enable = ENABLE;
    ptr_nvic.nvic_irq_pre_priority = 1;
    ptr_nvic.nvic_irq_sub_priority = 0;
    nvic_init(&ptr_nvic);

    /* Config rising detect */
    __EXTI_EDGE_ENABLE(EXTI_EDGE_RISING, EXTI_LINE_17);

    /* Enable the interrupt */
    __EXTI_INTR_ENABLE(EXTI_LINE_17);

    rtc_alarm_set(AM_8 +2);
    while (__RTC_FLAG_STATUS_GET(RTC_FLAG_OPERATION_COMPLETE) == RESET)
        ;
}

void deinit_befor_sleep(uint8_t state)
{
    
    usart_def_init(USART3);
    usart_def_init(USART2);
    tim_def_init(TIM1);
    exti_init(state);  //!这个影响rtc闹钟中断唤醒系统
    if(state == STATE_ON)
    {
        uart3_to_exit();
    }
    else 
    {
       
    }
    __ADC_DEF_INIT(ADC1);
    __ADC_DISABLE(ADC1);
    if (state == STATE_ON)
    {
        BQ769x2_RESET_DSG_OFF();
        BQ769x2_RESET_CHG_OFF();
    }
    else
    {
        BQ769x2_DSG_OFF(); // 放电禁止
        BQ769x2_CHG_OFF(); // 充电禁止
        CommandSubcommands(DEEPSLEEP);
        CommandSubcommands(DEEPSLEEP);
    }

    io_sleep_conf(state);
   

}

void init_after_wakeup(uint8_t last_state)
{
    log_init();
    uart_init();
    adc_init_(0);
    tim_init();
    i2c_init_2();
    other_io_init();
    led_init();
    // if (last_state == STATE_ON)
    // {
        
    // }
    // else
    {
        CommandSubcommands(EXIT_DEEPSLEEP);
       // bq76942_reset();

        BQ769x2_RESET_CHG_OFF();
        if (sys.flag.bms_active == 1)
        {
            BQ769x2_RESET_DSG_OFF();
        }
        else
        {
            BQ769x2_DSG_OFF();
        }
            
    }
}
