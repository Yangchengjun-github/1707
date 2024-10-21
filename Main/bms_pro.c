
#include "bms_pro.h"
#include "bms.h"
#include "stdio.h"
#include "app.h"
#include "coulomp.h"
extern uint8_t bms_chg_en, bms_dis_en;
/***************************************************
 * @fn          SysTickServ_100ms()
 * @brief       100ms时机执行，每个分支间隔100ms执行一次
 * @param       void
 * @return      void
 **************************************************/
// #define CELLS_NUMS 6

uint16_t bms_tmp1,bms_tmp2,bms_tmp3,bms_tmp4;
uint16_t bms_tmp_H,bms_tmp_L;
uint16_t max_cell_V,min_cell_V;
int16_t bms_curr; //chg + ,dsg -,2mA/lsb
uint16_t bms_battery_status;
int16_t i_bms_tmp_H,i_bms_tmp_L;
uint16_t bms_vbat_sum;
void find_max_min(){
	bms_vbat_sum=0;
	max_cell_V=0;
	min_cell_V=0xffff;
	for(uint8_t i=0;i<CELLS_NUMS;i++){
		if(max_cell_V<V_cells[i]){
			max_cell_V=V_cells[i];
		}
		if(min_cell_V>V_cells[i]){
			min_cell_V=V_cells[i];
		}
		bms_vbat_sum+=V_cells[i];
	}
	if(bms_tmp2>bms_tmp3){
		bms_tmp_H=bms_tmp2;
		bms_tmp_L=bms_tmp3;
	}else{
		bms_tmp_H=bms_tmp3;
		bms_tmp_L=bms_tmp2;
	}
	if(bms_tmp1>bms_tmp_H){
		bms_tmp_H=bms_tmp1;
	}else if(bms_tmp1<bms_tmp_L){
		bms_tmp_L=bms_tmp1;
	}
	
	i_bms_tmp_H=bms_tmp_H-2730;
	i_bms_tmp_L=bms_tmp_L-2730;
}
uint8_t err_0v5_cnt = 0;
uint8_t err_3v8_cnt = 0;
uint8_t err_1v8_cnt = 0;
uint8_t bms_mos_err_cnt;
uint8_t bms_t_mos_cnt;
uint8_t bms_th_chg_cnt,bms_tl_chg_cnt;
uint8_t bms_th_dsg_cnt,bms_tl_dsg_cnt;
uint8_t bms_t_err_cnt;
uint8_t bms_cell_chg_cnt;
uint8_t bms_cell_dsg_cnt;
uint8_t bms_curr_chg_cnt;
uint8_t bms_curr_dsg_cnt;
uint8_t bms_bat_full_cnt;
uint8_t flag_chg_dsg;
uint8_t bms_rst_cnt;
uint8_t bms_cell_chg_curr_cnt;
// void protect_conter_fn(uint8_t idx,uint16_t value){

//}
uint16_t pack_pin_voltage;
uint16_t t_bms_rst_flag = 0;

