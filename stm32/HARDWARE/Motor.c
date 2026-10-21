#include "stm32f10x.h"

/*
左轮方向控制引脚（B11 和 B10）：
B11：左轮前进控制引脚（LEFT_FORWARD_PIN）
B10：左轮后退控制引脚（LEFT_BACKWARD_PIN）
右轮方向控制引脚（A7 和 A6）：
A7：右轮前进控制引脚（RIGHT_FORWARD_PIN）
A6：右轮后退控制引脚（RIGHT_BACKWARD_PIN）
PWM 输出引脚：
B0：左轮 PWM 控制引脚（PWM_LEFT_PIN）
B1：右轮 PWM 控制引脚（PWM_RIGHT_PIN）
*/


void Moto_GPIO_Init(void) {
    // 打开 GPIOB 和 GPIOA 的时钟，使能 GPIOB 和 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // 配置左轮方向引脚 (B11, B10) 为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10;  // 左轮方向控制引脚 B11, B10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 输出速度设置为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure); // 初始化 GPIOB 引脚

    // 配置右轮方向引脚 (A7, A6) 为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;  // 右轮方向控制引脚 A7, A6
    GPIO_Init(GPIOA, &GPIO_InitStructure); // 初始化 GPIOA 引脚

    // 配置 PWM 引脚 (B0, B1) 为复用功能输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; // 选择 B0 和 B1 引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 输出速度设置为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure); // 初始化 GPIOB 引脚
}

void TIM3_Moto_Init(u16 arr,u16 psc) //初始化pwm输出引脚
{		 	

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;                             

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);   //使能定时器3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);  //使能GPIOB的时钟

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;          //复用输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;                //PB1
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);

	TIM_TimeBaseInitStruct.TIM_Period = arr;              //设定计数器自动重装值 
	TIM_TimeBaseInitStruct.TIM_Prescaler  = psc;          //设定预分频器
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;         //设置时钟分割
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);       //初始化定时器

	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;             //选择PWM1模式
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStruct.TIM_Pulse = 0;                            //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;     //设置输出极性
	TIM_OC4Init(TIM3,&TIM_OCInitStruct);                       //初始化输出比较参数
	TIM_OC3Init(TIM3,&TIM_OCInitStruct);                       //初始化输出比较参数

	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);  //CH3预装载使能
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);   //CH4使能预装载寄存器

	TIM_ARRPreloadConfig(TIM3, ENABLE);                //使能TIM3在ARR上的预装载寄存器

	TIM_Cmd(TIM3,ENABLE);                              //使能定时器3
} 

void Right_Motor_Control(int pwm_duty) {
	pwm_duty = (-1)*pwm_duty;
    // 左轮控制（B11 - B10）
    if (pwm_duty > 0) {  // 正转
        GPIO_SetBits(GPIOB, GPIO_Pin_11);   // 设置 B11 为高电平，驱动电机正转
        GPIO_ResetBits(GPIOB, GPIO_Pin_10); // 设置 B10 为低电平，停止反向
    } else if (pwm_duty < 0) {  // 反转
        GPIO_ResetBits(GPIOB, GPIO_Pin_11); // 设置 B11 为低电平，停止正向
        GPIO_SetBits(GPIOB, GPIO_Pin_10);   // 设置 B10 为高电平，驱动电机反转
    } else {  // 停止
        GPIO_ResetBits(GPIOB, GPIO_Pin_11); // 设置 B11 为低电平，停止正向
        GPIO_SetBits(GPIOB, GPIO_Pin_10);   // 设置 B10 为高电平，驱动电机反转
    }

    // 设置 PWM 占空比来控制左轮电机的转速
    if (pwm_duty > 0) {
        TIM_SetCompare4(TIM3, pwm_duty);  // 设置左轮 PWM 占空比
    } else {
        TIM_SetCompare4(TIM3, -pwm_duty); // 设置左轮 PWM 占空比
    }
}

void Left_Motor_Control(int pwm_duty) {
	pwm_duty = (-1)*pwm_duty;
    // 右轮控制（A7 - A6）
    if (pwm_duty < 0) {  // 正转8
        GPIO_SetBits(GPIOA, GPIO_Pin_7);   // 设置 A7 为高电平，驱动电机正转
        GPIO_ResetBits(GPIOA, GPIO_Pin_6); // 设置 A6 为低电平，停止反向
    } else if (pwm_duty > 0) {  // 反转
        GPIO_ResetBits(GPIOA, GPIO_Pin_7); // 设置 A7 为低电平，停止正向
        GPIO_SetBits(GPIOA, GPIO_Pin_6);   // 设置 A6 为高电平，驱动电机反转
    } else {  // 停止
        GPIO_ResetBits(GPIOB, GPIO_Pin_11); // 设置 B11 为低电平，停止正向
        GPIO_SetBits(GPIOB, GPIO_Pin_10);   // 设置 B10 为高电平，驱动电机反转
    }

    // 设置 PWM 占空比来控制右轮电机的转速
    if (pwm_duty > 0) {
        TIM_SetCompare3(TIM3, pwm_duty);  // 设置右轮 PWM 占空比
    } else {
        TIM_SetCompare3(TIM3, -pwm_duty); // 设置右轮 PWM 占空比
    }
}

void stop() {
    Left_Motor_Control(0);  // 停止左轮电机
    Right_Motor_Control(0); // 停止右轮电机
}

/* 示例操作：前进，停止，后退
int main(void) {
    SystemInit();  // 系统初始化
    Moto_GPIO_Init();   // 初始化 GPIO 引脚
    TIM3_Moto_Init();   // 初始化定时器（PWM）

    // 示例操作：前进，停止，后退
    while (1) {
        Left_Motor_Control(500);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(500);   // 右轮前进，PWM占空比为500
        for (int i = 0; i < 1000000; i++); // 延时
        stop();                     // 停止
        for (int i = 0; i < 1000000; i++); // 延时
        Left_Motor_Control(-500);   // 左轮后退，PWM占空比为500
        Right_Motor_Control(-500);  // 右轮后退，PWM占空比为500
        for (int i = 0; i < 1000000; i++); // 延时
        stop();                     // 停止
        for (int i = 0; i < 1000000; i++); // 延时
    }
}
*/

