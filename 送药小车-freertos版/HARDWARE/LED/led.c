#include "led.h"
#include "stm32f4xx.h"



//LED灯 PF9  PF10
//输出模式，推挽

void LED_Init(void)
{
	RCC->AHB1ENR|=1<<5;                       //外设gpio时钟一般挂在APB1上 （不同于103）
                        //第五位置1即IO 端口 E 时钟使能（见手册）
  GPIOF->MODER &=~(0x3 << 18);          //代码中 * 号的核心作用是 “通过指针访问其指向的寄存器地址中的数据
                                        //清楚最终效果：仅目标位被清 0，其他位保持不变。
  GPIOF->MODER |=(0x1 << 18);          //清楚完再配置，01为通用输出模式，见手册
	
  GPIOF->OTYPER &=~(0x1 << 9);           //因为推挽配置目标为已经是0，所以不需要额外配置
//-------------------------	
//  PF10	
	GPIOF->MODER &=~(0x3 << 20);
	
  GPIOF->MODER |=(0x1 << 20);
	
  GPIOF->OTYPER &=~(0x1 << 10);
	
	//GPIOF->ODR|=1<<9;      //控制输出高电平
	//GPIOF->ODR|=1<<10;
	
	GPIOF->ODR=~(1<<9);
	GPIOF->ODR=~(1<<10);//输出低
	
	
}


