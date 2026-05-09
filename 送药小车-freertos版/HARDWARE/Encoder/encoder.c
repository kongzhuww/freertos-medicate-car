// #include "stm32f4xx.h"
// #include "encoder.h"
// #include "usart.h"
// #include "yl_62.h"



// volatile int32_t Encoder_Count = 0;


// void Encoder_RIGHT_Init(void)
// {
//     /* PB6/PB7 -> TIM4 CH1/CH2 (AF2) */

//     // 1. 开时钟
//     RCC->AHB1ENR |= 1U << 1;    // GPIOB 时钟使能
//     RCC->APB1ENR |= 1U << 2;    // TIM4 时钟使能

//     // 2. PB6 PB7 复用功能模式
//     GPIOB->MODER &= ~((0x3U << 12) | (0x3U << 14));
//     GPIOB->MODER |=  ((0x2U << 12) | (0x2U << 14));

//     // 3. 上拉（和你第二段代码保持一致）
//     GPIOB->PUPDR &= ~((0x3U << 12) | (0x3U << 14));
//     GPIOB->PUPDR |=  ((0x1U << 12) | (0x1U << 14));

//     // 4. 复用功能 AF2 (TIM4)
//     GPIOB->AFR[0] &= ~((0xFU << 24) | (0xFU << 28));
//     GPIOB->AFR[0] |=  ((0x2U << 24) | (0x2U << 28));

//     // 5. TIM4 编码器模式 3（AB相 4倍频，最稳定）
//     TIM4->CR1 = 0;
//     TIM4->SMCR = 0;
//     TIM4->CCMR1 = 0;
//     TIM4->CCER = 0;
//     TIM4->ARR = 0xFFFFU;
//     TIM4->CNT = 0;
//     TIM4->CCMR1 |= (1U << 0) | (1U << 8);
//     TIM4->SMCR |= 3U;
//     TIM4->CR1 |= 1U;
// }

// void Encoder_LEFT_Init(void)
// {
//     /* PA0/PA1 -> TIM5 CH1/CH2 (AF2) */
//     PWR->CR |= (1U << 8);
//     RTC->TAFCR &= ~(1U << 8);
//     // 1. 开时钟
//     RCC->AHB1ENR |= 1U << 0;    // GPIOA 时钟使能
//     RCC->APB1ENR |= 1U << 3;    // TIM5 时钟使能

//     // 2. PA0 PA1 复用功能模式
//     GPIOA->MODER &= ~((0x3U << 0) | (0x3U << 2));
//     GPIOA->MODER |=  ((0x2U << 0) | (0x2U << 2));

//     // 3. 上拉（提高稳定性，与编码器输出类型匹配）
//     GPIOA->PUPDR &= ~((0x3U << 0) | (0x3U << 2));
//     GPIOA->PUPDR |=  ((0x1U << 0) | (0x1U << 2));

//     // 4. 复用功能 AF2 (TIM5)
//     GPIOA->AFR[0] &= ~((0xFU << 0) | (0xFU << 4));
//     GPIOA->AFR[0] |=  ((0x2U << 0) | (0x2U << 4));

//     // 5. TIM5 编码器模式3（AB相4倍频）
//     TIM5->CR1 = 0;
//     TIM5->SMCR = 0;
//     TIM5->CCMR1 = 0;
//     TIM5->CCER = 0;
//     TIM5->ARR = 0xFFFFU;
//     TIM5->CNT = 0;
    
//     // 设置输入滤波器（滤除噪声，必须）
//     TIM5->CCMR1 |= (1U << 4) | (1U << 12);

//     // CC1S=01, CC2S=01 (通道1/2作为输入)
//     TIM5->CCMR1 |= (1U << 0) | (1U << 8);
    
//     // SMS=011 (编码器模式)
//     TIM5->SMCR |= 3U;
    
//     // 使能计数器
//     TIM5->CR1 |= 1U;
// }


// // 获取编码器当前总计数
// int32_t Encoder_GetCount(void)
// {
//     int32_t count_right= (int16_t)TIM4->CNT; // 右轮计数（可能为负）
//     int32_t count_left= -(int16_t)TIM5->CNT;  // 左轮计数（可能为负）

//     // // return (int32_t)TIM4->CNT;
//     // // return -(int32_t)TIM5->CNT;
//      return (count_right + count_left)/2; // 返回两轮平均计数，减少误差
// }


// void Encoder_Reset(void)
// {
//     TIM4->CNT = 0;
//     TIM5->CNT = 0;
// }


// // void Turn_timeFlag (void)
// // {
   
// //    encoder_val = Encoder_GetCount();  // 直接读取全局变量 

// //    if (encoder_val >= 4650)
// //    {
// //              turn_flag = 1;             
// //               //YL62_flag = 0; 
// //    }
   
// // }

