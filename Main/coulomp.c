#include "coulomp.h"
#include "app.h"
#include "bms.h"
coulomp_t coulomp = {0};
uint8_t coulomp_calc(coulomp_t *p,uint16_t bat_vol);


static const uint8_t table[] = {85, 71, 57, 43, 29, 14, 5, 0};
void  coulomp_init(void)
{
    uint8_t vol_soc;
    coulomp.total_cap = BAT_CAP; // 8000mah
    coulomp.residue_cap = 0;
    coulomp.current = 0;
    sys.bat.vol = BQ769x2_ReadVoltage(StackVoltage) ; //TODO
    sys.bat.vol_soc = estimate_soc_from_voltage(sys.bat.vol);
    coulomp.residue_cap = sys.bat.vol_soc / 100.0 * coulomp.total_cap;
    for (int i = 0; i < 8; i++)
    {
        if (sys.bat.per > table[i])
        {
            sys.bat.cap = 8 - i;
            break;
        }
        sys.bat.cap = 0;
    }
    printf("coulomp_init  %d  V %d soc %d\n", sys.bat.vol_soc, sys.bat.vol, coulomp.residue_cap);
}


void task_coulomp( void)
{
    sys.bat.per = coulomp_calc(&coulomp, sys.bat.vol);
    for (int i = 0; i < 8; i++)
    {
        if (sys.bat.per > table[i])
        {
            sys.bat.cap = 8 - i;
            break;
        }
        sys.bat.cap = 0;
    }
}

#define ERROR_THRESHOLD 15     // 误差阈值 (%)
#define COMPENSATION (BAT_CAP /100 / 2) // 补偿值 0.0005
uint8_t coulomp_calc(coulomp_t *p,uint16_t bat_vol)
{
    sys.bat.vol_soc = estimate_soc_from_voltage(bat_vol);
    p->residue_cap = p->residue_cap + p->current;
    if (p->residue_cap <= 0)
    {
        p->residue_cap = 0;
    }

    p->percent = p->residue_cap * 100.0 / p->total_cap;
    if (p->percent > 100)
    {
        p->percent = 100;
    }

    float error = sys.bat.vol_soc - p->percent;

    // 如果误差超过阈值，进行补偿
    if ((error > ERROR_THRESHOLD && p->current > 0 && sys.port.PG_status == PG_CHARGE) || sys.bat.vol_soc == 100) //
    {
        p->residue_cap += COMPENSATION; // 逐步增加，避免突变
    }
    else if ((error < -ERROR_THRESHOLD) || sys.bat.vol_soc == 0)
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
        //cc_soc = vol_soc; // 或者也可以不做任何调整
    }

    // 确保 SoC 不超过 0-100% 范围
    if (p->percent > 100)
        p->percent = 100;
    if (p->percent < 0)
        p->percent = 0;

    return p->percent;
}

// 定义六串电池的电压范围（单位：毫伏）
#define MAX_VOLTAGE_MV 21600           // 6S * 3600mV
#define FULLY_CHARGED_VOLTAGE_MV 21600 // 100%
#define EMPTY_VOLTAGE_MV 15000         // 0%

// 定义各电压区间对应的 SoC 范围
typedef struct
{
    int voltage_min; // 最小电压（毫伏）
    int voltage_max; // 最大电压（毫伏）
    float soc_min;   // 对应的最小 SoC（%）
    float soc_max;   // 对应的最大 SoC（%）
} VoltageSocRange;

// 定义电压与 SoC 的对应关系
VoltageSocRange voltage_soc_ranges[] = {
    {21600, 30000, 100.0, 100.0}, // 完全充满
    {21000, 21599, 90.0, 100.0},  // 90%-100%
    {20400, 20999, 80.0, 90.0},   // 80%-90%
    {19800, 20399, 60.0, 80.0},   // 60%-80%
    {19200, 19799, 40.0, 60.0},   // 40%-60%
    {18600, 19199, 20.0, 40.0},   // 20%-40%
    {18000, 18599, 10.0, 20.0},   // 10%-20%
    {15000, 17999, 0.0, 10.0},    // 0%-10%
    {0, 14999, 0.0, 0.0}          // 完全放电
};

// 计算 SoC 的函数
float estimate_soc_from_voltage(int voltage_mv)
{
    int num_ranges = sizeof(voltage_soc_ranges) / sizeof(voltage_soc_ranges[0]);

    for (int i = 0; i < num_ranges; i++)
    {
        if (voltage_mv >= voltage_soc_ranges[i].voltage_min && voltage_mv <= voltage_soc_ranges[i].voltage_max)
        {
            // 如果最大电压等于最小电压，直接返回对应的 SoC
            if (voltage_soc_ranges[i].voltage_max == voltage_soc_ranges[i].voltage_min)
            {
                return voltage_soc_ranges[i].soc_max;
            }
            // 线性插值计算 SoC
            float soc = voltage_soc_ranges[i].soc_min +
                        ((float)(voltage_mv - voltage_soc_ranges[i].voltage_min) /
                         (voltage_soc_ranges[i].voltage_max - voltage_soc_ranges[i].voltage_min)) *
                            (voltage_soc_ranges[i].soc_max - voltage_soc_ranges[i].soc_min);
            return soc;
        }
    }
    // 如果电压超出定义范围，返回 0%
    return 0.0;
}