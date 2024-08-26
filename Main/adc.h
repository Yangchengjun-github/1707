#ifndef _ADC_H
#define _ADC_H

enum
{
    CH_A_I = 0,  //AIN7
    CH_A_V = 1,  //AIN8
    CH_NUM
};


#define TIME_TASK_ADC_CALL   (5)
void adc_init_(void);

void task_adc(void);
#endif


