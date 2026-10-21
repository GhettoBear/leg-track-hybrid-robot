#include "stm32f10x.h"

// LED 连接引脚，假设连接在 PA5
#define LED_PIN  GPIO_Pin_13
#define LED_GPIO_PORT GPIOC
#define LED_GPIO_CLK RCC_APB2Periph_GPIOC


// 定时器和 LED 控制变量
volatile uint32_t led_counter = 0;  // 用于计算 LED 闪烁的时间（单位：ms）
volatile uint8_t led_state = 0;     // LED 当前状态（0: 关，1: 开）

// 初始化 LED 引脚
void LED_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);  // 启用 GPIOA 时钟

    GPIO_InitStructure.GPIO_Pin = LED_PIN;  // 设置 PA5 引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);  // 初始化 PA5

    GPIO_ResetBits(LED_GPIO_PORT, LED_PIN);  // 默认关闭 LED
}

// 更新 LED 状态（在每次 5ms 的中断周期内调用）
void Led_Update(void) {
    if (led_counter > 0) {
        led_counter--;  // 每次调用时，减少计数器
    }

    // 如果计数器为 0，关闭 LED
    if (led_counter == 0 && led_state == 1) {
        GPIO_ResetBits(LED_GPIO_PORT, LED_PIN);  // 关闭 LED
        led_state = 0;  // 更新 LED 状态为关闭
    }
}

// LED 控制函数：让 LED 点亮指定的时间（毫秒）
void Led_ms(uint32_t time_ms) {
    led_counter = time_ms / 5;  // 根据 5ms 中断更新的频率计算总的中断次数
    GPIO_SetBits(LED_GPIO_PORT, LED_PIN);  // 点亮 LED
    led_state = 1;  // LED 打开
}

// 定时器中断更新函数
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志

        // 每 5ms 调用一次 Led_Update 来检查 LED 是否需要关闭
        Led_Update();
    }
}

// 初始化定时器
void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // 启用定时器 TIM2 时钟

    TIM_TimeBaseStructure.TIM_Period = 199;  // 计数周期 (200 * 5ms = 1000ms, 即 1s)
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;  // 设置预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  // 初始化定时器

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);  // 使能定时器更新中断
    TIM_Cmd(TIM2, ENABLE);  // 启动定时器
}

// 自定义 NVIC 初始化函数
void My_NVIC_Init(void) {
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  // TIM2 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 使能中断
    NVIC_Init(&NVIC_InitStructure);
}


/*
int main(void) {
    // 初始化 LED 引脚
    LED_Init();

    // 初始化定时器
    TIM2_Init();

    // 初始化 NVIC
    My_NVIC_Init();

    while (1) {
        // 主循环中可以根据需要调用 Led_ms 函数来控制 LED
        // 比如，控制 LED 点亮 500ms
        Led_ms(500);  // 点亮 LED 500 毫秒
        // 等待 LED 熄灭
        while (led_state == 1);  // 等待 LED 熄灭
    }
}
*/
