#include "coulomp.h"
#include "app.h"
#include "bms.h"
coulomp_t coulomp = {0};
float coulomp_calc(coulomp_t *p,uint16_t bat_vol);


static const uint8_t table[] = {99,85, 71, 57, 43, 29, 14, 5, 0};
void  coulomp_init(void)
{

    coulomp.total_cap = BAT_CAP; // 8000mah
    coulomp.residue_cap = 0;
    coulomp.current = 0;
  
    sys.bat.vol = BQ769x2_ReadVoltage(StackVoltage) ; //TODO
    sys.bat.vol_soc = estimate_soc_from_voltage(sys.bat.vol,0);
    coulomp.residue_cap = sys.bat.vol_soc / 100.0 * coulomp.total_cap;
    for (int i = 0; i < 9; i++)
    {
        if (sys.bat.soc > table[i])
        {
            sys.bat.soc_level = 9 - i;
            break;
        }
        sys.bat.soc_level = 0;
    }

    if(sys.bat.soc_level == 9 && sys.bat.batIsFull == 0)
    {
        sys.bat.soc_level = 8;
    }
    printf("coulomp_init  %d  V %d soc %d\n", sys.bat.vol_soc, sys.bat.vol, coulomp.residue_cap);
}


void task_coulomp( void)
{
    sys.bat.soc = coulomp_calc(&coulomp, sys.bat.vol);
    for (int i = 0; i < 9; i++)
    {
        if (sys.bat.soc > table[i])
        {
            sys.bat.soc_level = 9 - i;
            break;
        }
        sys.bat.soc_level = 0;
    }
    // sys.bat.soc = 0; // TODO TEST
    // sys.bat.soc_level = 0;
}
#define ERROR_THRESHOLD 75     // 误差阈值 (%)
#define ERROR_THRESHOLD_2 50     // 误差阈值 (%)
#define COMPENSATION (BAT_CAP /100 / 2) // 补偿值 0.0005
float coulomp_calc(coulomp_t *p,uint16_t bat_vol)
{

    static uint16_t full_cnt = 0;

    static uint16_t empty_cnt = 0;
    sys.bat.vol_soc = estimate_soc_from_voltage(bat_vol, sys.port.PG_status == PG_CHARGE);
    /* ----------------------------------- 库仑计 ---------------------------------- */
    p->residue_cap = p->residue_cap + p->current;
   
    float error = sys.bat.vol_soc - p->percent;


    /* ---------------------------------- 判满判空 ---------------------------------- */
    if (sys.port.PG_status == PG_CHARGE &&  sys.bat.vol_soc >= 90 && fabs(bms_curr) < 150) //充满
    {
        full_cnt++;
        if(full_cnt > 3000 / TIME_TASK_COULOMP_CALL)
        {
            sys.bat.batIsFull = 1;
            sys.bat.batIsEmpty = 0;
        }
    }
    else
    {
        full_cnt = 0;
        sys.bat.batIsFull = 0 ;
        
    }


    if (sys.port.PG_status != PG_CHARGE &&  sys.bat.vol_soc <= 00 && fabs(bms_curr) < 100)
    {
        empty_cnt++;
        if(empty_cnt > 3000 / TIME_TASK_COULOMP_CALL)
        {
            sys.bat.batIsEmpty = 1;
            sys.bat.batIsFull = 0;
        }
    }
    else
    {
        empty_cnt = 0;
        sys.bat.batIsEmpty = 0;
    }

    if(sys.bat.batIsFull == 1)
    {
        p->residue_cap = BAT_CAP;
    }
    else if(sys.bat.batIsEmpty)
    {
        p->residue_cap = 0;
    }

    /* ---------------------------------- 电压调整库伦（缓和）正补偿  ---------------------------------- */
    if ((/* && p->current > 0 */ sys.port.PG_status == PG_CHARGE) && (sys.bat.vol_soc == 100 || error > ERROR_THRESHOLD)) //
    {
        p->residue_cap += COMPENSATION; // 逐步增加，避免突变
    }
    else if ((sys.port.PG_status != PG_CHARGE) && (sys.bat.vol_soc == 0 || error < -ERROR_THRESHOLD))
    {
        p->residue_cap -= COMPENSATION; // 逐步减少，避免突变
        if(p->residue_cap < 0)
        {
            p->residue_cap = 0;
        }
    }
    else
    {
        // 在误差范围内，不进行补偿

    }

    // /* ---------------------------------- 电压调整库伦（激进） ---------------------------------- */
    // if ((error > ERROR_THRESHOLD_2/*  && p->current > 0 */ && sys.port.PG_status == PG_CHARGE) || sys.bat.vol_soc == 100) //
    // {
    //     p->residue_cap += COMPENSATION * 10; // 逐步增加，避免突变
    // }
    // else if (((error < -ERROR_THRESHOLD_2) || sys.bat.vol_soc == 0) && sys.port.PG_status != PG_CHARGE)
    // {
    //     p->residue_cap -= COMPENSATION * 10; // 逐步减少，避免突变
    //     if (p->residue_cap < 0)
    //     {
    //         p->residue_cap = 0;
    //     }
    // }
    // else
    // {
    //     // 在误差范围内，不进行补偿

    // }

    // 确保 SoC 不超过 0-100% 范围
    if (p->residue_cap <= 0)
    {
        p->residue_cap = 0;
    }
    if (p->residue_cap >= BAT_CAP)
    {
        p->residue_cap = BAT_CAP;
    }
    p->percent = p->residue_cap * 100.0 / p->total_cap;
    if (p->percent > 100)
    {
        p->percent = 100;
    }

    return p->percent;
}

