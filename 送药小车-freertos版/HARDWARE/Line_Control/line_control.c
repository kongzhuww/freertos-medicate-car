# include "line_control.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "motor.h"
#include  "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include "yl_62.h"
#include "oled.h"
#include "key.h"
#include "encoder.h"
#include "usart.h"

// PID参数定义
#define KP 0.2f   // 比例系数
#define KI 0.0f   // 积分系数
#define KD 0.0f   // 微分系数

#define PATH_MAX_LEN 3  //最多记录动作数

volatile int16_t line_error=0;    // 巡线偏差值
volatile uint8_t error_update=0;
volatile int16_t base_speed=0; // 基础速度 
volatile uint8_t back_flag=0; // 返程标志

char path[PATH_MAX_LEN];   // 路径数组：'L'=左  'R'=右
uint8_t path_len = 0;     // 当前记录长度
int back_index = 0;   //记住当前动作


// PID控制器结构体
typedef struct {
    float kp, ki, kd;
    float target;       // 目标值（中线x=160，偏差目标0）
    float error;        // 当前偏差
    float last_error;   // 上一次偏差
    float integral;     // 积分项
    float output;       // 输出值（电机差速）
} PID_TypeDef;

PID_TypeDef line_pid = {KP, KI, KD, 0, 0, 0, 0, 0};

// PID计算函数
float PID_Calc(PID_TypeDef *pid, float error) {
    // 第一步：先定义变量（移到函数内最前面）
    float output = 0.0f; // 提前声明，解决位置错误
    
    // 第二步：执行逻辑
    pid->error = error;

    if(fabs(error) < 5) {
        pid->integral = 0;
    } else {
        pid->integral += pid->error;
        pid->integral = (pid->integral > 50) ? 50 : (pid->integral < -50 ? -50 : pid->integral);
    }
    
    output = pid->kp * pid->error + pid->ki * pid->integral + pid->kd * (pid->error - pid->last_error);
    output = (output > 50) ? 50 : (output < -50 ? -50 : output);

    pid->last_error = pid->error;
    pid->output = output;
    return output;
}

void line_trace(void)
{
   
    float pid_out = 0;
    int16_t motor1_speed = base_speed+5; // 基础速度
    int16_t motor2_speed = base_speed;

pid_out = PID_Calc(&line_pid, line_error);
            // 调整左右电机速度（限幅-1000~1000）
            motor1_speed = base_speed- pid_out;  // 右轮
            motor2_speed = base_speed+ pid_out;  // 左轮
            // 限制速度范围
            motor1_speed = (motor1_speed > 50) ? 50 : ((motor1_speed < -50) ? -50 : motor1_speed);
            motor2_speed = (motor2_speed > 50) ? 50 : ((motor2_speed < -50) ? -50 : motor2_speed);
            
            // 设置电机速度
            Motor1_Set(motor1_speed);
            Motor2_Set(motor2_speed);
}


// 记录动作
void Record_Path(char action)
{
    if(path_len < PATH_MAX_LEN)
    {
        path[path_len++] = action;
    }
}


// 巡线任务
void task_go_trace(void* arg) 
{   
    uint8_t trace_enable = 0;
    static int count = 0;  // 只用1个计数变量

while(1) 
 {
    if(back_flag == 1)
    {
      Motor1_Set(0);
      Motor2_Set(0);

     vTaskDelete(NULL); // 毙死当前任务
     return; // 确保退出循环
    }
    
    YL62_state = YL62_Get_State(); // 实时更新传感器状态
    if(turn_flag ==1)  // 如果暂停巡线
    {
     //trace_enable = 0;        // 暂停巡线 
     if(key_pressed1 || left_flag)  
     {          
        TURN_LEFT();
        turn_flag = 0;
        key_count3 = 0; // 清除按键计数
        Record_Path('L'); // 记录左转动作
     }
       else if(key_pressed2 || right_flag)
         {  
           
           TURN_RIGHT();
           turn_flag = 0;
           key_count3 = 0; // 清除按键计数
           Record_Path('R'); // 记录右转动作
         }
     //trace_enable = 1;        // 恢复巡线
      else { 
        turn_flag = 0; 
        }  

     continue;  // 直接跳过本次巡线循环，不执行下面逻辑
    }
    

    if (YL62_flag)
    {
       // USART3_SendString("hello!\r\n"); 
         YL62_flag = 0; // 清除传感器触发标志
		
         count= 1; // 启动巡线        
    }
    
   // 2. 计数到 200 → 2秒 (10ms延时一次)
    if(count > 0)
    {
        count++;
        if(count >= 200)  // 10*200=2000ms=2秒
        {
            trace_enable = 1;
            count = 0;
            OLED_ShowString(4, 1, "Line Trace OK");
        }
    }

     if (trace_enable)
     {        
        if(error_update) 
        {            
            // 计算PID输出（偏差值转电机差速）
            line_trace(); // 执行巡线控制           
            error_update = 0; // 清除更新标志         
        }   
        else if (stop_flag)
             {
               Motor1_Set(0);
               Motor2_Set(0);
               
			      if(YL62_state == 0) // 如果传感器仍然触发，说明还在障碍物上
                  {
                      TURN_AROUND();
                      stop_flag = 0; // 清除停止标志，准备下一轮
                     //trace_enable = 1;  // 恢复巡线
                     back_flag=1; // 设置返程标志，结束巡线任务
                  }
            }     
    }
  vTaskDelay(pdMS_TO_TICKS(10)); // 10ms更新一次
  }

}


void task_back_trace (void*arg)
 {
     //int i = 0;
    while(1)
    {
        if(back_flag == 1)
        {
            if(turn_flag ==1)
            {
              // 取一个动作（倒序）
                char action = path[ path_len - 1 - back_index ];     
    // 执行这一个动作
                  if(action == 'L')
                     {
                       TURN_RIGHT_BACK();
                     }
                    else if(action == 'R')
                      {
                         TURN_LEFT_BACK();
                      }   
                         // 执行完一个，计数+1，准备下一个
                         back_index++;
                         // 关掉本次标志，等待下一次触发
                         turn_flag = 0;
                         // 所有动作跑完了，重置
                        if(back_index >= path_len)
                            {
                               back_index = 0;
                               path_len = 0;
                          }
            }
        else if (stop_flag)
        {
          Motor1_Set(0);
          Motor2_Set(0);  
        }

        else if(error_update) 
            {            
                // 计算PID输出（偏差值转电机差速）
                line_trace(); // 执行巡线控制           
                error_update = 0; // 清除更新标志         
            }     
       }
       vTaskDelay(pdMS_TO_TICKS(10)); // 10ms更新一次
   }
 }


