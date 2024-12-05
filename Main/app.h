#ifndef _APP_H
#define _APP_H


#define TIME_TASK_APP_CALL (10)
#include "stdint.h"


//uint 0.1C
#define TEMPERATURE_TH1_C  (580)    //uint 摄氏度
#define TEMPERATURE_TH2_C  (500)
#define TEMPERATURE_TH3_C  (500)
#define TEMPERATURE_TH4_C  (500)

#define TEMPERATURE_TH1_K (TEMPERATURE_TH1_C + 2730)
#define TEMPERATURE_TH2_K (TEMPERATURE_TH2_C + 2730)
#define TEMPERATURE_TH3_K (TEMPERATURE_TH3_C + 2730)
#define TEMPERATURE_TH4_K (TEMPERATURE_TH4_C + 2730)

#define CHA_UTP_PROTECT (20 + 2730) //2摄氏度
#define CHA_UTP_RECOVER (40 + 2730) //4摄氏度

#define CHA_OTP_PROTECT (630 + 2730) //63摄氏度
#define CHA_OTP_RECOVER (590 + 2730)

#define DISC_UTP_PROTECT (-180 + 2730)
#define DISC_UTP_RECOVER (-140 + 2730)

#define DISC_OTP_PROTECT (630 + 2730)
#define DISC_OTP_RECOVER (590 + 2730)

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
    C_IDLE = 0,
    C_DISCHARGE, 
    C_PROTECT,
}port_Cstatus_t;
typedef enum
{
    A_IDLE = 0,
    A_DISCHARGE,
	A_PROTECT,
} port_Astatus_t;
typedef enum
{
    A_IDLE_2 = 0,
    A_DISCHARGE_2,
} port_Astatus_t_2;

typedef enum
{
    PG_IDLE = 0,
    PG_CHARGE,
    PG_PROTECT,
} port_PGstatus_t;
typedef struct 
{
    volatile uint32_t tick;
    volatile uint32_t uart3_idle_cntdown;
    uint16_t line;
    uint8_t isShake;
    uint8_t bms_protect; // bms UVP/OVP/OCP/SCP 保护
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
        uint8_t soc_level;//[0-9]10个level
        float soh;
        uint8_t soh_level;
        float soc;
        uint16_t  vol;
        uint16_t vol_soc;
        
        uint8_t batIsEmpty : 1;
        uint8_t batIsFull : 1;
    }bat;
    struct 
    {
        uint8_t data;
    }abnormal;
    struct 
    {
        port_Astatus_t A1_status;
        port_Astatus_t_2 A1_status_To_g020;
        port_Cstatus_t C1_status;
        port_Cstatus_t C2_status;
        port_PGstatus_t PG_status;

        uint8_t dis_portA_dsg : 1; // 禁止A口放电
        uint8_t dis_G020_chg : 1;  // 禁止g020充电
        uint8_t dis_G020_dsg : 1; // 禁止020放电
        uint8_t a_exit :1;
        uint8_t a_pulgin :1;

        level_t charge_powerdowm; 
		level_t discharge_powerdown;
        
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
    uint8_t eta_en;
    struct 
    {
        uint8_t charge_utp :1;
        uint8_t charge_otp :1;
        uint8_t discharge_utp :1;
        uint8_t discharge_otp :1;
        uint8_t : 4;
    } temp_err;
    struct 
    {
        uint8_t temp_scan :1;
        uint8_t health_trig :1 ;
        uint8_t bms_active : 1;
        volatile uint8_t iic_err    : 1;
        uint8_t aPort_low_current : 1; //a口小电流标志
        uint8_t Low_current_unload : 1; //小电流退载
        volatile uint8_t wake_usart  : 1;
        volatile uint8_t wake_aport  : 1;
        volatile uint8_t wake_key    : 1;
        volatile uint8_t wake_shake  : 1;
    }flag;


}sys_t;

typedef struct
{
	uint16_t nack_cnt;
	uint16_t ack_total;
}xbms_t;


extern xbms_t xbms;
extern sys_t sys;




void task_app(void);

void eta_driver(void);

void tick_delay(uint16_t ms);

#endif
