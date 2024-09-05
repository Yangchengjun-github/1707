#include "bathealth.h"

#include "app.h"

#include "coulomp.h"

health_t health = {0};



uint8_t health_calc(int16_t current);

void health_init(void)
{
    sys.bat.health = 100;
}

static const uint8_t table[] = {94,89,83,77,71,66,60};

void task_health(void)
{
    uint8_t temp = 0 ;
    temp = health_calc(coulomp.current);
    //printf("temp %d\n",temp);
    for(int i = 0; i< 7; i++)
    {
        if(temp >= table[i])
        {
            sys.bat.health = 7-i;
            break;
        }
        sys.bat.health = 0;
    }
   
}

uint8_t health_calc(int16_t current)
{
    health.used_mhs +=  (current < 0 ? 0-current : current) ;
    return  100 - (health.used_mhs / BAT_CAP / 100); // 每循环100次 寿命减1
}