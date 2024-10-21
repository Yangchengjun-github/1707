
#include "cs32f10x_exti.h"
#include "cs32f10x_gpio.h"
#include "cs32f10x_i2c.h"
#include "cs32f10x_misc.h"
#include "cs32f10x_rcu.h"
#include "string.h"
#include "iic.h"
#include "bms.h"
#include "stdio.h"
#include "app.h"
#define I2C_TIMEOUT_MAX 0x1FFF
#define WIAT_TIME  (30000)


/**
 * @struct i2c_error_t
 *
 * @brief  I2C error code
 */
typedef enum
{
    I2C_SUCCESS = 0,
    I2C_ERROR_TIMEOUT = 1,
} i2c_error_t;



/**@brief       I2C configure
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void i2c_init_(void)
{
    /* Enable the clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    /*SCL*/
    gpio_mode_config(GPIOB, GPIO_PIN_6, GPIO_MODE_OUT_AFOD(GPIO_SPEED_MEDIUM));
    /*SDA*/
    gpio_mode_config(GPIOB, GPIO_PIN_7, GPIO_MODE_OUT_AFOD(GPIO_SPEED_MEDIUM));

    i2c_config_t i2c_struct;

    __RCU_APB1_CLK_ENABLE(RCU_APB1_PERI_I2C1);
    /* initialize the I2C mode */
    i2c_struct.mode = I2C_MODE_I2C;
    /* initialize the I2C speed 100KHz */
    i2c_struct.speed = 100000;
    /* initialize the I2C duty cycle Tlow/Thigh = 2 */
    i2c_struct.duty_cycle = I2C_DUTY_CYCLE_2;
    /* initialize the I2C address1 */
    i2c_struct.address1 = 0xAA;
    /* initialize the I2C ack enable */
    i2c_struct.ack = I2C_ACK_ENABLE;
    /* initialize the I2C address mode */
    i2c_struct.addr_mode = I2C_ADDRESS_MODE_7BITS;

    i2c_init(I2C1, &i2c_struct);

    /*Enable I2C*/
    __I2C_ENABLE(I2C1);
}



/**@brief       I2C master write data
 *
 * @param[in]   ptr_i2c: pointer to I2Cx where x can be 1 or 2 to select I2C peripheral.
 *
 * @param[in]   addr: I2C slave address.
 *
 * @param[in]   data: I2C write data.
 *
 * @param[in]   len: I2C write data length.
 *
 * @return      I2C write result,I2C_SUCCESS or I2C_ERROR_TIMEOUT.
 */
