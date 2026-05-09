#ifndef __KEY_H
#define __KEY_H
#include "stm32f4xx.h"

void KEY_Init(void);
void TIM2_Init(void);
 void TIM2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void KEY_EXTI_Init(void);



extern volatile  int count;
extern volatile uint8_t key_pressed1;
extern volatile uint8_t key_pressed2;
extern volatile uint8_t key_pressed3;
extern volatile uint8_t key_count3;
extern volatile uint8_t key_count4;
void SPEED_Choose (void*age);
void TURN_Choose (void*age);
#endif