// 定义六串电池的电压范围（单位：毫伏）
#define MAX_VOLTAGE_MV 21600           // 6S * 3600mV
#define FULLY_CHARGED_VOLTAGE_MV 21300 // 100%
#define EMPTY_VOLTAGE_MV 15000         // 0%

// 定义各电压区间对应的 SoC 范围
typedef struct
{
    int voltage_min; // 最小电压（毫伏）
    int voltage_max; // 最大电压（毫伏）
    float soc_min;   // 对应的最小 SoC（%）
    float soc_max;   // 对应的最大 SoC（%）
} VoltageSocRange;

// 定义电压与 SoC 的对应关系 充0.2c
VoltageSocRange voltage_soc_ranges_chg[] = {
    {3650 * 6, 4000 * 6 - 1, 100.0, 100.0}, // 完全充满 //根据g020停止充电电压来调整
    {3421 * 6, 3650 * 6 - 1, 90.0, 100.0}, // 90%-100%
    {3407 * 6, 3421 * 6 - 1, 80.0, 90.0},  // 80%-90%
    {3401 * 6, 3407 * 6 - 1, 70.0, 80.0},  // 70%-80%
    {3381 * 6, 3401 * 6 - 1, 60.0, 70.0},  // 60%-70%
    {3366 * 6, 3381 * 6 - 1, 50.0, 60.0},  // 50%-60%
    {3360 * 6, 3366 * 6 - 1, 40.0, 50.0},   // 40%-50%
    {3354 * 6, 3360 * 6 - 1, 30.0, 40.0},   // 30%-40%
    {3323 * 6, 3354 * 6 - 1, 20.0, 30.0},       // 20%-30%
    {3272 * 6, 3323 * 6 - 1, 10.0, 20.0},  // 10%-20%
    {2050 * 6, 3272 * 6 - 1, 0.0, 10.0},   // 0%-10%
    {0 * 6, 2050 * 6 - 1, 0.0, 0.0}         // 完全放电
};

// 定义电压与 SoC 的对应关系 放0.2c
VoltageSocRange voltage_soc_ranges_dsg[] = {
    {3326 * 6, 4000 * 6 - 1, 100.0, 100.0}, // 完全充满 //根据g020停止充电电压来调整
    {3250 * 6, 3326 * 6 - 1, 90.0, 100.0},  // 90%-100%
    {3242 * 6, 3250 * 6 - 1, 80.0, 90.0},   // 80%-90%
    {3230 * 6, 3240 * 6 - 1, 70.0, 80.0},   // 70%-80%
    {3216 * 6, 3230 * 6 - 1, 60.0, 70.0},   // 60%-70%
    {3205 * 6, 3216 * 6 - 1, 50.0, 60.0},   // 50%-60%
    {3193 * 6, 3205 * 6 - 1, 40.0, 50.0},   // 40%-50%
    {3176 * 6, 3193 * 6 - 1, 30.0, 40.0},   // 30%-40%
    {3146 * 6, 3176 * 6 - 1, 20.0, 30.0},   // 20%-30%
    {3084 * 6, 3146 * 6 - 1, 10.0, 20.0},   // 10%-20%
    {2050 * 6, 3084 * 6 - 1, 0.0, 10.0},    // 0%-10%
    {0 * 6, 2100 * 6 - 1, 0.0, 0.0}         // 完全放电
};

// 计算 SoC 的函数
float estimate_soc_from_voltage(int voltage_mv,uint8_t isCharge)
{
    int num_ranges = sizeof(voltage_soc_ranges_chg) / sizeof(voltage_soc_ranges_chg[0]);
    if (isCharge)
    {
        for (int i = 0; i < num_ranges; i++)
        {
            if (voltage_mv >= voltage_soc_ranges_chg[i].voltage_min && voltage_mv <= voltage_soc_ranges_chg[i].voltage_max)
            {
                // 如果最大电压等于最小电压，直接返回对应的 SoC
                if (voltage_soc_ranges_chg[i].voltage_max == voltage_soc_ranges_chg[i].voltage_min)
                {
                    return voltage_soc_ranges_chg[i].soc_max;
                }
                // 线性插值计算 SoC
                float soc = voltage_soc_ranges_chg[i].soc_min +
                            ((float)(voltage_mv - voltage_soc_ranges_chg[i].voltage_min) /
                             (voltage_soc_ranges_chg[i].voltage_max - voltage_soc_ranges_chg[i].voltage_min)) *
                                (voltage_soc_ranges_chg[i].soc_max - voltage_soc_ranges_chg[i].soc_min);
                return soc;
            }
        }
        // 如果电压超出定义范围，返回 0%
        return 0.0;
    }
    else
    {
        for (int i = 0; i < num_ranges; i++)
        {
            if (voltage_mv >= voltage_soc_ranges_dsg[i].voltage_min && voltage_mv <= voltage_soc_ranges_dsg[i].voltage_max)
            {
                // 如果最大电压等于最小电压，直接返回对应的 SoC
                if (voltage_soc_ranges_dsg[i].voltage_max == voltage_soc_ranges_dsg[i].voltage_min)
                {
                    return voltage_soc_ranges_dsg[i].soc_max;
                }
                // 线性插值计算 SoC
                float soc = voltage_soc_ranges_dsg[i].soc_min +
                            ((float)(voltage_mv - voltage_soc_ranges_dsg[i].voltage_min) /
                             (voltage_soc_ranges_dsg[i].voltage_max - voltage_soc_ranges_dsg[i].voltage_min)) *
                                (voltage_soc_ranges_dsg[i].soc_max - voltage_soc_ranges_dsg[i].soc_min);
                return soc;
            }
        }
        // 如果电压超出定义范围，返回 0%
        return 0.0;
    }
    
}

