#ifndef __YL_62_H
#define __YL_62_H
#include "stm32f4xx.h"


void YL62_OUT_Init(void);
void YL62_EXTI_Init(void);
uint8_t YL62_Get_State(void);

extern volatile uint8_t YL62_flag; 
extern volatile uint8_t YL62_state;
#endif

