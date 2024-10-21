#include "debug.h"
#include "led.h"
#include "app.h"
#include "adc.h"
#include "iic.h"
#include "bms.h"
#include "bms_pro.h"
#include "init.h"
#include "key.h"
#include "communication.h"
extern uint16_t V_cells[6];
//char *p[] = {"关机","开机"};
//char *pCPort[] = {"空闲","放电","保护关闭状态"};
//char *pGPort[] = {"空闲", "充电", "保护关闭状态"};
//char *pAPort[] = {"空闲", "放电"};
//char *pled[] = {"全灭", "开机显示", "放电显示","充电显示","电池健康显示","异常","通讯错误"};
//char *pled2[] = {"正常", "异常", };
//char *pg020[] = {"禁止充放电", "允许充电，禁止放电", "禁止充电，允许放电", "允许充电，允许放电"};
void task_debug(void)
{
#if 0
	static int i = 0;
	printf("%d\n",i++);
#endif
#if 1//
	
   // DirectCommands(AlarmStatus, 0xF800, W);
    //DirectCommands(AlarmStatus, 0x0080, W);
	//CommandSubcommands(ALL_FETS_ON);
	
   //  printf("A%s cur:%d ma,vol:%d mv\n", p[sys.state], sys.adc.conver[CH_A_I], sys.adc.conver[CH_A_V]);
//	printf("A%s cur:%d,vol:%d\n", p[sys.state], sys.adc.value[CH_A_I], sys.adc.value[CH_A_V]);
	// printf("CA:%s,CB:%s,PG:%s,A:%s\n",pCPort[sys.port.C1_status],pCPort[sys.port.C2_status],pGPort[sys.port.PG_status],pAPort[sys.port.A1_status]);
	// printf("dis otp:%d,utp:%d,charge otp:%d ,utp:%d\n",sys.temp_err.discharge_otp,sys.temp_err.discharge_utp,sys.temp_err.charge_otp,sys.temp_err.charge_utp);
	// printf("bat_led:%s  port_led:%s\n",pled[led.bat.status],pled2[led.port.status]);
    // printf("bat_per :%d vol_per:%d, health_l/7:%d,bat_l/9:%d \n", sys.bat.per, sys.bat.vol_soc, sys.bat.health, sys.bat.cap);
    // printf("Stack_Voltage:%d\n",Stack_Voltage);
	// printf("disA:%d\n",sys.port.dis_output);
    // printf("wake level%d\n", __GPIO_INPUT_PIN_GET(WAKE_A_PORT, WAKE_A_PIN));
	// printf("chag_down %d,disg_down %d,cmd %d\n",sys.port.charge_powerdowm,sys.port.discharge_powerdown,cmd_g020_get());
	// printf("iic_err:%d\n",sys.flag.iic_err);
	// printf("bms_ac:%d\n",sys.flag.bms_active);
	// printf("DSG:%d,CHG:%d,PCHG:%d,PDSG:%d\n",DSG,CHG,PCHG,PDSG);
    printf("T1:%d T2:%d T3:%d T4:%d\n", (int16_t)bms_tmp1 - 2730, (int16_t)bms_tmp2 - 2730, (int16_t)bms_tmp3 - 2730, (int16_t)bms_tmp4 - 2730);
    //printf("g020:%s\n",cmd_g020_get());
#endif
#if 1 //BMS debug
	printf("V_cells:%d,%d,%d,%d,%d,%d\n",V_cells[0],V_cells[1],V_cells[2],V_cells[3],V_cells[4],V_cells[5]);
	printf("total :%d nack:%d\n",xbms.ack_total,xbms.nack_cnt);
	printf("bms_curr:%d\n",bms_curr);
	printf("DSG:%d,CHG:%d,PCHG:%d,PDSG:%d\n",DSG,CHG,PCHG,PDSG);
	printf("UV_F:%d,OV_F:%d,SCD_F:%d,OCD_F:%d,OCC_F:%d\n",UV_Fault,OV_Fault,SCD_Fault,OCD_Fault,OCC_Fault);
	printf("AlarmBits:0x%x\n",AlarmBits);
    printf("AlarmBits2:0x%x\n", AlarmBits2);
    printf("Stack_Voltage:%d\n",Stack_Voltage);
	printf("Pack_Voltage:%d\n",Pack_Voltage);
	printf("LD_Voltage:%d\n",LD_Voltage);
	printf("SafetyStatusA:0x%x\n",value_SafetyStatusA);
	printf("SafetyStatusB:0x%x\n",value_SafetyStatusB);
	printf("SafetyStatusC:0x%x\n",value_SafetyStatusC);
	printf("controlStatus:0x%x\n",value_ControlStatus);
	printf("PFStatusA:0x%x\n",value_PFStatusA);
	printf("PFStatusB:0x%x\n",value_PFStatusB);
	printf("PFStatusC:0x%x\n",value_PFStatusC);
	printf("FET_Status:0x%x\n",FET_Status);
	printf("CB_ActiveCells:%d\n",CB_ActiveCells);
    printf("T1:%d T2:%d T3:%d T4:%d\n", (int16_t)bms_tmp1 - 2730, (int16_t)bms_tmp2 - 2730, (int16_t)bms_tmp3 - 2730, (int16_t)bms_tmp4 - 2730);
    printf("bms_battery_status 0x%x\n",bms_battery_status);
    printf("ProtectionsTriggered :%d\n", ProtectionsTriggered);
	printf("key_level:%d\n",KEY2_IO_LEVEL );

#endif 
}

//void debug_init(void)
//{
//    iic_test();
//    
//}