// uint16_t bms_test_step;
//extern uint8_t cell_bat_full;
void protect_scan_flag(void){//恢复时间15S

static	uint8_t pidx=0;
//	return;
	if(write_nack>20){
		bms_pro_flag.afe_nack20_soft=1;
	}else{
//		Uart_PutChar(write_nack);
//		bms_pro_flag.afe_nack20_soft=0;
	}
	if((!u16_nack_flag)&&(!RX_CRC_Fail)){
		switch(pidx){
			case 1:
				find_max_min();
				if((max_cell_V-min_cell_V>500)&&(max_cell_V>3200)&&(bms_curr>-100)){
					if(err_0v5_cnt>100){
					bms_pro_flag.bat_err_0v5_soft = 1;  //long error
					}else{
						err_0v5_cnt++;
					}
				}else{
					err_0v5_cnt=0;
				}
				break;
			case 2:
				if(max_cell_V>3800){
					if(err_3v8_cnt>100){
						bms_pro_flag.bat_err_ovp_soft=1;
					}else{
						err_3v8_cnt++;
					}
				}else{
					err_3v8_cnt=0;
				}
				if(min_cell_V<2000){
					if(err_1v8_cnt>100){
						bms_pro_flag.bat_err_uvp_soft=1;
					}else{
						err_1v8_cnt++;
					}
				}else{
					err_1v8_cnt=0;
				}
				if(((!DSG)&&(bms_curr<-300))||((!CHG)&&(bms_curr>300))){
						if(bms_mos_err_cnt>80){//?
							bms_pro_flag.mos_crash_soft=1;
							bms_mos_err_cnt=0;
						}else{
							bms_mos_err_cnt++;
						}
					
				}else if(bms_mos_err_cnt){
					bms_mos_err_cnt--;
				}
				break;
		case 3:
			if (bms_pro_flag.ovp_soft)
			{
				if (max_cell_V < 3500)
				{
					if (bms_cell_chg_cnt > 30)
					{
						bms_cell_chg_cnt = 0;
						bms_pro_flag.ovp_soft = 0;
						bms_cell_chg_curr_cnt = 0;
					}
					else
					{
						bms_cell_chg_cnt++;
					}
				}
				else
				{
					bms_cell_chg_cnt = 0;
				}
				if (bms_curr < -300)
				{
					if (bms_cell_chg_curr_cnt > 5)
					{
						bms_cell_chg_cnt = 0;
						bms_cell_chg_curr_cnt = 0;
						bms_pro_flag.ovp_soft = 0;
					}
					else
					{
						bms_cell_chg_curr_cnt++;
					}
				}
				else
				{
					bms_cell_chg_curr_cnt = 0;
				}
			}
			else
			{
				if (max_cell_V > 3650)//todo bat_cell_full ->ok
				{
					if (bms_cell_chg_cnt > 10)
					{
						bms_cell_chg_cnt = 0;
						bms_pro_flag.ovp_soft = 1;
					}
					else
					{
						bms_cell_chg_cnt++;
					}
				}
				else
				{
					bms_cell_chg_cnt = 0;
				}
			}
			if (bms_pro_flag.uvp_soft)
			{
				if (min_cell_V > 2800)
				{
					if (bms_cell_dsg_cnt > 20)
					{
						bms_cell_dsg_cnt = 0;
						bms_pro_flag.uvp_soft = 0;
					}
					else
					{
						bms_cell_dsg_cnt++;
					}
				}
				else
				{
					bms_cell_dsg_cnt = 0;
				}
//				if (Input.CableC_UFP || Input.TypeCB_UFP)//todo remove 
//				{
//					bms_cell_dsg_cnt = 0;
//					bms_pro_flag.uvp_soft = 0;
//				}
			}
			else
			{
				if (min_cell_V < 2500)
				{
					if (bms_cell_dsg_cnt > 50)
					{
						bms_cell_dsg_cnt = 0;
						bms_pro_flag.uvp_soft = 1;
					}
					else
					{
						bms_cell_dsg_cnt++;
					}
				}
				else
				{
					bms_cell_dsg_cnt = 0;
				}
			}
			break;
		case 4:
			if (!bms_pro_flag.mos_otp_soft)
			{
				if ((bms_tmp4 > 1000 + 2730) || ((bms_tmp4 < 2730 - 400)))
				{
					if (bms_t_mos_cnt > 30)
					{
						bms_t_mos_cnt = 0;
						bms_pro_flag.mos_otp_soft = 1;
					}
					else
					{
						bms_t_mos_cnt++;
					}
				}
				else
				{
					bms_t_mos_cnt = 0;
				}
			}
			else
			{
				if ((bms_tmp4 < 850 + 2730) && (bms_tmp4 > 2730 - 350))
				{
					if (bms_t_mos_cnt > 20)
					{
						bms_t_mos_cnt = 0;
						bms_pro_flag.mos_otp_soft = 0;
					}
					else
					{
						bms_t_mos_cnt++;
					}
				}
				else
				{
					bms_t_mos_cnt = 0;
				}
			}
			if (bms_pro_flag.ntc_error_soft)
			{
//					if((bms_tmp_H<2730+1150)&&(bms_tmp4<2730+1150)&&(bms_tmp_L>2730-350)&&(bms_tmp4>2730-350)){
//						if(bms_t_err_cnt>10){
//							bms_pro_flag.ntc_error_soft=1;
//							bms_t_err_cnt=0;
//						}else{
//							bms_t_err_cnt++;
//						}
//					}else{
//							bms_t_err_cnt=0;
//					}
			}
			else
			{
				if ((bms_tmp_H > 2730 + 1150) || (bms_tmp4 > 2730 + 1150) || (bms_tmp_L < 2730 - 400) || (bms_tmp4 < 2730 - 400))
				{
					if (bms_t_err_cnt > 20)
					{
						bms_pro_flag.ntc_error_soft = 1;
						bms_t_err_cnt = 0;
					}
					else
					{
						bms_t_err_cnt++;
					}
				}
				else
				{
					bms_t_err_cnt = 0;
				}
			}
			break;
		case 5:
			if (bms_pro_flag.chg_otp_soft)//todo 优化高低温判定，使高低温可以同时检测到
			{
				if (bms_tmp_H < 500 + 2730)
				{
					if (bms_th_chg_cnt > 20)
					{
						bms_pro_flag.chg_otp_soft = 0;
						bms_th_chg_cnt = 0;
					}
					else
					{
						bms_th_chg_cnt++;
					}
				}
				else
				{
					bms_th_chg_cnt = 0;
				}
			}else{
				if (bms_tmp_H > 540 + 2730)
				{
					if (bms_th_chg_cnt > 30)
					{
						bms_pro_flag.chg_otp_soft = 1;
						bms_th_chg_cnt = 0;
					}
					else
					{
						bms_th_chg_cnt++;
					}
				}else{
					bms_th_chg_cnt=0;
				}
			}
			if (bms_pro_flag.chg_ltp_soft)
			{
				if (bms_tmp_L > 20 + 2730)
				{
					if (bms_tl_chg_cnt > 20)
					{
						bms_pro_flag.chg_ltp_soft = 0;
						bms_tl_chg_cnt = 0;
					}
					else
					{
						bms_tl_chg_cnt++;
					}
				}
				else
				{
					bms_tl_chg_cnt = 0;
				}
			}
			else
			{
				
				if (bms_tmp_L < 10 + 2730)
				{
					if (bms_tl_chg_cnt > 30)
					{
						bms_pro_flag.chg_ltp_soft = 1;
						bms_tl_chg_cnt = 0;
					}
					else
					{
						bms_tl_chg_cnt++;
					}
				}
				else
				{
					bms_tl_chg_cnt = 0;
				}
			}
			break;
		case 6:
			if (bms_pro_flag.dsg_otp_soft)//todo 优化高低温判定，使高低温可以同时检测到
			{
				if (bms_tmp_H < 550 + 2730)
				{
					if (bms_th_dsg_cnt > 20)
					{
						bms_pro_flag.dsg_otp_soft = 0;
						bms_th_dsg_cnt = 0;
					}
					else
					{
						bms_th_dsg_cnt++;
					}
				}
				else
				{
					bms_th_dsg_cnt = 0;
				}
			}else{
				if (bms_tmp_H > 590 + 2730)
				{
					if (bms_th_dsg_cnt > 30)
					{
						bms_pro_flag.dsg_otp_soft = 1;
						bms_th_dsg_cnt = 0;
					}
					else
					{
						bms_th_dsg_cnt++;
					}
				}else{
					bms_th_dsg_cnt=0;
				}
			}
			if (bms_pro_flag.dsg_ltp_soft)
			{
				if (bms_tmp_L > -150 + 2730)
				{
					if (bms_tl_dsg_cnt > 20)
					{
						bms_pro_flag.dsg_ltp_soft = 0;
						bms_tl_dsg_cnt = 0;
					}
					else
					{
						bms_tl_dsg_cnt++;
					}
				}
				else
				{
					bms_tl_dsg_cnt = 0;
				}
			}
			else
			{
				if (bms_tmp_L < -190 + 2730)
				{
					if (bms_tl_dsg_cnt > 30)
					{
						bms_pro_flag.dsg_ltp_soft = 1;
						bms_tl_dsg_cnt = 0;
					}
					else
					{
						bms_tl_dsg_cnt++;
					}
				}
				else
				{
					bms_tl_dsg_cnt = 0;
				}
			}
			break;
		case 7:
			if (bms_pro_flag.chg_ocp_soft)
			{
				if (bms_curr < 1000)
				{
					if (bms_curr_chg_cnt > 150)
					{
						bms_pro_flag.chg_ocp_soft = 0;
						bms_curr_chg_cnt = 0;
					}
					else
					{
						bms_curr_chg_cnt++;
					}
				}
				else
				{
					bms_curr_chg_cnt = 0;
				}
			}
			else
			{
				if (bms_curr > 6000)
				{
					if (bms_curr_chg_cnt > 50)
					{
						bms_curr_chg_cnt = 0;
						bms_pro_flag.chg_ocp_soft = 1;
					}
					else
					{
						bms_curr_chg_cnt++;
					}
				}
				else
				{
					bms_curr_chg_cnt = 0;
				}
			}
			break;
		case 8:
			if (bms_pro_flag.dsg_ocp_soft)
			{
				if (bms_curr > -1000)
				{
					if (bms_curr_dsg_cnt > 150)
					{
						bms_curr_dsg_cnt = 0;
						bms_pro_flag.dsg_ocp_soft = 0;
					}
					else
					{
						bms_curr_dsg_cnt++;
					}
				}
				else
				{
					bms_curr_dsg_cnt = 0;
				}
			}
			else
			{
				if (bms_curr < -8500)
				{
					if (bms_curr_dsg_cnt > 30)
					{
						bms_curr_dsg_cnt = 0;
						bms_pro_flag.dsg_ocp_soft = 1;
					}
					else
					{
						bms_curr_dsg_cnt++;
					}
				}
				else
				{
					bms_curr_dsg_cnt = 0;
				}
			}
				break;
			case 9:	
//				if(bms_pro_flag.bat_full_soft){
//					if(max_cell_V<3320){
//						if(bms_bat_full_cnt>10){
//							bms_bat_full_cnt=0;
//							bms_pro_flag.bat_full_soft=0;
//						}else{
//							bms_bat_full_cnt++;
//						}
//					}else{
//						bms_bat_full_cnt=0;
//					}
//				}else{
//					if((max_cell_V>3600)&&(bms_curr>100)){//todo ask: how to 
//						if(bms_bat_full_cnt>20){
//							bms_bat_full_cnt=0;
//							bms_pro_flag.bat_full_soft=1;
//						}else{
//							bms_bat_full_cnt++;
//						}
//					}else{
//						bms_bat_full_cnt=0;
//					}
//				}
				//bms_pro_flag.bat_full_soft=cell_bat_full;
				//pidx=0;
//				break;
//			case 10:
			if (bms_battery_status & 0x08)
			{
				if (bms_rst_cnt < 10)
				{
					bms_rst_cnt++;
				}
				else
				{
					bms_rst_cnt = 0;
					t_power_on_cnt_s = 2;
				}
			}
			if (bms_battery_status & 0x04)
			{
				CommandSubcommands(SLEEP_DISABLE);
			}
			break;
		default:
			if (pidx >= 10)
				pidx = 0;
			break;
		}
		pidx++;
	}
}

