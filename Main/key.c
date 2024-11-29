
#include "key.h"
#include "string.h"
key_cb_T key_cb[KEY_NUM_MAX];

// 10MS 任务

void key_init(void)
{
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOC);
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOA);

	gpio_mode_config(KEY1_PORT, KEY1_PIN, GPIO_MODE_IN_PU);
    gpio_mode_config(KEY2_PORT, KEY2_PIN, GPIO_MODE_IN_PU);


    memset(key_cb, 0, sizeof(key_cb));

}
/**
 * @description:
 * @return {*}
 */
void task_key(void)
{
    unsigned char  i = 0;
    for (i = 0; i < KEY_NUM_MAX; i++)
    {
        if (key_cb[i].timer_interval < 255)
            key_cb[i].timer_interval++;
        if (KEY_IO(i))
        {
            key_cb[i].lock = 0;
            key_cb[i].timer = 0;
            if (key_cb[i].flag_short_press)
            {

                if (key_cb[i].timer_interval < 100)
                {
                    key_cb[i].count_press++;
                }
                else
                {
                    key_cb[i].count_press = 1;
                }
                if (key_cb[i].count_press >= 5)
                {
                    key_cb[i].count_press = 0;
                    key_cb[i].three_press = 1;
                }
                key_cb[i].flag_short_press = 0;
                key_cb[i].short_press = 1;
                key_cb[i].timer_interval = 0;
            }
        }
        else if (!key_cb[i].lock)
        {
            key_cb[i].timer++;
            if (key_cb[i].timer > 5)
            {
                key_cb[i].flag_short_press = 1;
            }
            if (key_cb[i].timer > 200)
            {
                key_cb[i].long_press = 1;
                key_cb[i].flag_short_press = 0;
                key_cb[i].lock = 1;
            }
        }
    }
}

