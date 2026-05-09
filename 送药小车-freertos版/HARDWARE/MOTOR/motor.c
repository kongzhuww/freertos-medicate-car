#include "motor.h"
#include "stm32f4xx.h"
#include "stdlib.h"
#include "delay.h"
#include "key.h"
#include  "FreeRTOS.h"
#include "task.h"
#include "led.h"



void MOTOR_Init(void)
{
    // 开启 GPIOC, GPIOD 的时钟  p135 (1<<2是GPIOC, 1<<3是GPIOD)  //U代表无符号整形
    RCC->AHB1ENR |= (1U<<2) | (1U<<3);

    // 清零PD12~15的MODER位配置
    GPIOD->MODER &= ~(3U<<24 | 3U<<26 | 3U<<28 | 3U<<30); 

    // 设置PD12~15为通用输出模式 (01)  p187
    GPIOD->MODER |= (1U<<24 | 1U<<26 | 1U<<28 | 1U<<30);  

    // 设置PD12~15为高速输出 (11)   
    GPIOD->OSPEEDR |= (3U<<24 | 3U<<26 | 3U<<28 | 3U<<30); 
  
    // 设置PD12~15为推挽输出模式 (0)
    GPIOD->OTYPER &= ~(1U<<12 | 1U<<13 | 1U<<14 | 1U<<15); 

    // 设置PD12~15为无上下拉 (00)
    GPIOD->PUPDR &= ~(3U<<24 | 3U<<26 | 3U<<28 | 3U<<30);

     // 3. 配置STBY引脚（C13）为推挽输出
	  RCC->APB1ENR |= RCC_APB1ENR_PWREN;   // 使能电源接口时钟
    PWR->CR |= PWR_CR_DBP;                // 使能备份域访问（DBP位）
	
    GPIOC->MODER &= ~(3<<26);         // 清空B11模式
    GPIOC->MODER |= (1<<26);          // C13设为通用输出
    GPIOC->OSPEEDR |= (3<<26);        // 高速输出
    GPIOC->OTYPER &= ~(1<<13);        // 推挽模式
	  GPIOC->PUPDR &= ~(3<<26);         // PC13无上下拉
	
    GPIO_SetBits(GPIOC, GPIO_Pin_13);          // STBY=1（工作）
	
}


void Motor1_Set(int16_t speed) //右轮
{
    speed = (speed > 1000) ? 1000 : ((speed < -1000) ? -1000 : speed);
    if (speed > 0) {
        GPIO_SetBits(GPIOD, GPIO_Pin_12);   // AIN2=1（正转）
        GPIO_ResetBits(GPIOD, GPIO_Pin_13); // AIN1=0
    } else if (speed < 0) {
        GPIO_ResetBits(GPIOD, GPIO_Pin_12); // AIN2=0（反转）
        GPIO_SetBits(GPIOD, GPIO_Pin_13);   // AIN1=1
    } else {
        GPIO_ResetBits(GPIOD, GPIO_Pin_12); // AIN2=0（停止）
        GPIO_ResetBits(GPIOD, GPIO_Pin_13); // AIN1=0
    }
    TIM_SetCompare1(TIM1, abs(speed));  // PWMA PWM占空比
}

// 电机2（左轮）速度设置
void Motor2_Set(int16_t speed) 
{
    speed = (speed > 1000) ? 1000 : ((speed < -1000) ? -1000 : speed);
    if (speed > 0) {
        GPIO_SetBits(GPIOD, GPIO_Pin_15);   // BIN1=1（正转）
        GPIO_ResetBits(GPIOD, GPIO_Pin_14); // BIN2=0
    } else if (speed < 0) {
        GPIO_ResetBits(GPIOD, GPIO_Pin_15); // BIN1=0（反转）
        GPIO_SetBits(GPIOD, GPIO_Pin_14);   // BIN2=1
    } else {
        GPIO_ResetBits(GPIOD, GPIO_Pin_15); // BIN1=0（停止）
        GPIO_ResetBits(GPIOD, GPIO_Pin_14); // BIN2=0
    }
    TIM_SetCompare3(TIM1, abs(speed));  // PWMB PWM占空比
}

void TURN_LEFT(void)
{
    //LED_GREEN_ON();
    Motor1_Set(18);
    Motor2_Set(18);
    vTaskDelay(pdMS_TO_TICKS(725));
   
    Motor1_Set(-20);
    Motor2_Set(20);
    vTaskDelay(pdMS_TO_TICKS(650));
}


void TURN_RIGHT(void)
{
    //LED_GREEN_ON();
    Motor1_Set(18);
    Motor2_Set(18);
    vTaskDelay(pdMS_TO_TICKS(725));
   
    Motor1_Set(20);
    Motor2_Set(-20);
    vTaskDelay(pdMS_TO_TICKS(650));
    
}

void TURN_AROUND(void)
{
    Motor1_Set(18);
    Motor2_Set(-20);
    vTaskDelay(pdMS_TO_TICKS(1130));

    Motor1_Set(0);
    Motor2_Set(0);
    vTaskDelay(pdMS_TO_TICKS(650));
}

void TURN_LEFT_BACK(void)
{
    //LED_GREEN_ON();
    Motor1_Set(18);
    Motor2_Set(18);
    vTaskDelay(pdMS_TO_TICKS(550));
   
    Motor1_Set(-20);
    Motor2_Set(20);
    vTaskDelay(pdMS_TO_TICKS(500));
}


void TURN_RIGHT_BACK(void)
{
    //LED_GREEN_ON();
    Motor1_Set(18);
    Motor2_Set(18);
    vTaskDelay(pdMS_TO_TICKS(750));
   
    Motor1_Set(20);
    Motor2_Set(-20);
    vTaskDelay(pdMS_TO_TICKS(500));
    
}



