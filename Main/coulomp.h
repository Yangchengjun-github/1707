#ifndef _COULOMP_H
#define _COULOMP_H
#include "math.h"

#include "stdio.h"
#include "stdint.h"


#define TIME_TASK_COULOMP_CALL (1000)
#define BAT_CAP (8000 * 3600)
typedef struct 
{
    int32_t total_cap;
    int32_t residue_cap;
    int16_t current;
    int16_t percent;
}coulomp_t;

extern coulomp_t coulomp;
extern void coulomp_init(void);
extern void task_coulomp( void);
float estimate_soc_from_voltage(int voltage_mv);


#endif
