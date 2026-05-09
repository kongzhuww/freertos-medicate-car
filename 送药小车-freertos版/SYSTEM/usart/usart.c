#include "sys.h"
#include "usart.h"	
#include "stm32f4xx.h"
#include "line_control.h"
#include <stdlib.h>  
#include <string.h>
#include "led.h"
#include  "FreeRTOS.h"
#include "task.h"
#include "oled.h"

#define APB1_CLK 42000000
/////////////PA0----TX  PA1----RX//////////////////
volatile uint8_t uart2_received = 0;  // ���ձ�־��1=�յ����� 0=û�յ�
volatile uint8_t stop_flag = 0;       // ֹͣ��־��1=ֹͣ 0=����
volatile uint8_t turn_flag=0;

volatile uint8_t left_flag = 0;  //����4�ı�־λ
volatile uint8_t right_flag = 0;



void UART4_Init(u32 bound)
{ 
 RCC->APB1ENR|=1<<19;            //ʹ��USART4ʱ�� p139
 RCC->AHB1ENR|=1<<0;            //ʹ��PORTAʱ��
 
 GPIOA->MODER&=~(3<<0);        //����˿�  TX
 GPIOA->MODER|=2<<0;           //�������ģʽ p187
 GPIOA->AFR[0]&=~(0xF << 0);     //����˿�     p191
 GPIOA->AFR[0]|=8<<0;           //PA0����ΪUART4   ��AF8����PA0��AFR��
 GPIOA->PUPDR &=~(0x3 << 0);  //����˿� 
 GPIOA->OTYPER &= ~(1<<0);    // ������� 
 GPIOA->OSPEEDR |= (3<<0);    // ����
 GPIOA->PUPDR &= ~(0x3 << 0);     // ����������     p188


 GPIOA->MODER&=~(3<<2);        //����˿�  RX
 GPIOA->MODER|=2<<2;           //���ø���ģʽ
 GPIOA->AFR[0]&=~(0xF << 4);     //����˿�
 GPIOA->AFR[0]|=8<<4;           //PA1����ΪUART4     ��AF8����PA1��AFR��
 GPIOA->PUPDR &=~(0x3 << 2);  //����˿�  
 GPIOA->PUPDR|=1<<2;           //������������ �����������յ��µĸ����󴥷� p188

 UART4->CR1 &= ~(1<<13); 

    
 UART4->CR1 &= ~(1<<12);  // 8λ����ģʽ��Ĭ�ϣ�
 UART4->CR2 &= ~(3<<12);  // 1λֹͣλ��Ĭ�ϣ�
 UART4->CR1 &= ~(1<<10);  // ��У�飨Ĭ�ϣ�

 UART4->BRR = APB1_CLK/bound;

 UART4->CR1|=1<<2;          //���ڽ���ʹ��
 UART4->CR1|=1<<3;          //���ڷ���ʹ��
 UART4->CR1|=1<<5;          //ʹ�ܴ����жϴ���ʹ������жϷ������
 
 NVIC_SetPriority(UART4_IRQn, 1);
 NVIC_EnableIRQ(UART4_IRQn);   //ʹ��UART4�ж�ͨ��

 UART4->CR1|=1<<13;          //ʹ��USART2 �����жϣ�����USART���ж�ʹ��
}

