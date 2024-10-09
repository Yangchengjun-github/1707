#include "adc.h"
#include "cs32f10x_adc.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#include "app.h"
void adc_init_(uint8_t cal)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    gpio_mode_config(GPIOA, GPIO_PIN_7, GPIO_MODE_IN_ANALOG); // VBUSA_I  //A口电流
    gpio_mode_config(GPIOB, GPIO_PIN_0, GPIO_MODE_IN_ANALOG); // OVP-A  //A口电压

    /* Configure ADC1 clock. */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_ADC1);
    rcu_adcclk_config(RCU_ADCCLK_SEL_PCLK2_DIV8);
    adc_cfg_t ptr_cfg;
    /* Configure the basic information of ADC1. */
    __ADC_DEF_INIT(ADC1);
    adc_struct_init(&ptr_cfg);
    ptr_cfg.ext_trigger = ADC_EXT_TRIGGER_SWSTART;
    adc_init(ADC1, &ptr_cfg);
    if(cal == 1)
    {
        // calibration
        adc_regular_channel_config(ADC1, ADC_CHANNEL_7, ADC_SAMPLE_TIME_55_5_CYCLE, 1);

        /* Enable ADC1. */
        __ADC_ENABLE(ADC1);

        /* Enable ADC1 reset calibration register. */
        __ADC_RESET_CALI(ADC1);
        /* Check the end of ADC1 reset calibration register. */
        while (__ADC_RESET_CALI_STATUS_GET(ADC1))
            ;

        /* Start ADC1 calibration. */
        __ADC_CALI_START(ADC1);
        /* Check the end of ADC1 calibration. */
        while (__ADC_CALI_STATUS_GET(ADC1))
            ;
    }
   

    /* ADC1 regular channel10 configuration. */
    adc_regular_channel_config(ADC1, ADC_CHANNEL_8, ADC_SAMPLE_TIME_55_5_CYCLE, 1);

    /* Enable ADC1. */
    __ADC_ENABLE(ADC1);

    /* Enable ADC1 reset calibration register. */
    //    __ADC_RESET_CALI(ADC1);
    /* Check the end of ADC1 reset calibration register. */
         while (__ADC_RESET_CALI_STATUS_GET(ADC1))
             ;

    /* Start ADC1 calibration. */
         __ADC_CALI_START(ADC1);
    /* Check the end of ADC1 calibration. */
        while (__ADC_CALI_STATUS_GET(ADC1))
           ;
}


void task_adc(void)
{
    uint8_t ch = 0;
    for(ch = 0; ch < CH_NUM; ch++)
    {
        switch (ch)
        {
        case CH_A_I:
			#if 1
            /* ADC1 regular channel10 configuration. */
            adc_regular_channel_config(ADC1, ADC_CHANNEL_7, ADC_SAMPLE_TIME_55_5_CYCLE, 1);

            /* Enable ADC1. */
       //     __ADC_ENABLE(ADC1);

            /* Enable ADC1 reset calibration register. */
       //     __ADC_RESET_CALI(ADC1);
            /* Check the end of ADC1 reset calibration register. */
       //     while (__ADC_RESET_CALI_STATUS_GET(ADC1))
       //         ;

            /* Start ADC1 calibration. */
       //     __ADC_CALI_START(ADC1);
            /* Check the end of ADC1 calibration. */
         //   while (__ADC_CALI_STATUS_GET(ADC1))
          //      ;

            __ADC_REG_CONV_START(ADC1);

            /* Wait for the conversion to complete. */
            while (RESET == __ADC_FLAG_STATUS_GET(ADC1, ADC_FLAG_EOC))
                ;

            /* Calculate the voltage value. */
            sys.adc.value[CH_A_I]= __ADC_CONV_VALUE_GET(ADC1);
            sys.adc.conver[CH_A_I] = (sys.adc.value[CH_A_I] / 4095.0 * 3300ul)  * 100 * 0.9;//todo 0.9是补偿
			#endif
            break;
        case CH_A_V:
			#if 1
            /* ADC1 regular channel10 configuration. */
            adc_regular_channel_config(ADC1, ADC_CHANNEL_8, ADC_SAMPLE_TIME_55_5_CYCLE, 1);

            /* Enable ADC1. */
        //    __ADC_ENABLE(ADC1);

            /* Enable ADC1 reset calibration register. */
        //    __ADC_RESET_CALI(ADC1);
            /* Check the end of ADC1 reset calibration register. */
       //     while (__ADC_RESET_CALI_STATUS_GET(ADC1))
       //         ;

            /* Start ADC1 calibration. */
       //     __ADC_CALI_START(ADC1);
            /* Check the end of ADC1 calibration. */
       //     while (__ADC_CALI_STATUS_GET(ADC1))
       //         ;

            __ADC_REG_CONV_START(ADC1);

            /* Wait for the conversion to complete. */
            while (RESET == __ADC_FLAG_STATUS_GET(ADC1, ADC_FLAG_EOC))
                ;

            /* Calculate the voltage value. */
            sys.adc.value[CH_A_V] = __ADC_CONV_VALUE_GET(ADC1);
            sys.adc.conver[CH_A_V] = sys.adc.value[CH_A_V] / 4095.0 * 3300ul*16.45 ;
#endif
            break;
        }
    }
    

    
    /* Start ADC1 Software Conversion. */
   
}