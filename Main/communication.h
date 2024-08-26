#ifndef _COMMUNI_
#define _COMMUNI_


#include "cs32f10x_usart.h"
#include "key.h"
#include "queue.h"
#include "string.h"
#include "app.h"


#define TIME_TASK_COMM_RX_CALL (100)
#define TIME_TASK_COMM_TX_CALL (100)



typedef enum
{

    INACTICE = 0,
    ACTICVE
}bms_t;

typedef enum
{
    OFF = 0,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4
}level_t;



typedef enum
{
    DIS_CHARGE_DIS_DISCHAR = 0,             //禁止充放电
    EN_CHARGE_DIS_DISCHAR,                  //允许充电，禁止放电
    DIS_CHARGE_EN_DISCHAR,                  //禁止充电，允许放电
    EN_CHARGE_EN_DISCHAR,                   //允许充电，允许放电
}cmd_G020_t;
typedef struct
{
	uint8_t receiveBuffer[20];
	uint8_t receiveCnt;
	uint16_t Overtimer;
	uint8_t ackDelay;
	uint8_t repTime;
	uint8_t ack;
	uint8_t rep;
}_rx;
extern _rx rx_CB ;

#pragma pack(1)
typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t head;
        union 
        {
            uint8_t byte;
            struct
            {
                bms_t bms : 1;
                cmd_G020_t cmd_G020 : 3;
                port_Astatus_t usba : 1;
                uint8_t : 3;
            } bit_field;
        }un_cmd1;
        union
        {
            uint8_t byte;
            struct
            {
                level_t charge_level : 4;
                level_t discharge_level : 4;
            } bit_field;
        } un_cmd2;

        uint8_t sum1;
        uint8_t sum2;
    } parameter;
} T_field_Transmit;
#pragma pack()

#pragma pack(1)
typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t head;
        union 
        {
            uint8_t byte;
            struct 
            {
                port_Cstatus_t ca_status : 2;
                port_Cstatus_t cb_status : 2;
                port_PGstatus_t pg_status : 2;
                uint8_t : 2;

            }bit_field;
        }un_cmd1;
        uint8_t sum1;
        uint8_t sum2;

    } parameter;
} T_field_receive;

#pragma pack()

extern T_field_Transmit field_Transmit;
extern T_field_receive field_receive;
extern uint8_t rx_buffer[];
extern uint8_t receive_ok ;
void task_com_rx(void);
void task_com_tx(void);




void data_printf(uint8_t *p , uint8_t len);

void transmit_data(uint8_t *data, uint8_t len);

#endif

