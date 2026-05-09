 #include "yl_62.h"
 #include "stm32f4xx.h"
 #include "key.h"
 #include "usart.h"
 /******* OUT-----PE0 ********/
void YL62_OUT_Init(void)
{ 
    RCC->AHB1ENR|=1<<5;                       
                                        
    GPIOE->MODER &=~(0x3 << 0);          
                                        
    GPIOE->MODER |= (0x0 << 0);        

    GPIOE->PUPDR &=~(0x3 << 0);          

    GPIOE->PUPDR |=(0x1 << 0);          
   
  }
 


void YL62_EXTI_Init(void)
{
    
    RCC->APB2ENR |= 1 << 14;
    

    SYSCFG->EXTICR[0] &= ~(0xF << 0);
    SYSCFG->EXTICR[0] |=  (0x4 << 0);  
    
  
    EXTI->FTSR |= 1 << 0;
    EXTI->IMR  |= 1 << 0;  
    
  
    NVIC_SetPriority(EXTI0_IRQn, 3);
    NVIC_EnableIRQ(EXTI0_IRQn);
}



volatile uint8_t YL62_flag = 0; 	
volatile uint8_t YL62_state = 0;


uint8_t YL62_Get_State(void)
{
    // PE0是传感器输出引脚，低电平触发，所以取反后返回
    return (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0) == 0) ? 1 : 0;
}	


void EXTI0_IRQHandler(void)
	
{
    
    if(EXTI->PR & (1 << 0))
    {
        if (key_pressed1 || key_pressed2 || key_pressed3 == 1)
        {
           // USART3_SendString("hello!\r\n"); 
           YL62_flag = 1;  
           //count_begin_flag = 1;
          // key_pressed1 = 0;         // 清除按键等待标志，防止重复触发
        }
  
        EXTI->PR |= 1 << 0;     
    }
} 
 



