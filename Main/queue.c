#include "queue.h"
#include "app.h"

#include "debug.h"
circ_buffer_t rxBuffer;




void circ_buffer_init(circ_buffer_t *cb)
{
    cb->head = 0;
    cb->tail = 0;
}

uint8_t circ_buffer_push(circ_buffer_t *cb, uint8_t data)
{
    uint8_t next = (cb->head + 1) % BUFFER_SIZE;
    if (next == cb->tail)
    {
        return 0;
    }
    cb->buffer[cb->head] = data;
    cb->head = next;
    return 1;
}

uint8_t circ_buffer_pop(circ_buffer_t *cb, uint8_t *data)
{
    if (cb->head == cb->tail)
    {
        return 0;
    }
    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    return 1;
}




void output_buf(uint8_t *buf, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\r\n");

}

void ProcessPacket(uint8_t *packet, uint16_t length)
{
    uint8_t temp;
    // 处理接收到的数据包
    	//printf("收到包%d:",xSystem.num_pack++);
#if(LOG_DEBUG)
        output_buf(packet, length);
#endif
    uint8_t sum = 0, i = 0;
 //   static uint8_t cnt = 0;
    memset(&field_receive.byte, 0, sizeof(field_receive));
    memcpy(&field_receive.parameter.un_cmd1.byte, packet, sizeof(field_receive)-1);
    //output_buf(packet, length);
    for (i = 0; i < sizeof(field_receive)-3 ; i++)
    {
        sum += (packet)[i];
    }
    
    sum = sum + (uint8_t)0x3c;
#if(LOG_DEBUG)
	printf("sum:%x\n",sum);
#endif
    if (sum == field_receive.parameter.sum1)
    {
        if (field_receive.parameter.sum2 == (field_receive.parameter.sum1 ^ 0xff))
        {
            // data_printf(&field_receive.byte,sizeof(field_receive));
            receive_ok = 1;
#if(LOG_DEBUG)
            printf("[接收到主机数据cmd1:] %d\n", field_receive.parameter.un_cmd1.byte);
#endif
            sys.port.C1_status = field_receive.parameter.un_cmd1.bit_field.ca_status;
            sys.port.C2_status = field_receive.parameter.un_cmd1.bit_field.cb_status;
            sys.port.PG_status = field_receive.parameter.un_cmd1.bit_field.pg_status;
            
        }
        else
        {
            printf("sum2_err\n");
        }
            
    }
    else
    {
        
        printf("sum1_err clac_sum  %d but data %d \n" ,sum, field_receive.parameter.sum1);
        if(field_receive.parameter.sum1 == 0x3c && field_receive.parameter.un_cmd1.byte == 0xc3)
        {
            circ_buffer_pop(&rxBuffer, &temp);//扔掉一个数据
        }
    }
}

/**
 * @brief 利用帧头帧尾处理接收到的不定长数据
 * @param [in] cb 
 * 
 * @details 
 */
void ProcessData(circ_buffer_t *cb)
{
    static uint8_t packet[BUFFER_SIZE];
    static uint16_t packetIndex = 0;
    static uint8_t receiving = 0;
    uint8_t data;

    while (circ_buffer_pop(cb, &data))
    {
        if (data == 0x02)
        { // STX
            packetIndex = 0;
            receiving = 1;
        }
        else if (data == 0x03)
        { // ETX
            if (receiving)
            {
                // Process complete packet
                ProcessPacket(packet, packetIndex);
                receiving = 0;
            }
        }
        else
        {
            if (receiving)
            {
                packet[packetIndex++] = data;
                if (packetIndex >= BUFFER_SIZE)
                {
                    // Packet too large, reset
                    receiving = 0;
                }
            }
        }
    }
}
/**
 * @brief 利用帧头加上帧大小处理接收到的定长数据
 * @param [in] cb 
 * 
 * @details 
 */
void ProcessData1(circ_buffer_t *cb)
{
    static uint8_t packet[BUFFER_SIZE];
    static uint16_t packetIndex = 0;
    static uint8_t receive_status = 0;
    uint8_t data;

    while (circ_buffer_pop(cb, &data))
    {
        switch(receive_status)
        {
        case 0:
            if (data == 0x3C)
            {
                packetIndex = 0;
                receive_status = 1;
            }
            break;
        

        case 1:
            packet[packetIndex++] = data;
            if ( 3 == packetIndex)  //一个字节
            {
                receive_status = 0;              
                ProcessPacket(packet, packetIndex);
            }
            if (packetIndex >= BUFFER_SIZE)
            {
                // Packet too large, reset
                receive_status = 0;
            }
            break;
        default:   
            break;
        }
       // printf("status:%d\n",receive_status);   
       
    }
}
