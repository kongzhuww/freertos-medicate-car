#ifndef __PWM_H
#define __PWM_H

#include "stm32f4xx.h"
void TIM1_PWMA_CH1_Init(u16 arr1, u16 psc1);
void TIM1_PWMB_CH3_Init(u16 arr2, u16 psc2);

#endif

