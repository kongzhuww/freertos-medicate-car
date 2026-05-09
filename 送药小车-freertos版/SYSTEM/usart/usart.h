#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

void USART2_Init(u32 bound);
void USART3_Init(u32 bound);
void UART4_Init(u32 bound);

void USART3_SendString(char *str);
void USART3_SendByte(u8 byte);

extern volatile uint8_t uart2_received;
extern volatile uint8_t stop_flag;
 extern volatile uint8_t turn_flag;
 extern volatile uint8_t left_flag;
extern volatile uint8_t right_flag;
#endif


