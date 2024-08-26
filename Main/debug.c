#include "debug.h"
#include "led.h"
#include "app.h"
#include "adc.h"
#include "iic.h"
#include "bms.h"
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

	static int i = 0;
	printf("%d\n",i++);
	BQ769x2_DSG_OFF();
	BQ769x2_CHG_OFF();
	
	CommandSubcommands(ALL_FETS_ON);
//	CommandSubcommands(FET_ENABLE);
//	if(i== 3)
	if(DSG== 0)
	CommandSubcommands(DSGTEST );
	if(CHG == 0)
	CommandSubcommands(CHGTEST);
//	}
	
    printf("%s cur:%d,vol:%d\n", p[sys.state], sys.adc.conver[CH_A_I], sys.adc.conver[CH_A_V]);
	printf("%s cur:%d,vol:%d\n", p[sys.state], sys.adc.value[CH_A_I], sys.adc.value[CH_A_V]);
	printf("CA:%s,CB:%s,PG:%s,A:%s\n",pCPort[sys.port.C1_status],pCPort[sys.port.C2_status],pGPort[sys.port.PG_status],pAPort[sys.port.A1_status]);
	printf("bat_led:%s  port_led:%s\n",pled[led.bat.status],pled2[led.port.status]);
#if 1   //BMS debug
	printf("V_cells:%d,%d,%d,%d,%d\n",V_cells[0],V_cells[1],V_cells[2],V_cells[4],V_cells[5]);
	printf("total :%d nack:%d\n",xbms.ack_total,xbms.nack_cnt);
	printf("bms_curr:%d\n",bms_curr);
	printf("DSG:%d,CHG:%d,PCHG:%d,PDSG:%d\n",DSG,CHG,PCHG,PDSG);
	printf("UV_F:%d,OV_F:%d,SCD_F:%d,OCD_F:%d,OCC_F:%d\n",UV_Fault,OV_Fault,SCD_Fault,OCD_Fault,OCC_Fault);
	printf("AlarmBits:%d\n",AlarmBits);
	printf("Stack_Voltage:%d\n",Stack_Voltage);
	printf("Pack_Voltage:%d\n",Pack_Voltage);
	printf("LD_Voltage:%d\n",LD_Voltage);
	printf("Pack_Current:%d\n",Pack_Current);
	printf("value_SafetyStatusA:%d\n",value_SafetyStatusA);
	printf("value_SafetyStatusB:%d\n",value_SafetyStatusB);
	printf("value_SafetyStatusC:%d\n",value_SafetyStatusC);
	printf("value_PFStatusA:%d\n",value_PFStatusA);
	printf("value_PFStatusB:%d\n",value_PFStatusB);
	printf("value_PFStatusC:%d\n",value_PFStatusC);
	printf("FET_Status:%d\n",FET_Status);
	printf("CB_ActiveCells:%d\n",CB_ActiveCells);
	printf("T1:%d T2:%d T3:%d T4:%d\n",bms_tmp1,bms_tmp2,bms_tmp3,bms_tmp4);
#endif 
}

//void debug_init(void)
//{
//    iic_test();
//    
//}

