#include "communication.h"

_rx     rx_CB      = {0} ;
uint8_t receive_ok = 0;
volatile uint8_t rx_cnt;
uint8_t rx_buffer[sizeof(T_field_receive)];
#pragma pack(1)

    T_field_Transmit field_Transmit;
    T_field_receive field_receive;
#pragma pack()
    cmd_G020_t cmd_g020;
void task_com_rx(void)
{
    ProcessData1(&rxBuffer);
}
void cmd_g020_write(cmd_G020_t cmd)
{
    cmd_g020 = cmd;
}

cmd_G020_t cmd_g020_get(void)
{
    return cmd_g020;
}
void task_com_tx(void)
{
    uint8_t i;
    if (rx_CB.ack == 0)
    {
        if (rx_CB.ackDelay++ >= 500 / TIME_TASK_COMM_TX_CALL)
        {
            rx_CB.repTime++;
            rx_CB.rep = 1;
        }
    }
    else
    {
        rx_CB.repTime = 0;
        rx_CB.rep = 0;
    }


    memset(&field_Transmit.byte, 0, sizeof(T_field_Transmit));
    field_Transmit.parameter.head                              = 0xB7;
    field_Transmit.parameter.un_cmd1.bit_field.bms             = sys.flag.bms_active;
    field_Transmit.parameter.un_cmd1.bit_field.cmd_G020        = cmd_g020;                              //EN_CHARGE_EN_DISCHAR;
    field_Transmit.parameter.un_cmd1.bit_field.usba            = sys.port.A1_status_To_g020;
    field_Transmit.parameter.un_cmd2.bit_field.charge_level    = sys.port.charge_powerdowm ;
    field_Transmit.parameter.un_cmd2.bit_field.discharge_level = sys.port.discharge_powerdown;

    for (i = 0; i < sizeof(T_field_Transmit) - 2; i++)
    {
        field_Transmit.parameter.sum1 += (&field_Transmit.byte)[i];
    }
	field_Transmit.parameter.sum2 = field_Transmit.parameter.sum1 ^ 0xff;
    while (__USART_FLAG_STATUS_GET(USART3, TXE) != SET)
    {
    }
	if(1)
	{
		receive_ok = 0;
		transmit_data((uint8_t *)&field_Transmit, sizeof(T_field_Transmit));
	}
    
}

void data_printf(uint8_t *p , uint8_t len)
{
	uint8_t i = 0;
	printf("s_len:%d \n",len);
	for(i= 0; i<len; i++)
	{
		printf("%x ",p[i]);
	}
	printf("\n");
}

void  transmit_data(uint8_t *data, uint8_t len)
{
    uint8_t i = 0;
    for(i = 0; i < len; i++)
    {
        while (__USART_FLAG_STATUS_GET(USART3, TXE) != SET);
        __USART_DATA_SEND(USART3, data[i]);
    }
}






