#include "init.h"
#include "cs32f10x_exti.h"
#include "cs32f10x_fwdt.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_tim.h"
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
    __USART_ENABLE(USART3);
    return;
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

void tick_init(void)
{
	//systick_clk_src_config(SysTick_CLKSource_HCLK);
    if(SysTick_Config(SystemCoreClock / 1000)) 
    {
        while(1);
    }
}

void nvic_configuration(void)
{	
	//USART3
    __NVIC_EnableIRQ(IRQn_USART3);
	//TIM1
    nvic_init_t ptr_nvic;

    /* Configure and enable TIM1 interrupt. */
    ptr_nvic.nvic_irqchannel = IRQn_TIM1_UP;
    ptr_nvic.nvic_irq_enable = ENABLE;
    ptr_nvic.nvic_irq_pre_priority = 0;
    ptr_nvic.nvic_irq_sub_priority = 0;
    nvic_init(&ptr_nvic);

    //EXTI
    nvic_init_t nvic_struct = {0};

    nvic_priority_group_config(NVIC_PriorityGroup_2);

    nvic_struct.nvic_irqchannel = IRQn_EXTI4;
    nvic_struct.nvic_irq_pre_priority = 0;
    nvic_struct.nvic_irq_sub_priority = 1;
    nvic_struct.nvic_irq_enable = ENABLE;
    nvic_init(&nvic_struct);

    nvic_struct.nvic_irqchannel = IRQn_EXTI1;
    nvic_init(&nvic_struct);
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
    ptr_time.pre_div = 71;
    tim_base_init(TIM1, &ptr_time);
    //ptr_time.period = 20000;
    //tim_base_init(TIM2, &ptr_time);

    /* TIM interrupt enable */
    __TIM_INTR_ENABLE(TIM1, TIM_INTR_UPDATE);
    //__TIM_INTR_ENABLE(TIM2, TIM_INTR_UPDATE);

    /* TIM enable */
    __TIM_ENABLE(TIM1);
    //__TIM_ENABLE(TIM2);
}

void exti_init(void)
{
    //for wake a
    /* Enable the GPIOC clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    /* Enable the AFIO clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_AFIO);

    /* PC13 -- button */
    gpio_mode_config(WAKE_A_PORT, WAKE_A_PIN, GPIO_MODE_IN_FLOAT);

    /* Config exti line to pin */
    gpio_exti_pin_config(GPIO_EXTI_EVEVT_PORT_GPIOA, GPIO_EXTI_EVENT_PIN4);

    /* Config rising detect */
    __EXTI_EDGE_ENABLE(EXTI_EDGE_RISING, EXTI_LINE_4);

    /* Enable the interrupt */
    __EXTI_INTR_ENABLE(EXTI_LINE_4);

    //for key2 (总开关)
    /* Enable the GPIOC clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);
    /* Config exti line to pin */
    gpio_exti_pin_config(GPIO_EXTI_EVEVT_PORT_GPIOA, GPIO_EXTI_EVENT_PIN1);
    /* Config rising detect */
    __EXTI_EDGE_ENABLE(EXTI_EDGE_FALLING, EXTI_LINE_1);
    /* Enable the interrupt */
    __EXTI_INTR_ENABLE(EXTI_LINE_1);
}

void fwdt_init(void)
{
    fwdt_write_access_enable_ctrl(FWDT_WRITE_ACCESS_ENABLE);
    fwdt_prescaler_set(FWDT_PRESCALER_4);
    fwdt_reload_set(0xFFF);
    fwdt_enable();
}