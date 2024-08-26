#ifndef _QUEUE_H
#define _QUEUE_H

#define BUFFER_SIZE 50

#include "communication.h"
#include "stdio.h"
typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
}circ_buffer_t;

uint8_t circ_buffer_push(circ_buffer_t *cb, uint8_t data);

void ProcessData(circ_buffer_t *cb);
void ProcessData1(circ_buffer_t *cb);
void output_buf(uint8_t *buf, uint16_t len);
extern circ_buffer_t rxBuffer;
#endif

