#ifndef _BATHEALTH_H
#define _BATHEALTH_H

#define TIME_TASK_BATHEALTH_CALL (1000)

#include "stdint.h"
typedef struct 
{
    uint64_t used_mhs;
}health_t;

extern health_t health;
void health_init(void);
void task_health(void);

#endif