//----------------PA2----TX  PA3---RX-------------------------------
void USART2_Init(u32 bound)
{ 
 RCC->APB1ENR|=1<<17;            //ʹ��USART2ʱ�� p144
 RCC->AHB1ENR|=1<<0;            //ʹ��PORTAʱ��
	
 GPIOA->MODER&=~(3<<4);        //����˿�  TX
 GPIOA->MODER|=2<<4;           //�������ģʽ p187
 GPIOA->AFR[0]&=~(0xF << 8);     //����˿�     p191
 GPIOA->AFR[0]|=7<<8;           //PA2����ΪUART2   ��AF7����PA2��AFR��
 GPIOA->PUPDR &=~(0x3 << 4);  //����˿�  
 GPIOA->OTYPER &= ~(1<<2);    // �������
 GPIOA->OSPEEDR |= (3<<4);    // ����
 GPIOA->PUPDR &= ~(3<<4);     // ����������     p188

 GPIOA->MODER&=~(3<<6);        //����˿�  RX
 GPIOA->MODER|=2<<6;           //���ø���ģʽ
 GPIOA->AFR[0]&=~(0xF << 12);     //����˿�
 GPIOA->AFR[0]|=7<<12;           //PA3����ΪUART2     ��AF7����PA3��AFR��
 GPIOA->PUPDR &=~(0x3 << 6);  //����˿�  
 GPIOA->PUPDR|=1<<6;           //������������ �����������յ��µĸ����󴥷� p188

	  // �ȹرմ��ڣ�������
 USART2->CR1 &= ~(1<<13);
	
 //USART2->BRR=((SystemCoreClock/bound)); //����������
USART2->CR1 &= ~(1<<12);  // 8λ����ģʽ��Ĭ�ϣ�
 USART2->CR2 &= ~(3<<12);  // 1λֹͣλ��Ĭ�ϣ�
 USART2->CR1 &= ~(1<<10);  // ��У�飨Ĭ��
	
USART2->BRR = APB1_CLK/bound;
 	
 USART2->CR1|=1<<2;          //���ڽ���ʹ��
 USART2->CR1|=1<<3;          //���ڷ���ʹ��
 USART2->CR1|=1<<5;          //ʹ�ܴ����жϴ���ʹ������жϷ������


	// 8. �����ж����ȼ�
 NVIC_SetPriority(USART2_IRQn, 1);
 NVIC_EnableIRQ(USART2_IRQn);
	
	 USART2->CR1|=1<<13;          //ʹ��USART2 �����жϣ�����USART���ж�ʹ��
}
//----------------PA10----TX  PA11---RX-------------------------------
void USART3_Init(u32 bound)
{ 
   RCC->APB1ENR|=1<<18;            //ʹ��USART3ʱ�� p139
   RCC->AHB1ENR|=1<<1;            //ʹ��PORTBʱ��
   
 GPIOB->MODER&=~(3<<20);        //����˿�  TX
 GPIOB->MODER|=2<<20;           //�������ģʽ p187
 GPIOB->AFR[1]&=~(0xF << 8);     //����˿�     p191
 GPIOB->AFR[1]|=7<<8;           //PB10����ΪUART3   ��AF7����PB10��AFR��
 GPIOB->PUPDR &=~(0x3 << 20);  //����˿�  
 GPIOB->OTYPER &= ~(1<<10);    // �������
 GPIOB->OSPEEDR |= (3<<20);    // ����
 GPIOB->PUPDR &= ~(3<<20);     // ����������     p188

 GPIOB->MODER&=~(3<<22);        //����˿�  RX
 GPIOB->MODER|=2<<22;           //���ø���ģʽ
 GPIOB->AFR[1]&=~(0xF << 12);     //����˿�
 GPIOB->AFR[1]|=7<<12;           //PB11����ΪUART3     ��AF7����PB11��AFR��
 GPIOB->PUPDR &=~(0x3 << 22);  //����˿�  
 GPIOB->PUPDR|=1<<22;           //������������ �����������յ��µĸ����󴥷� p188

 // �ȹرմ��ڣ�������
 USART3->CR1 &= ~(1<<13);
	
 //USART3->BRR=((SystemCoreClock/bound)); //����������
USART3->CR1 &= ~(1<<12);  // 8λ����ģʽ��Ĭ�ϣ�
 USART3->CR2 &= ~(3<<12);  // 1λֹͣλ��Ĭ�ϣ�
 USART3->CR1 &= ~(1<<10);  // ��У�飨Ĭ��
	
USART3->BRR = APB1_CLK/bound;
 	
 USART3->CR1|=1<<2;          //���ڽ���ʹ��
 USART3->CR1|=1<<3;          //���ڷ���ʹ��
 USART3->CR1|=1<<5;          //ʹ�ܴ����жϴ���ʹ������жϷ������


	// 8. �����ж����ȼ�
 NVIC_SetPriority(USART3_IRQn, 1);
 NVIC_EnableIRQ(USART3_IRQn);
	
	 USART3->CR1|=1<<13;          //ʹ��USART3 �����жϣ�����USART���ж�ʹ��
}
   
//----------------USART2�жϷ������------------------------------------------
void USART2_IRQHandler(void)
 {
    static uint8_t recv_buf[16] = {0};
    static uint8_t buf_idx = 0;
    uint8_t recv_data;
    
    if(USART2->SR & (1<<5))
     { // �����жϱ�־
        recv_data = USART2->DR;
	    uart2_received = 1;   // ֻ�����
        if(recv_data == '\n')
        {
          if(strcmp((char*)recv_buf, "stop") == 0)
           {
                // ��⵽ stop
                stop_flag = 1;        // ֹͣ��־λ
                error_update = 0;     // ƫ�����
            } 
            
            else if(strcmp((char*)recv_buf, "turn") == 0)
           {
                
                turn_flag = 1;       
                 error_update = 0;     
            }

              else
		         {       
                    line_error = atoi((char*)recv_buf);// �ַ���ת����
                    error_update = 1;                             // ���ƫ��ֵ���� 
                 }
                    buf_idx = 0;                        
                   memset(recv_buf, 0, sizeof(recv_buf));
        }
        else
        {
            recv_buf[buf_idx % 16] = recv_data;
            buf_idx++;
        }
    }
}

//----------------UART4�жϷ������?------------------------------------------
void UART4_IRQHandler(void)                 
{
  u8 recv_data;
  if(UART4->SR&(1<<5))          
  {
	recv_data=UART4->DR;              
    if(recv_data == 'L')
        {
            left_flag = 1;
 
        }
        else if(recv_data == 'R')
        {
            right_flag = 1;

        }
  }
}


void USART3_SendByte(u8 byte)
{
    USART_SendData(USART3, byte);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void USART3_SendString(char *str)
{
    while(*str)
    {
        USART3_SendByte(*str);
        str++;
    }
}

