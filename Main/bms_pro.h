#ifndef __BMS_PRO_H__
#define __BMS_PRO_H__

#include"stdint.h"
#define TIME_TASK_BMS_CALL  (100)
#define CELLS_NUMS 6

extern uint16_t bms_tmp1,bms_tmp2,bms_tmp3,bms_tmp4;
extern uint16_t bms_tmp_H,bms_tmp_L;
extern uint16_t max_cell_V,min_cell_V;
extern int16_t bms_curr; //chg + ,dsg -,2mA/lsb
extern int16_t i_bms_tmp_H,i_bms_tmp_L;
extern uint16_t bms_vbat_sum;
extern uint16_t long_ship_mode;
extern uint16_t occ_flag;
extern uint8_t auto_termination_CA,auto_termination_CB;
extern uint16_t bms_battery_status;
void task_bms(void);
void flag_fix_statu(void);
void protect_scan_flag(void);

#endif
