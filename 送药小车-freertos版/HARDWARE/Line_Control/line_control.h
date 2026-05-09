#ifndef __LINE_CONTROL_H
#define __LINE_CONTROL_H
#include "stm32f4xx.h"

// 全局变量（巡线相关）
extern volatile int16_t line_error;    // 巡线偏差值
extern volatile uint8_t error_update; // 偏差值更新标志
extern volatile int16_t base_speed;   // 基础速度
extern volatile uint8_t back_flag;   // 返程标志
void task_go_trace(void* arg) ;
void line_trace(void);
void task_back_trace (void*arg);
#endif

