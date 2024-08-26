#ifndef _IIC_H
#define _IIC_H
#include "cs32f10x_i2c.h"



void i2c_init_(void);
uint8_t i2c_master_write(i2c_reg_t *ptr_i2c, uint8_t addr, uint8_t *data, uint16_t len);

uint8_t i2c_master_read(i2c_reg_t *ptr_i2c, uint8_t addr, uint8_t *data, uint16_t len);
void iic_test(void);

void i2c_init_2(void);

void I2C_WriteData(uint8_t deviceAddr, uint8_t regAddr, uint8_t data);

uint8_t I2C_ReadData(uint8_t deviceAddr, uint8_t regAddr);

void I2C_WriteMulti(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint8_t length);

void I2C_ReadMulti(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint8_t length);

#endif


