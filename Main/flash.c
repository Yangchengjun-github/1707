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
    flash_data.used_mas = health.used_mas;

    flash_write(ADDR_DATA, (uint32_t*)(&flash_data), sizeof(flash_data)/ sizeof(uint32_t));
    printf("flash_write :soh %f, mhs %llu", sys.bat.soh, health.used_mas);
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
        health.used_mas = flash_data.used_mas;
        printf("flash_read :soh %f, mhs %llu",sys.bat.soh,health.used_mas);
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
