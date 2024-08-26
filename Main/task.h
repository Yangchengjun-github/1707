#ifndef _TASK_H_
#define _TASK_H_


typedef  unsigned char  U8_T;
typedef  unsigned short  U16_T;  

typedef struct
{
	U8_T Run;					// 任务状态：Run/Stop
	U16_T TIMCount;			// 定时计数器
	U16_T TRITime;			// 重载计数器
	void (*TaskHook)(void); // 任务函数
} TASK_COMPONENTS;



void Task_Marks_Handler_Callback(void);
void Task_Pro_Handler_Callback(void);

#endif

