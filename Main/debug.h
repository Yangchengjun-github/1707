#ifndef _DEBUG_H
#define _DEBUG_H
#include "stdio.h"
#include "stdint.h"


#define TIME_TASK_DEBUG_CALL (1000)
#define LOG_DEBUG    (0)
void task_debug(void);

#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 3

#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO

#define LOG(level, fmt, ...)                                                                  \
    if (level >= CURRENT_LOG_LEVEL)                                                           \
    {                                                                                         \
        printf("[%s] %s:%d:%s(): " fmt, #level, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    }

#endif

