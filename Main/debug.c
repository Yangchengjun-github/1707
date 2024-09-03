#include "debug.h"
#include "led.h"
#include "app.h"
#include "adc.h"
#include "iic.h"
#include "bms.h"
#include "bms_pro.h"
#include "init.h"
extern uint16_t V_cells[6];
char *p[2] = {"关机","开机"};
char *pCPort[3] = {"空闲","放电","保护关闭状态"};
char *pGPort[3] = {"空闲", "充电", "保护关闭状态"};
char *pAPort[3] = {"空闲", "放电"};
char *pled[5] = {"全灭", "开机显示", "放电显示","充电显示","电池健康显示"};
char *pled2[] = {"正常", "错误", };

void task_debug(void)
{
#if 1 //
	static int i = 0;
	printf("%d\n",i++);
   // DirectCommands(AlarmStatus, 0xF800, W);
    //DirectCommands(AlarmStatus, 0x0080, W);
	//CommandSubcommands(ALL_FETS_ON);
    printf("%s cur:%d,vol:%d\n", p[sys.state], sys.adc.conver[CH_A_I], sys.adc.conver[CH_A_V]);
	printf("%s cur:%d,vol:%d\n", p[sys.state], sys.adc.value[CH_A_I], sys.adc.value[CH_A_V]);
	printf("CA:%s,CB:%s,PG:%s,A:%s\n",pCPort[sys.port.C1_status],pCPort[sys.port.C2_status],pGPort[sys.port.PG_status],pAPort[sys.port.A1_status]);
	printf("dis otp:%d,utp%d,charge otp:%d ,utp:%d\n",sys.temp_err.discharge_otp,sys.temp_err.discharge_utp,sys.temp_err.charge_otp,sys.temp_err.charge_utp);
	printf("bat_led:%s  port_led:%s\n",pled[led.bat.status],pled2[led.port.status]);
    printf("bat_per :%d vol_per:%d\n", sys.bat.per,sys.bat.vol_soc);
	printf("Stack_Voltage:%d\n",Stack_Voltage);
#endif
#if 0   //BMS debug
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
	printf("PFStatusA:0x%x\n",value_PFStatusA);
	printf("PFStatusB:0x%x\n",value_PFStatusB);
	printf("PFStatusC:0x%x\n",value_PFStatusC);
	printf("FET_Status:0x%x\n",FET_Status);
	printf("CB_ActiveCells:%d\n",CB_ActiveCells);
	printf("T1:%d T2:%d T3:%d T4:%d\n",bms_tmp1,bms_tmp2,bms_tmp3,bms_tmp4);
    printf("bms_battery_status 0x%x\n",bms_battery_status);
    printf("ProtectionsTriggered :%d\n", ProtectionsTriggered);

#endif 
}

//void debug_init(void)
//{
//    iic_test();
//    
//}