// UV_Fault
// OV_Fault
// SCD_Fault
// OCD_Fault
// OCC_Fault

#if 0
uint8_t auto_termination_CA, auto_termination_CB;
void set_charge_auto_stop(uint8_t enable){
	uint8_t tmp;
		if(auto_termination_CA!=enable){
			auto_termination_CA=enable;
			tmp=CXA_I2C_ReadReg(0x0A);
			tmp&=0xDF;
			if(!enable){
				tmp|=0x20;
			}
			CXA_I2C_WriteReg(0x0A,tmp);
		}
		if(auto_termination_CB!=enable){
			auto_termination_CB=enable;
			tmp=CXB_I2C_ReadReg(0x0A);
			tmp&=0xDF;
			if(!enable){
				tmp|=0x20;
			}
			CXB_I2C_WriteReg(0x0A,tmp);
		}
}
uint16_t t_15s_delay = 0;
#endif 
#if 0
void flag_fix_statu(void)
{
	//	error_strerm(1,((uint8_t*)&bms_pro_flag)[1]);
	//	error_strerm(0,((uint8_t*)&bms_pro_flag)[0]);
	if ((OCC_Fault) || (bms_pro_flag.chg_ocp_soft))
	{
		occ_flag = 1;
	}
	if (SCD_Fault || OCD_Fault || OCC_Fault || bms_pro_flag.chg_ocp_soft || bms_pro_flag.dsg_ocp_soft || bms_pro_flag.afe_nack20_soft 
	|| error_long_flag)
	{
		// t_15s_delay = 400;
	}
	else
	{
		if (t_15s_delay)
			t_15s_delay--;
	}
	if (bms_pro_flag.bat_err_0v5_soft || bms_pro_flag.bat_err_ovp_soft || bms_pro_flag.bat_err_uvp_soft)
	{
		if (error_long_flag == 0)
		{
			write_error_long_flag();
			error_long_flag = 1;
		}
	}

	if (error_long_flag || bms_pro_flag.mos_crash_soft
		//	||(bms_pro_flag.uvp_soft&&(!Input.TypeCB_UFP)&&(!Input.CableC_UFP))
		|| bms_pro_flag.afe_nack20_soft || OCD_Fault || SCD_Fault || bms_pro_flag.dsg_ocp_soft || long_ship_mode)//
	{ // 充放电都不开，且掉电
		BQ769x2_RESET_CHG_OFF();
		BQ769x2_RESET_DSG_OFF();
		BQ769x2_ShutdownPin();
		bms_chg_en = 0;
		bms_dis_en = 0;
		if (OCD_Fault || SCD_Fault || bms_pro_flag.dsg_ocp_soft)//todo ask: how to
		{
			t_15s_delay = 1500;
		}
		else if (t_15s_delay < 50)
		{
			t_15s_delay = 50;
		}
//		t_bms_rst_flag = 1;
	}
	else if (t_15s_delay || bms_pro_flag.mos_otp_soft)
	{ // 不开充放电
		BQ769x2_RESET_CHG_OFF();
		BQ769x2_RESET_DSG_OFF();
		BQ769x2_ReleaseShutdownPin();
		bms_chg_en = 0;
		bms_dis_en = 0;
	}
	else if (bms_pro_flag.uvp_soft 
	&& (!((Input.CableC_UFP || Input.TypeCB_UFP) && !(Protect.Charge_Temper_Lv_Flag || bms_pro_flag.dsg_ltp_soft))))
	{
		BQ769x2_RESET_CHG_OFF();
		BQ769x2_RESET_DSG_OFF();
		//		BQ769x2_ReleaseShutdownPin();
		BQ769x2_ShutdownPin();
		bms_chg_en = 1;
		bms_dis_en = 0;
	}
	else
	{
		if(bms_pro_flag.chg_ltp_soft || bms_pro_flag.chg_otp_soft || OV_Fault || bms_pro_flag.ovp_soft || occ_flag)
		{
			bms_chg_en = 0;
			if (bms_curr > -50)
			{
				BQ769x2_RESET_CHG_OFF();
			}
			else if (bms_curr < -150)
			{
				BQ769x2_CHG_OFF();
			}
		}
		else if (cell_bat_full || Charge.Full)
		{ //||{//
			bms_chg_en = 1;
			if (bms_curr > -50)
			{
				BQ769x2_RESET_CHG_OFF();
			}
			else if (bms_curr < -150)
			{
				BQ769x2_CHG_OFF();
			}
			if (Input.CableC_DFP || Input.TypeCB_DFP || Input.TypeA|| first_init_8815_delay)
			{
				set_charge_auto_stop(0);
			}
			else
			{
				set_charge_auto_stop(1);
			}
		}
		else
		{
			BQ769x2_CHG_OFF();
			//			if(CHG)
			bms_chg_en = 1;
			if(first_init_8815_delay)
				set_charge_auto_stop(0);
			else
				set_charge_auto_stop(1);
		}
		if (bms_pro_flag.dsg_otp_soft || UV_Fault || bms_pro_flag.dsg_ltp_soft || bms_pro_flag.uvp_soft)
		{
			bms_dis_en = 0;
			if (bms_curr < 100)
			{
				BQ769x2_RESET_DSG_OFF();
			}
			else
			{
				BQ769x2_DSG_OFF();
			}
		}
		else
		{
			bms_dis_en = 1;
			BQ769x2_DSG_OFF();
			//			if(DSG)
		}
		BQ769x2_ReleaseShutdownPin();
	}

	//	if(bms_pro_flag.chg_ltp_soft||
	//		bms_pro_flag.chg_ocp_soft||
	////		bms_pro_flag.chg_otp_soft||
	////		(bms_pro_flag.ovp_soft&&(System.WorkMode != SYSTEM_WORK_DISCHARGE))||
	//		bms_pro_flag.bat_err_0v5_soft||
	//		bms_pro_flag.bat_full_soft||
	//		(bms_pro_flag.ovp_soft&&(bms_curr>-300))||
	//		bms_pro_flag.bat_err_ovp_soft||
	//		bms_pro_flag.bat_err_uvp_soft||
	//		bms_pro_flag.mos_otp_soft||error_long_flag){//todo
	//			BQ769x2_RESET_CHG_OFF();
	//			bms_chg_en=0;
	//	}else{
	//		if(bms_pro_flag.ovp_soft){
	//			bms_chg_en=0;
	//		}else{
	//			bms_chg_en=1;
	//		}
	//		BQ769x2_CHG_OFF();
	//	}
	//	if(bms_pro_flag.dsg_ltp_soft||
	//		bms_pro_flag.dsg_ocp_soft||
	////		bms_pro_flag.dsg_otp_soft||
	////		(bms_pro_flag.uvp_soft&&(System.WorkMode != SYSTEM_WORK_CHARGE))||
	//		bms_pro_flag.bat_err_0v5_soft||
	//		(bms_pro_flag.uvp_soft&&(bms_curr<300))||
	//		bms_pro_flag.bat_err_ovp_soft||
	//		bms_pro_flag.bat_err_uvp_soft||
	//		bms_pro_flag.mos_otp_soft||
	//		error_long_flag){
	//			BQ769x2_RESET_DSG_OFF();
	//			dc_reset_flag=1;
	//			DSG=0;
	//			bms_dis_en=0;
	//	}else{ //todo

	//		if(bms_pro_flag.uvp_soft){
	//			bms_dis_en=0;
	//		}else{
	//			bms_dis_en=1;
	//		}
	//		BQ769x2_DSG_OFF();
	//	}
	//	//bms_pro_flag.afe_nack20_soft||
	//	if((bms_pro_flag.uvp_soft&&(!Input.TypeCB_UFP)&&(!Input.CableC_UFP))
	//		||error_long_flag
	//		||bms_pro_flag.afe_nack20_soft){
	//		BQ769x2_ShutdownPin();
	//		t_bms_rst_flag=1;
	//	}else{
	//		BQ769x2_ReleaseShutdownPin();
	//	}
	//	if(
	//		bms_pro_flag.bat_err_0v5_soft||
	////	//todo long_error
	////		bms_pro_flag.bat_err_ovp_soft||
	//		bms_pro_flag.mos_crash_soft||
	//		bms_pro_flag.bat_err_uvp_soft){
	//			if(error_long_flag==0){
	//				write_error_long_flag();
	//				error_long_flag=1;
	//			}else{
	//
	//			}
	//		}
	//		bms_pro_flag.ntc_error_soft=0;
	//		if((Input.TypeCB_UFP||Input.CableC_UFP)&&t_bms_rst_flag){
	//			t_bms_rst_flag=0;
	//			t_power_on_cnt_s=1;
	//		}
	//		error_strerm(3,error_long_flag);
	//		error_strerm(4,t_bms_rst_flag);
	//		error_strerm(5,t_power_on_cnt_s);
	//		error_strerm(6,bms_dis_en);
	//		error_strerm(7,bms_chg_en);
}
#endif
uint8_t dc_reset_flag = 0; // 用于指示电池是否曾经关闭，以保证dcdc被正确初始化
uint16_t bms_test_step = 0;
void task_bms(void)
{
	static unsigned char idx = 0;
	//	Uart_PutChar(0xf0);
	//	Uart_PutChar(idx);
	//printf("bms:%02d\n", idx);
	//		Uart_PutChar(idx);
	switch (idx)
	{
	case 1:
		//			idx=test_banch(idx);
        AlarmBits = BQ769x2_ReadAlarmRaw();
		AlarmBits2 = BQ769x2_ReadAlarmStatus();
        BQ769x2_ReadAllVoltages();
		break;
	case 2:
		BQ769x2_ReadFETStatus();
		break;
	case 3:
		BQ769x2_ReadSafetyStatusA();
		BQ769x2_ReadPFStatusA();
		break;
	case 4:
		BQ769x2_ReadSafetyStatusB();
        BQ769x2_ReadPFStatusB();
        break;
	case 5:
		BQ769x2_ReadSafetyStatusC();
        BQ769x2_ReadPFStatusC();
        break;
	case 6:

        BQ769x2_ReadFETStatus();
        BQ769x2_ReadControlStatus();
        break;
	case 7:
		bms_curr = BQ769x2_ReadCurrent(); // 2mA/lsb
		coulomp.current = 1000;//bms_curr;
		break;
	case 8:
		BQ769x2_Read_Vcells_123();
		break;
	case 9:
		BQ769x2_Read_Vcells_5();
	case 10:
		BQ769x2_Read_Vcells_7();
	case 11:
		BQ769x2_Read_Vcells_9();
		break;
	case 12:
		BQ769x2_ReadFETStatus();
		break;
	case 13:
		bms_tmp3 = BQ769x2_ReadTemperature(TS3Temperature); // 0.1K
		break;
	case 14:
		bms_tmp1 = BQ769x2_ReadTemperature(TS1Temperature); // 0.1K
		break;
	case 15:
#if KEY_LOW_TMPL
		if (bms_test_step < 30)
			bms_tmp2 = 2730 + 80;
		else
			bms_tmp2 = BQ769x2_ReadTemperature(TS2Temperature); // 0.1K
#else
		bms_tmp2 = BQ769x2_ReadTemperature(TS2Temperature); // 0.1K
#endif
		break;
	case 16:
		BQ769x2_ReadFETStatus();
		break;
	case 17:
		bms_tmp4 = BQ769x2_ReadTemperature(HDQTemperature); // 0.1K
		sys.flag.temp_scan = 1;
		break;
	case 18:
		// DirectCommands(ControlStatus, 0x00, R);
		pack_pin_voltage = BQ769x2_ReadVoltage(PACKPinVoltage);
		
		
		break;
	case 19:
		DirectCommands(BatteryStatus, 0x00, R);
		bms_battery_status = RX_data[0] + (RX_data[1] << 8);
		break;
		//	BQ769x2_ReadPassQ();// Read Accumulated Charge and Time from DASTATUS6 电荷量

		//		case 17:
		//			BQ769x2_ReadPFStatusA();//保险丝
		//		break;
		//		case 13:
		//			BQ769x2_ReadPFStatusB();
		//		break;
		//		case 14:
		//			BQ769x2_ReadPFStatusC();
		//		break;
		//		case 17:
		//		break;
		//		case 18:
		//		break;
		//		case 20:
		//		break;
	default:
		if (idx > 19)
		{
			idx = 0;
		}
		break;
	}

	//	Uart_PutChar(0xf0);
	//	Uart_PutChar(idx);
	if (u16_nack_flag || RX_CRC_Fail)
	{
		if ((!t_power_on_cnt_s) && (write_nack < 100)) // nack
			write_nack++;
	}
	else
	{
		idx++;
		write_nack = 0;
	}
}
