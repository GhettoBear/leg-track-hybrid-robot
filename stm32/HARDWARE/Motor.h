#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "stm32f10x.h"

// PWM周期和频率设置（根据具体电机和需求进行调整）
#define PWM_PERIOD 1000          // PWM周期
#define PWM_FREQUENCY 1000       // PWM频率（Hz）


// 函数声明
void Moto_GPIO_Init(void);                      // 初始化GPIO引脚
void TIM3_Moto_Init(u16 arr,u16 psc);
void Left_Motor_Control(int pwm_duty);     // 控制左轮电机转动：正值正转，负值反转
void Right_Motor_Control(int pwm_duty);    // 控制右轮电机转动：正值正转，负值反转
void stop(void);                           // 停止电机


#endif // MOTOR_CONTROL_H

