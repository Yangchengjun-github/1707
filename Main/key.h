
#ifndef _KEY_H
#define _KEY_H
#include "cs32f10x_gpio.h"
#include "cs32f10x_rcu.h"
#define TIME_TASK_KEY_CALL (10)

#define KEY_NUM_MAX 2

#define KEY1_PORT GPIOC
#define KEY1_PIN GPIO_PIN_13
#define KEY2_PORT GPIOA
#define KEY2_PIN GPIO_PIN_1

#define KEY1_IO_LEVEL  __GPIO_INPUT_PIN_GET(KEY1_PORT, KEY1_PIN)
#define KEY2_IO_LEVEL  __GPIO_INPUT_PIN_GET(KEY2_PORT, KEY2_PIN)

typedef struct
{
    unsigned char long_press : 1;
    unsigned char short_press : 1;
    unsigned char three_press : 1;
    unsigned char flag_short_press : 1;
    unsigned char lock : 1;
    unsigned char null : 3;
    unsigned char timer;
    unsigned char timer_interval;
    unsigned char count_press;
} key_cb_T;
extern key_cb_T key_cb[KEY_NUM_MAX];

enum
{
    KEY_ONOFF = 0,
    KEY_PULSE,
    KEY_CONTINUE,

};

//#define KEY_IO(i)              \
//    (                          \
//        {                      \
//            unsigned char io = 0;         \
//            switch (i)         \
//            {                  \
//            case 0:            \
//                io = KEY1_IO_LEVEL; \
//                break;         \
//            case 1:            \
//                io = KEY2_IO_LEVEL; \
//                break;         \
//            default:           \
//                break;         \
//            }                  \
//            return io;                \
//        })

static inline unsigned char KEY_IO(int i)
{
    unsigned char io = 0;
    switch (i)
    {
        case 0:
            io = KEY1_IO_LEVEL;
            break;
		case 1:            
            io = KEY2_IO_LEVEL; 
            break;  
        default:
            break;
    }
    return io;
}		
		
		
void task_key(void);
void key_init(void);

#endif
