#ifndef _APP_H
#define _APP_H

#define TIME_TASK_APP_CALL (10)
#include "stdint.h"
typedef enum
{
    C_IDLE = 0,
    C_DISCHARGE, 
    C_PROTECT,
} port_Cstatus_t;
typedef enum
{
    A_IDLE = 0,
    A_DISCHARGE,
	A_PROTECT
} port_Astatus_t;
typedef enum
{
    PG_IDLE = 0,
    PG_CHARGE,
    PG_PROTECT,
} port_PGstatus_t;
typedef struct 
{
    enum
    {
        STATE_OFF = 0,
        STATE_ON
    }state;
    struct 
    {
        uint8_t powON :1;
        uint8_t powOFF : 1;
    }cmd; 
    struct 
    {
        uint16_t value[5];
        uint16_t conver[5];
    }adc;
    struct 
    {
        uint8_t cap;
        uint8_t health;

    }bat;
    struct 
    {
        /* data */
    }abnormal;
    struct 
    {
        port_Astatus_t A1_status;
        port_Cstatus_t C1_status;
        port_Cstatus_t C2_status;
        port_PGstatus_t PG_status;

        uint8_t dis_output :1;
        uint8_t a_exit :1;
		struct 
		{
			uint8_t oc :1;
			uint8_t ov :1;
		}porta_fault;
        struct 
        {
            void (*usbaOpen)(void);
            void (*usbaClose)(void);
			void (*usbaFault)(void);
        }method;
        
       
    }port;
    
    
    
    
}sys_t;

typedef struct
{
	uint16_t nack_cnt;
	uint16_t ack_total;
}xbms_t;


extern xbms_t xbms;
extern sys_t sys;


#endif

void task_app(void);
