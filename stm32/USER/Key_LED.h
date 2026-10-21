#ifndef __LED_CONTROL_H
#define __LED_CONTROL_H

#include "stm32f10x.h"  // 包含 STM32 标准外设库的头文件


// 函数声明
void LED_Init(void);               // 初始化 LED 引脚
void Led_Update(void);             // 更新 LED 状态
void Led_ms(uint32_t time_ms);    // 控制 LED 点亮指定的时间（毫秒）
void TIM2_IRQHandler(void);       // TIM2 中断处理函数
void TIM2_Init(void);             // 初始化定时器 TIM2
void My_NVIC_Init(void);          // 自定义 NVIC 初始化

#endif /* __LED_CONTROL_H */
