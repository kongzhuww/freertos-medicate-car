#include "key.h"
#include "stm32f4xx.h"
#include "delay.h"
#include "usart.h"
#include  "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "yl_62.h"
#include "encoder.h"
#include "oled.h"
#include "line_control.h"

volatile uint8_t count_begin_flag = 0;
volatile int count = 0;
volatile uint8_t key_count3 = 0;    // 记录按键3按下次数
volatile uint8_t key_count4 = 0;    // 记录按键4按下次数


volatile uint8_t key_pressed1 = 0;
volatile uint8_t key_pressed2 = 0;
volatile uint8_t key_pressed3 = 0;
volatile int16_t encoder_val=0;
//volatile uint8_t turn_flag = 0;


////KEY1--PE3---------------KEY2--PE4-----------KEY3--PE5-------------

void KEY_Init(void)
{ 
    RCC->AHB1ENR|=1<<4;                       //外设gpioe时钟一般挂在APB1上 （不同于103）
                                         //第五位置1即IO 端口 E 时钟使能（见手册）
    GPIOE->MODER &=~(0x3 << 6|0x3 << 8|0x3 << 10);          //清楚最终效果：仅目标位被清 0，其他位保持不变。
                                        
    GPIOE->MODER |=(0x0 << 6|0x0 << 8|0x0 << 10);          //清楚完再配置，00为输入模式，见手册

    GPIOE->PUPDR &=~(0x3 << 6|0x3 << 8|0x3 << 10);          //清除目标位

    GPIOE->PUPDR |=(0x1 << 6|0x1<< 8|0x1 << 10);          //上拉
    
}

void TIM2_Init(void)
{
    RCC->APB1ENR|=1<<0;      //开启外设时钟使能寄存器  p141
      
   TIM2->PSC=8399;          //预分频器 84M/8400=10KHz  P383

   TIM2->ARR=9999;         //自动重装值 10KHz/10000=1Hz  P383
   
   TIM2->CR1 |= (1<<7);    // 自动重装载预装载使能   P366

   TIM2->EGR |= 1 << 0; //重新初始化计数器并生成一个寄存器更新事件。p374
   
   TIM2->CR1|=1<<0;      //使能计数器，只有事先通过软件将 CEN 位置 1，才可以使用外部时钟

   TIM2->DIER|=1<<0;     //允许更新中断  p378

   NVIC_SetPriority(TIM2_IRQn, 3);

   NVIC_EnableIRQ(TIM2_IRQn);

   // 8. 开启定时器
    TIM2->CR1 |= 1 << 0;
}


void KEY_EXTI_Init(void)
{
    RCC->APB2ENR|=1<<14;     //开启系统配置控制器时钟使能 p144

    // PE3 -> EXTI3
    SYSCFG->EXTICR[0] &= ~(0xF << 12);    //p196
                                          //位偏移 = (EXTI线号 % 4) × 4（线号求余再乘4）
    SYSCFG->EXTICR[0] |=  (0x4 << 12);    //选择PE口

    // PE4 -> EXTI4
    SYSCFG->EXTICR[1] &= ~(0xF << 0);
    SYSCFG->EXTICR[1] |=  (0x4 << 0);    

    // PE5 -> EXTI5
    SYSCFG->EXTICR[1] &= ~(0xF << 4);
    SYSCFG->EXTICR[1] |=  (0x4 << 4);   

    EXTI->IMR |= (1 << 3 | 1 << 4 | 1 << 5);   //开放PE3、PE4、PE5的中断请求   p244

    EXTI->FTSR |= (1 << 3 | 1 << 4 | 1 << 5);   //配置为下降沿触发   p245

    NVIC_SetPriority(EXTI3_IRQn, 6);
    NVIC_EnableIRQ(EXTI3_IRQn);

    NVIC_SetPriority(EXTI4_IRQn, 6);
    NVIC_EnableIRQ(EXTI4_IRQn);

    NVIC_SetPriority(EXTI9_5_IRQn, 6);
    NVIC_EnableIRQ(EXTI9_5_IRQn);   //EXTI5～9：挤在 EXTI9_5 里
}


// PE3 中断
void EXTI3_IRQHandler(void)
{  
   
    if(EXTI->PR & (1 << 3))  // 检查中断标志
    {
       // delay_ms(10);        // 消抖
        if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
        {
          //key_pressed1 = 1;
          key_count3++;    // 记录按键按下次数
          //count_begin_flag = 0;
          //USART3_SendString("hello!\r\n");            
        }        
        EXTI->PR |= 1 << 3;  // 清除中断标志
    }
}

//  void TURN_Choose (void*age)
//  {
//    while(1)
//    { 
//      if(key_count3 ==1)
//      {
//        key_pressed1 = 1 ;
//        key_pressed2 = 0;
//        //USART3_SendString("hello!\r\n");    
//        //OLED_ShowString(3, 1, "left");
//      }
//          else if(key_count3 ==2)
//          {
//             key_pressed1 = 0 ;
//             key_pressed2 = 1;
//             //OLED_ShowString(3, 1, "right");
//             key_count3 = 0; // 执行完清零，准备下一轮
//          }
//       vTaskDelay(pdMS_TO_TICKS(10)); // 10ms更新一次 
//    }    
//  }

// PE4 中断
void EXTI4_IRQHandler(void)
{  
   
    if(EXTI->PR & (1 << 4))  // 检查中断标志
    {
        //delay_ms(10);        // 消抖
        if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
        {
         // key_pressed2 = 1;
          key_count4++;    // 记录按键按下次数         
        }        
        EXTI->PR |= 1 << 4;  // 清除中断标志
    }
}

// void SPEED_Choose (void*age)
//  {
//    while(1)
//    { 
//      if(key_count4 ==1)
//      {
//        base_speed = 18 ;
//       //OLED_ShowString(3, 7, " low");
//      }
//          else if(key_count4 ==2)
//          {
//             base_speed = 25;
//             //OLED_ShowString(3, 7, " mid");
//          }
//            else if(key_count4 ==3)
//          {
//             base_speed = 32;
//             //OLED_ShowString(3, 7, " high");
//             key_count4 = 0; // 执行完清零，准备下一轮
//          }
//      vTaskDelay(10); 
//    }    
//  }



void EXTI9_5_IRQHandler(void)
{  
   
    if(EXTI->PR & (1 << 5))  // 检查中断标志
    {
        //delay_ms(10);        // 消抖
        if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5))
        {
          key_pressed3 = 1;                   
        }        
        EXTI->PR |= 1 << 5;  // 清除中断标志
    }
}

// void START_Key(void*age)
// {
//   while(1)
//   { 
//     if(key_pressed3 == 1)
//     {
//       count_begin_flag = 1; // 开始计数
//       key_pressed3 = 0;     // 重置按键状态，准备下一次触发
//     }
//     vTaskDelay(10); 
//   }    
// }


//  void TIM2_IRQHandler(void)
// {

//     encoder_val = Encoder_GetCount();  // 直接读取全局变量
//     if(TIM2->SR & (1 << 0))// 检查更新中断标志位同usart
//     {
//       if (encoder_val >= 4650)  // 如果计数开始标志位被设置
//       {
//         turn_flag = 1;        
//         Encoder_Reset(); // 直接调用重置函数，确保计数器被清零
//        }
//       TIM2->SR &= ~(1 << 0);  // 必须清除更新中断标志位，否则会重复触发中断
//     }
// }



