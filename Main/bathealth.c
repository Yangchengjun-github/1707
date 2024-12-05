#include "bathealth.h"

#include "app.h"

#include "coulomp.h"
#include "flash.h"

#include "flash.h"
health_t health = {0};


float soh_calculus_interpolation(int a);
uint16_t bat_cycles_calc(int16_t current, float k);

void health_init(void)
{
    sys.bat.soh = 100;
    app_flash_read();
}

static const float table_soh_level[] = {85.8, 71.4, 57.1, 42.9, 28.6, 14.3, 5.0, 0};

void task_health(void)
{
    static uint16_t last_cycles;
    uint8_t level_num = sizeof(table_soh_level) / sizeof(table_soh_level[0]);
    health.cycles = bat_cycles_calc(coulomp.current,1.0);
    sys.bat.soh = soh_calculus_interpolation(health.cycles);


    if(health.cycles != last_cycles)
    {
        printf("soh_save cycle:%d\n",health.cycles);
        app_flash_save();
    }

    for(int i = 0; i< level_num; i++)
    {
        if (sys.bat.soh >= table_soh_level[i])
        {
            sys.bat.soh_level = (level_num - 1)-i;
            break;
        }
    }

    last_cycles = health.cycles;
}

uint16_t bat_cycles_calc(int16_t current,float k) //需要1S执行一次
{
    uint16_t cycles;
    health.used_mas += k * (current < 0 ? -current : 0); // 只累计放电电流
    cycles =(uint16_t)( health.used_mas / BAT_CAP);
	return  cycles;
}

typedef struct
{
    int a_node;
    float b_node;
} soh_data_table_t;

soh_data_table_t soh_data_table[] = {
    {0, 100.0},
    {200, 93.3},
    {400, 86.7},
    {600, 80.0},
    {800, 73.4},
    {1000, 66.7},
    {1200, 60.0},
    {1400, 53.3},
    {1600, 47.6},
    {1800, 40.0},
    {2000, 33.3},
    {2200, 26.7},
    {2400, 20.0},
    {2600, 13.3},
    {2800, 6.7},
    {3000, 0.0},
};

float soh_calculus_interpolation(int a)
{
    int num_ranges = sizeof(soh_data_table) / sizeof(soh_data_table[0]);
    // 输入值在数据表范围外时的处理
    if (a <= soh_data_table[0].a_node)
    {
        return soh_data_table[0].b_node;
    }
    if (a >= soh_data_table[num_ranges - 1].a_node)
    {
        return soh_data_table[num_ranges - 1].b_node;
    }
    for (int i = 0; i < num_ranges - 1; i++)
    {

        if (a >= soh_data_table[i].a_node && a <= soh_data_table[i + 1].a_node)
        {
            // 线性插值计算
            float ret = soh_data_table[i].b_node -
                        ((float)(a - soh_data_table[i].a_node) /
                         (soh_data_table[i + 1].a_node - soh_data_table[i].a_node)) *
                            (soh_data_table[i + 1].b_node - soh_data_table[i].b_node);
            return ret;
        }
    }
    // 如果未能找到，默认返回最后一个 b_node
    return soh_data_table[num_ranges - 1].b_node;
}

typedef struct
{
    int a_node;
    float b_node;
} k_data_table_t;
#if 0
k_data_table_t k_data_table[] = {
    {0, 4.11},
    {5, 4.01},
    {10, 1},
    {45, 80.0},
    {55, 73.4},
    {60, 66.7},
};

float k_calculus_interpolation(int a)
{
    int num_ranges = sizeof(k_data_table) / sizeof(k_data_table[0]);
    // 输入值在数据表范围外时的处理
    if (a <= k_data_table[0].a_node)
    {
        return k_data_table[0].b_node;
    }
    if (a >= k_data_table[num_ranges - 1].a_node)
    {
        return k_data_table[num_ranges - 1].b_node;
    }
    for (int i = 0; i < num_ranges - 1; i++)
    {

        if (a >= k_data_table[i].a_node && a <= k_data_table[i + 1].a_node)
        {
            // 线性插值计算
            float ret = k_data_table[i].b_node -
                        ((float)(a - k_data_table[i].a_node) /
                         (k_data_table[i + 1].a_node - k_data_table[i].a_node)) *
                            (k_data_table[i + 1].b_node - k_data_table[i].b_node);
            return ret;
        }
    }
    // 如果未能找到，默认返回最后一个 b_node
    return k_data_table[num_ranges - 1].b_node;
}
#endif

float k_soh_calc(float temp)
{
    if (/*temp >= 0 && */ temp < 5)
    {
        return 4.11/4.01;
    }
    if(temp >= 5 && temp < 10 )
    {
        return 4.11/4.08;
    }
    if(temp >= 10 && temp < 45)
    {
        return 1.0;
    }
    if(temp >= 45 && temp < 55)
    {
        return 4.11/4.17;
    }
    if(temp >= 55 /*&& temp < 60 */)
    {
        return 4.11/4.18;
    }
}