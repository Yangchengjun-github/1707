#include "flash.h"
/***************************************************************************/ /**
                                                                               * @file        FLASH/PROGRAM/main.c
                                                                               * @version     V1.1.0
                                                                               * @author      Software Development
                                                                               * @brief       Main program body.
                                                                               * @copyright   Copyright (C) Software Development. All rights reserved.
                                                                               ****************************************************************************/

#include "cs32f10x_flash.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_rcu.h"
#include "stdint.h"
#include "stdio.h"
#include "app.h"
#include "bathealth.h"
#define ADDR_DATA 0x0801F000 //124页
#define LED_DELAY 0x5FFFF

void led_init(void);
void led1_toggle(void);
void led2_toggle(void);
void led1_on(void);
void led2_on(void);

void delay(__IO uint32_t count);

typedef enum
{
    ERR_PROGRAM = 0,
    ERR_OK = !ERR_PROGRAM
} Status;
#define FLASH_PAGE_SIZE ((uint16_t)0x400)
#define START_ADDR ((uint32_t)0x08001000)
#define END_ADDR ((uint32_t)0x08020000)

uint32_t erase_page_counter = 0x00, address = 0x00;
uint32_t Data = 0x00;
uint32_t page_total = 0x00;
volatile flash_status_t flash_status = FLASH_STATUS_COMPLETE;
volatile Status Program_Status = ERR_OK;

typedef struct 
{
    uint64_t used_mas; //累计容量
    uint8_t health_per; //
    uint8_t flash_valid;//数据有效性
}flash_data_t;


void flash_write(uint32_t addr,uint32_t *datap,uint8_t word_len)
{
    flash_unlock();
    uint8_t i = 0;
    flash_page_erase(addr);

    for (i = 0; i < word_len; i++)
    {
        flash_word_program(addr+i*4, datap[i]);
    }
    flash_lock();
}

void flash_read(uint32_t addr, uint32_t *datap, uint8_t word_len)
{
    uint8_t i = 0;
    for (i = 0; i < word_len; i++)
    {
        *(datap + i) = *(__IO uint32_t *)(addr + i* 4);
    }
}

void app_flash_save(void)
{

    flash_data_t flash_data = {0};
    flash_data.flash_valid = 0xaa;
    flash_data.health_per =  sys.bat.soh;
    flash_data.used_mas = health.used_mhs;

    flash_write(ADDR_DATA, (uint32_t*)(&flash_data), sizeof(flash_data)/ sizeof(uint32_t));
    printf("-----flash_write :soh %f, mhs %d", sys.bat.soh, health.used_mhs);
}


void app_flash_read(void)
{
    flash_data_t flash_data = {0};
    flash_read(ADDR_DATA, (uint32_t *)(&flash_data), sizeof(flash_data) / sizeof(uint32_t));
    if(flash_data.flash_valid != 0xaa)
    {
        printf("数据无效\n");
    }
    else
    {
        sys.bat.soh = flash_data.health_per;
        health.used_mhs = flash_data.used_mas;
        printf("flash_read :soh %f, mhs %d",sys.bat.soh,health.used_mhs);
    }
}

uint32_t arr[] = { 1,
                   2,
                   3,
                   4,
                   5,
                   6,
                   7,
                   8 };

void flash_test(void)
{
    uint32_t arr_r[8] = {0};
    flash_write(ADDR_DATA,(uint32_t *)arr,8);
    flash_read(ADDR_DATA,(uint32_t *)arr_r,8);
    uint8_t i = 0;
    for(i = 0; i < 8; i++)
    {
        printf("%d ",arr_r[i]);
    }
    printf("\n");
}

#if 0
int main(void)
{
    /* Initialization LED */
    led_init();

    flash_unlock();

    page_total = (END_ADDR - START_ADDR) / FLASH_PAGE_SIZE;

    __FLASH_FLAG_CLEAR(FLASH_FLAG_PGERR | FLASH_FLAG_WPERR | FLASH_FLAG_ENDF);

    for (erase_page_counter = 0; (erase_page_counter < page_total) && (flash_status == FLASH_STATUS_COMPLETE); erase_page_counter++)
    {
        flash_status = flash_page_erase(START_ADDR + (FLASH_PAGE_SIZE * erase_page_counter));
    }

    address = START_ADDR;

    while ((address < END_ADDR) && (flash_status == FLASH_STATUS_COMPLETE))
    {
        flash_status = flash_word_program(address, Data);
        address = address + 4;
        Data = Data + 4;
    }

    flash_lock();

    address = START_ADDR;
    Data = 0;
    while ((address < END_ADDR) && (Program_Status != ERR_PROGRAM))
    {
        if ((*(__IO uint32_t *)address) != Data)
        {
            Program_Status = ERR_PROGRAM;
            led1_on();
        }
        address += 4;
        Data += 4;
    }

    while (1)
    {
        if (Program_Status == ERR_OK)
        {
            led2_on();
        }
    }
}

/**@brief       Init LED1 and LED2.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void led_init(void)
{
    /* Enable the clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);

    gpio_mode_config(GPIOB, GPIO_PIN_15, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));
    gpio_mode_config(GPIOB, GPIO_PIN_14, GPIO_MODE_OUT_PP(GPIO_SPEED_HIGH));

    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_15); // LED1
    __GPIO_PIN_RESET(GPIOB, GPIO_PIN_14); // LED2
}

/**@brief       Toggle the LED.
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void led1_toggle(void)
{
    GPIOB->DO ^= GPIO_PIN_15;
}

void led1_on(void)
{
    GPIOB->SCR = GPIO_PIN_15;
}

void led2_toggle(void)
{
    GPIOB->DO ^= GPIO_PIN_14;
}

void led2_on(void)
{
    GPIOB->SCR = GPIO_PIN_14;
}

/**@brief       Software delay.
 *
 * @param[in]   count: the delay time length
 *
 * @return      None.
 */
void delay(__IO uint32_t count)
{
    for (; count != 0; count--)
        ;
}

#ifdef USE_FULL_ASSERT

/**@brief       Report the assert error.
 *
 * @param[in]   file: pointer to the source file name.
 *
 * @param[in]   line: error line source number.
 *
 * @return      None.
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    while (1)
        ;
}

#endif
#endif