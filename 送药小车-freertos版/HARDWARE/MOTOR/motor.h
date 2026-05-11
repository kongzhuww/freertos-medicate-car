#ifndef __MOTOR_H
#define __MOTOR_H
#include "stdint.h"



void MOTOR_Init(void);
void Motor1_Set(int16_t speed);
void Motor2_Set(int16_t speed);
void TURN_LEFT(void);
void TURN_RIGHT(void);
void TURN_FORWARD(void);
void TURN_AROUND(void);
void TURN_LEFT_BACK(void);
void TURN_RIGHT_BACK(void);
#endif

