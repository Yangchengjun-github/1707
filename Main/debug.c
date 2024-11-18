#include "debug.h"
#include "led.h"
#include "app.h"
#include "adc.h"
#include "iic.h"
#include "bms.h"
#include "bms_pro.h"
#include "init.h"
#include "key.h"
#include "coulomp.h"
#include "communication.h"
#include "cs32f10x_gpio.h"
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
    printf("PORT:BAT:soc :%02f vol_soc:%d vol:%d, cur:%d,health_l/7:%d,soh:%02f,bat_l/9:%d ,soc_level:%d(%d),empty: %d,full :%d\n",
           sys.bat.soc, sys.bat.vol_soc, sys.bat.vol,bms_curr, sys.bat.soh_level, sys.bat.soh, sys.bat.soc_level, coulomp.residue_cap, BAT_CAP,sys.bat.batIsEmpty,sys.bat.batIsFull);

    // DirectCommands(AlarmStatus, 0xF800, W);
    // DirectCommands(AlarmStatus, 0x0080, W);
    // CommandSubcommands(ALL_FETS_ON);
    printf("PORT:-----------s-----------\n");
    printf("PORT:BAT:soc :%02f vol_soc:%d, cur:%d,health_l/7:%d,soh:%02f,bat_l/9:%d ,soc_level:%d(%d)\n",
           sys.bat.soc, sys.bat.vol_soc, bms_curr, sys.bat.soh_level, sys.bat.soh, sys.bat.soc_level, coulomp.residue_cap, BAT_CAP);
    printf("PORT:A dis %d  cur:%d ma,vol:%d mv\n", sys.port.dis_output, sys.adc.conver[CH_A_I], sys.adc.conver[CH_A_V]);
    // printf("PORT:A%d cur:%d,vol:%d\n", sys.state, sys.adc.value[CH_A_I], sys.adc.value[CH_A_V]);
    printf("PORT:get A wake pin %d\n", __GPIO_INPUT_PIN_GET(WAKE_A_PORT, WAKE_A_PIN));
    printf("PORT:CA:%d,CB:%d,PG:%d,A:%d\n", sys.port.C1_status, sys.port.C2_status, sys.port.PG_status, sys.port.A1_status);
    printf("PORT:dis otp:%d,utp:%d,charge otp:%d ,utp:%d\n", sys.temp_err.discharge_otp, sys.temp_err.discharge_utp, sys.temp_err.charge_otp, sys.temp_err.charge_utp);
    printf("PORT:bat_led:%d  port_led:%d\n", led.bat.status, led.port.status);

    printf("PORT:Stack_Voltage:%d\n", Stack_Voltage);

    printf("PORT:g020:pow:%d chag_down %d,disg_down %d,cmd %d\n", __GPIO_OUTPUT_PIN_GET(EN_G020_PORT, EN_G020_PIN), sys.port.charge_powerdowm, sys.port.discharge_powerdown, cmd_g020_get());
    printf("PORT:iic_err:%d\n", sys.flag.iic_err);
    printf("PORT:bms_ac:%d\n", sys.flag.bms_active);

    printf("PORT:BMS>DSG:%d,CHG:%d,PCHG:%d,PDSG:%d\n", DSG, CHG, PCHG, PDSG);
    printf("PORT:MCU>DSG:%d,CHG:%d\n", __GPIO_OUTPUT_PIN_GET(DFETOFF_PORT, DFETOFF_PIN), __GPIO_OUTPUT_PIN_GET(CFETOFF_PORT, CFETOFF_PIN));
#endif
#if 1 //BMS debug
    printf("BMS:-----------s----------\n");

    printf("BMS:V_cells:%d,%d,%d,%d,%d,%d\n",V_cells[0],V_cells[1],V_cells[2],V_cells[3],V_cells[4],V_cells[5]);
	printf("BMS:total :%d nack:%d\n",xbms.ack_total,xbms.nack_cnt);
	printf("BMS:bms_curr:%d\n",bms_curr);
	printf("BMS:DSG:%d,CHG:%d,PCHG:%d,PDSG:%d\n",DSG,CHG,PCHG,PDSG);
	printf("BMS:UV_F:%d,OV_F:%d,SCD_F:%d,OCD_F:%d,OCC_F:%d\n",UV_Fault,OV_Fault,SCD_Fault,OCD_Fault,OCC_Fault);
	printf("BMS:AlarmBits:0x%x\n",AlarmBits);
    printf("BMS:AlarmBits2:0x%x\n", AlarmBits2);
    printf("BMS:Stack_Voltage:%d\n",Stack_Voltage);
	printf("BMS:Pack_Voltage:%d\n",Pack_Voltage);
	printf("BMS:LD_Voltage:%d\n",LD_Voltage);
	printf("BMS:SafetyStatusA:0x%x\n",value_SafetyStatusA);
	printf("BMS:SafetyStatusB:0x%x\n",value_SafetyStatusB);
	printf("BMS:SafetyStatusC:0x%x\n",value_SafetyStatusC);
	printf("BMS:controlStatus:0x%x\n",value_ControlStatus);
	printf("BMS:PFStatusA:0x%x\n",value_PFStatusA);
	printf("BMS:PFStatusB:0x%x\n",value_PFStatusB);
	printf("BMS:PFStatusC:0x%x\n",value_PFStatusC);
	printf("BMS:FET_Status:0x%x\n",FET_Status);
	printf("BMS:CB_ActiveCells:%d\n",CB_ActiveCells);
    printf("BMS:T1:%d T2:%d T3:%d T4:%d\n", (int16_t)bms_tmp1 - 2730, (int16_t)bms_tmp2 - 2730, (int16_t)bms_tmp3 - 2730, (int16_t)bms_tmp4 - 2730);
    printf("BMS:bms_battery_status 0x%x\n",bms_battery_status);
    printf("BMS:ProtectionsTriggered :%d\n", ProtectionsTriggered);
	printf("key_level:%d\n",KEY2_IO_LEVEL );

#endif 
}

//void debug_init(void)
//{
//    iic_test();
//    
//}