uint8_t i2c_master_write(i2c_reg_t *ptr_i2c, uint8_t addr, uint8_t *data, uint16_t len)
{
    uint16_t i2c_timeout = 0;
    uint16_t i = 0;
    uint8_t value[256] = {0};

    /*The write max length is 256*/
    if (len > 256)
    {
        len = 256;
    }
    memcpy(value, data, len);

    /*I2C start*/
    __I2C_GENSTART_ENABLE(ptr_i2c);

    i2c_timeout = I2C_TIMEOUT_MAX;
    /*Check the start signal*/
    while (i2c_flag_status_check(ptr_i2c, I2C_STR1_FLAG_START_BIT) == RESET)
    {
        i2c_timeout--;
        if (i2c_timeout == 0)
        {
            return I2C_ERROR_TIMEOUT;
        }
    }
    /*I2C address and write signal*/
    i2c_master_send7bits_address(ptr_i2c, addr, I2C_DIRECT_WRITE);

    i2c_timeout = I2C_TIMEOUT_MAX;
    while (i2c_event_check(ptr_i2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR)
    {
        i2c_timeout--;
        if (i2c_timeout == 0)
        {
            return I2C_ERROR_TIMEOUT;
        }
    }

    /*Write the data to slave*/
    for (i = 0; i < len; i++)
    {
        __I2C_DATA_SEND(ptr_i2c, data[i]);
        i2c_timeout = I2C_TIMEOUT_MAX;
        while ((i2c_flag_status_check(ptr_i2c, I2C_STR1_FLAG_TXEF) == RESET) &&
               (i2c_flag_status_check(ptr_i2c, I2C_STR1_FLAG_BTFS) == RESET))
        {
            i2c_timeout--;
            if (i2c_timeout == 0)
            {
				printf("iic_error");
                return I2C_ERROR_TIMEOUT;
				
            }
        }
    }
    /*I2C stop signal*/
    __I2C_GENSTOP_ENABLE(ptr_i2c);

    return I2C_SUCCESS;
}

/**@brief       I2C master read data.
 *
 * @param[in]   ptr_i2c: pointer to I2Cx where x can be 1 or 2 to select I2C peripheral.
 *
 * @param[in]   addr: I2C slave address.
 *
 * @param[out]  data: I2C read data.
 *
 * @param[in]   len: I2C read length.
 *
 * @return      I2C read result,I2C_SUCCESS or I2C_ERROR_TIMEOUT.
 */
uint8_t i2c_master_read(i2c_reg_t *ptr_i2c, uint8_t addr, uint8_t *data, uint16_t len)
{
    uint16_t i2c_timeout = 0;
    uint16_t i = 0;
    uint8_t value[256] = {0};

    /*I2C start*/
    __I2C_GENSTART_ENABLE(ptr_i2c);

    i2c_timeout = I2C_TIMEOUT_MAX;
    /*Check the start signal*/
    while (i2c_flag_status_check(ptr_i2c, I2C_STR1_FLAG_START_BIT) == RESET)
    {
        i2c_timeout--;
        if (i2c_timeout == 0)
        {
            return I2C_ERROR_TIMEOUT;
        }
    }

    /*I2C address and read signal*/
    i2c_master_send7bits_address(ptr_i2c, addr, I2C_DIRECT_READ);

    i2c_timeout = I2C_TIMEOUT_MAX;
    while (i2c_event_check(ptr_i2c, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR)
    {
        i2c_timeout--;
        if (i2c_timeout == 0)
        {
            return I2C_ERROR_TIMEOUT;
        }
    }

    /*I2C send ACK*/
    __I2C_ACK_ENABLE(ptr_i2c);

    /*I2C read data*/
    for (i = 0; i < len; i++)
    {
        while ((i2c_flag_status_check(ptr_i2c, I2C_STR1_FLAG_RXNEF) == RESET))
        {
            i2c_timeout--;
            if (i2c_timeout == 0)
            {
                return I2C_ERROR_TIMEOUT;
            }
        }
        value[i] = __I2C_DATA_RECV(ptr_i2c);
        if (i < len)
        {
            /*I2C send ACK*/
            __I2C_ACK_ENABLE(ptr_i2c);
        }
    }
    /*I2C send NACK*/
    __I2C_ACK_DISABLE(ptr_i2c);
    /*I2C send stop signal*/
    __I2C_GENSTOP_ENABLE(ptr_i2c);
    memcpy(data, value, len);
    return I2C_SUCCESS;
}




void iic_test(void)
{

   static uint8_t data[4] = {0x00, 0x01, 0x02, 0x03};

	data[3]++;
//    I2C_WriteData(0x11, 0x00, data[0]);
//    I2C_WriteData(0x11, 0x01, data[1]);
//    I2C_WriteData(0x11, 0x02, data[2]);
    //I2C_WriteData(0x08, 0x03, data[3]);
	// I2C_ReadData(0x04,0x03);
   // I2C_WriteMulti(0x08, 0x00, data, 4);
    //I2C_ReadMulti(0x08, 0x00, readBuf, 4);


    //i2c_master_write(I2C1, 0x00, data, 4);

    //i2c_master_read(I2C1, 0x00, readBuf, 4);
	//printf("readBuf: 0x%x 0x%x 0x%x 0x%x\r\n", readBuf[0], readBuf[1], readBuf[2], readBuf[3]);
 

}

#define SCL_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_6
#define SDA_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_7



#define DELAY_NOP  100

void i2c_nop(volatile uint16_t t)
{
	while(t--)
	{
		__nop();
	}
}


/**@brief       I2C configure
 *
 * @param[in]   None.
 *
 * @return      None.
 */
void i2c_init_2(void)
{
    /* Enable the clock */
    __RCU_APB2_CLK_ENABLE(RCU_APB2_PERI_GPIOB);
    /*SCL*/
    gpio_mode_config(GPIOB, GPIO_PIN_6, GPIO_MODE_OUT_OD(GPIO_SPEED_MEDIUM));
    /*SDA*/
    gpio_mode_config(GPIOB, GPIO_PIN_7, GPIO_MODE_OUT_OD(GPIO_SPEED_MEDIUM));


    
    __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
    __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);

    
}




void I2C_Start(void)
{
    __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);
    __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
    i2c_nop(DELAY_NOP);
    __GPIO_PIN_RESET(SDA_GPIO_Port, SDA_Pin);
   i2c_nop(DELAY_NOP);
    __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
	i2c_nop(DELAY_NOP);
}

void I2C_Stop(void)
{
    uint16_t wait = 0;
    __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
    __GPIO_PIN_RESET(SDA_GPIO_Port, SDA_Pin);
    i2c_nop(DELAY_NOP);
    __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
    while (!__GPIO_INPUT_PIN_GET(SCL_GPIO_Port, SCL_Pin) && wait < WIAT_TIME)
    {
        wait++;
    }
    if (wait >= WIAT_TIME)
    {
        sys.flag.iic_err = 1;
        return;
    }
    __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
    i2c_nop(DELAY_NOP);
    __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);
}

void I2C_SendAck(uint8_t ack)
{
    if(ack)
    {
        __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);
    }
    else
    {
        __GPIO_PIN_RESET(SDA_GPIO_Port, SDA_Pin);
    }
    
    __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
    i2c_nop(DELAY_NOP);
    __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
    i2c_nop(DELAY_NOP);
}

