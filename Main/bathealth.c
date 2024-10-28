#include "bathealth.h"

#include "app.h"

#include "coulomp.h"

health_t health = {0};



float health_calc(int16_t current);

void health_init(void)
{
    sys.bat.soh = 100;
}

static const uint8_t table[] = {94,89,83,77,71,66,60};

void task_health(void)
{
    sys.bat.soh = health_calc(coulomp.current);

    for(int i = 0; i< 7; i++)
    {
        if (sys.bat.soh >= table[i])
        {
            sys.bat.soh_level = 7-i;
            break;
        }
        sys.bat.soh_level = 0;
    }

}

float health_calc(int16_t current)
{
    health.used_mhs +=  (current < 0 ? 0-current : current) ;
    return 100.0 - (health.used_mhs / 100.0 /  BAT_CAP ); // 每循环100次 寿命减1
}

