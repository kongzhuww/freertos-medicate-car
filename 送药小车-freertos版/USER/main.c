#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include  "led.h"
#include  "FreeRTOS.h"
#include "task.h"
#include "pwm.h"
#include "key.h"
#include "motor.h"
#include "line_control.h"
#include "oled.h"
#include "yl_62.h"
//#include "encoder.h"

 void OLED_Show (void*age)
 {
   while(1)
   {   
    if(uart2_received == 1)
        {
            OLED_ShowString(1, 1, "UART2 RX: OK");
            uart2_received = 0;  
               if(error_update == 1)
                     {
                      
                       OLED_ShowSignedNum(2, 2, line_error, 4);
                       error_update = 0;
                      }
               vTaskDelay(10);       
        }
    if(key_count3 ==1)
    {
      key_pressed1 = 1 ;
      key_pressed2 = 0;
      OLED_ShowString(3, 1, "left"); 
    }
          else if(key_count3 ==2)
          {
            key_pressed1 = 0 ;
            key_pressed2 = 1;
              OLED_ShowString(3, 1, "right");
              key_count3 = 0;
          }
    if(key_count4 ==1)
     {
      base_speed = 18 ;
       OLED_ShowString(3, 7, " low");
     }
         else if(key_count4 ==2)
         {
          base_speed = 25;
            OLED_ShowString(3, 7, " mid");
         }
           else if(key_count4 ==3)
         {
          base_speed = 32;
            OLED_ShowString(3, 7, " high");
            key_count4 = 0;
         }
    //vTaskDelay(10);     
   }
 }



int main(void)
{
	UART4_Init(115200);
  USART2_Init(115200); 
	USART3_Init(115200); 
	delay_init(84);
	LED_Init();
	TIM1_PWMA_CH1_Init(84-1, 1000-1);  
  TIM1_PWMB_CH3_Init(84-1, 1000-1);
  KEY_Init();
  KEY_EXTI_Init();
  //TIM2_Init();
	MOTOR_Init();
	OLED_Init();
  YL62_OUT_Init();
  YL62_EXTI_Init(); 
  // Encoder_RIGHT_Init();
  // Encoder_LEFT_Init();


  xTaskCreate ( (TaskFunction_t) OLED_Show,  "OLED_Show",128, NULL,3,NULL);
  //xTaskCreate ( (TaskFunction_t) task_2,  "task_2",128, NULL,2,NULL);
              
  xTaskCreate((TaskFunction_t)task_go_trace, "task_go_trace",  1024, NULL, 4, NULL); 
  xTaskCreate((TaskFunction_t)task_back_trace, "task_back_trace",  1024, NULL, 4, NULL);

  vTaskStartScheduler();

}