uint8_t I2C_WaitAck(void)
{
    uint16_t wait = 0;

    uint8_t ack;
//    while (t--)
//    {
//        IIC_DELAY;
//    }
    __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);
        __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
        while (!__GPIO_INPUT_PIN_GET(SCL_GPIO_Port, SCL_Pin) && wait < WIAT_TIME)
        {
            wait++;
        }
        if (wait >= WIAT_TIME)
        {
            sys.flag.iic_err = 1;
            
        }
        __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
		 i2c_nop(DELAY_NOP);
  
    ack = __GPIO_INPUT_PIN_GET(SDA_GPIO_Port, SDA_Pin);
    __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
    i2c_nop(DELAY_NOP);
	if(ack == 1)
	{
		xbms.nack_cnt++;
	}
	
    if(ack)
    {
        printf("ack_n :%d\n", xbms.ack_total);
    }
    xbms.ack_total++;
    return ack;
}

void I2C_SendByte(uint8_t byte)
{
    uint16_t wait = 0;
    __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
    i2c_nop(DELAY_NOP);
    for (uint8_t i = 0; i < 8; i++)
    {
        if(byte & 0x80)
        {
            __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);
        }
        else
        {
            __GPIO_PIN_RESET(SDA_GPIO_Port, SDA_Pin);
        }
        
        byte <<= 1;
        i2c_nop(DELAY_NOP);
        __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
        while (!__GPIO_INPUT_PIN_GET(SCL_GPIO_Port, SCL_Pin) && wait < WIAT_TIME)
        {
            wait++;
        }
        if (wait >= WIAT_TIME)
        {
            sys.flag.iic_err = 1;
            return;
        }

        i2c_nop(DELAY_NOP);
        __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
        i2c_nop(DELAY_NOP);

    }
}

uint8_t I2C_ReceiveByte(void)
{
    uint8_t byte = 0;
    uint16_t wait = 0;
    __GPIO_PIN_SET(SDA_GPIO_Port, SDA_Pin);
	
    for (uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;
        __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
        while (!__GPIO_INPUT_PIN_GET(SCL_GPIO_Port, SCL_Pin) && wait < WIAT_TIME)
        {
            wait++;
        }
        if (wait >= WIAT_TIME)
        {
            sys.flag.iic_err = 1;
            
        }
        __GPIO_PIN_SET(SCL_GPIO_Port, SCL_Pin);
        i2c_nop(DELAY_NOP);
        if (__GPIO_INPUT_PIN_GET(SDA_GPIO_Port, SDA_Pin))
        {
            byte |= 0x01;
        }
        __GPIO_PIN_RESET(SCL_GPIO_Port, SCL_Pin);
        i2c_nop(DELAY_NOP);
    }

    return byte;
}

void I2C_WriteData(uint8_t deviceAddr, uint8_t regAddr, uint8_t data)
{
    I2C_Start();
    I2C_SendByte(deviceAddr << 1);
    I2C_WaitAck();
    I2C_SendByte(regAddr);
    I2C_WaitAck();
    I2C_SendByte(data);
    I2C_WaitAck();
    I2C_Stop();
}

uint8_t I2C_ReadData(uint8_t deviceAddr, uint8_t regAddr)
{
    uint8_t data;

    I2C_Start();
    I2C_SendByte(deviceAddr << 1);
    I2C_WaitAck();
    I2C_SendByte(regAddr);
    I2C_WaitAck();

    I2C_Start();
    I2C_SendByte((deviceAddr << 1) | 1);
    I2C_WaitAck();
    data = I2C_ReceiveByte();
    I2C_SendAck(1);
    I2C_Stop();

    return data;
}
void I2C_WriteMulti(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint8_t length)
{
    I2C_Start();
    I2C_SendByte(deviceAddr << 1); // 发送从设备地址（写）
    I2C_WaitAck();
    I2C_SendByte(regAddr); // 发送目标寄存器地址
    I2C_WaitAck();

    for (uint8_t i = 0; i < length; i++)
    {
        I2C_SendByte(data[i]); // 依次发送数据字节
        I2C_WaitAck();
    }

    I2C_Stop(); // 发送停止条件
}
void I2C_ReadMulti(uint8_t deviceAddr, uint8_t regAddr, uint8_t *data, uint8_t length)
{
    I2C_Start();
    I2C_SendByte(deviceAddr << 1); // 发送从设备地址（写）
    I2C_WaitAck();
    I2C_SendByte(regAddr); // 发送目标寄存器地址
    I2C_WaitAck();

    I2C_Start();
    I2C_SendByte((deviceAddr << 1) | 1); // 发送从设备地址（读）
    I2C_WaitAck();

    for (uint8_t i = 0; i < length; i++)
    {
        data[i] = I2C_ReceiveByte(); // 依次读取数据字节
        if (i < length - 1)
        {
            I2C_SendAck(0); // 发送ACK信号
        }
        else
        {
            I2C_SendAck(1); // 最后一个字节发送NACK
        }
    }

    I2C_Stop(); // 发送停止条件
}


