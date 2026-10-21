#include "stm32f10x.h"
#include "sys.h"
#include "usart.h"
#include "Myinclude.h"


// 配置重定向 printf 输出，避免使用 semihosting 方式
#if 1
#pragma import(__use_no_semihosting)  // 禁用 semihosting 支持
// 需要自定义 __FILE 结构体来重定向输出
struct __FILE {
    int handle;
};
FILE __stdout;  // 定义输出流
// 系统退出时的空函数
void _sys_exit(int x) { 
    x = x; 
}
// 重写 fputc 函数，将 printf 输出重定向到串口
int fputc(int ch, FILE *f) {      
    while((USART1->SR & 0X40) == 0);  // 等待发送完成
    USART1->DR = (u8)ch;  // 发送字符
    return ch;
}
#endif 


#if EN_USART1_RX  // 如果启用了串口1接收功能
// 接收缓冲区和状态
char USART_RX_BUF[USART_REC_LEN];  // 接收缓冲区，长度为 USART_REC_LEN
u16 USART_RX_STA = 0;  // 接收状态标志

// 初始化串口1
void uart_init(u32 bound) {
    // GPIO 和 USART 初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 启用 USART1 和 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置 USART1_TX (PA.9)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 初始化 PA.9

    // 配置 USART1_RX (PA.10)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // PA.10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // 浮空输入模式
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 初始化 PA.10

    // 配置 USART1 中断
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  // USART1 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 启用中断
    NVIC_Init(&NVIC_InitStructure);  // 初始化 NVIC

    // 配置 USART1 串口
    USART_InitStructure.USART_BaudRate = bound;  // 设置波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  // 数据位为 8 位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  // 1 个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;  // 无奇偶校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  // 不使用硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  // 发送和接收模式

    USART_Init(USART1, &USART_InitStructure);  // 初始化 USART1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // 开启接收中断
    USART_Cmd(USART1, ENABLE);  // 启用 USART1
}


int sumcout=11;		//一共有几个字节
int dataB[36];		//改字节记得改数组长度
int Rxdata[10];
int data_testB(int data[])
{
	if(data[0]!=0xA5) return 0;
	if(data[1]!=0x11) return 0;
	if(data[sumcout-1]!=0x5A) return 0;
	
	return 1;  	
}

// USART1 中断服务函数
void USART1_IRQHandler(void) {
	static int i = 0;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  // 如果接收到数据
    {
        dataB[i++] = USART_ReceiveData(USART1);
		if(dataB[0]!=0xA5) i=0;						//判断帧头1
		if((i==2)&&(dataB[1]!=0x11)) i=0;			//判断帧头2
		if(i==sumcout)
		{
			if(data_testB(dataB))				//判断帧头帧尾合理性
			{
			/************************************
		                 更新数据
			*************************************/
				for (int i = 0; i < 8; i++) {
					Rxdata[i] = dataB[2 + i];  // 将 dataB[2] 到 dataB[9] 的数据映射到 Rxdata[0] 到 Rxdata[7]
				}
				
				
				form_mode = Rxdata[0];
				mode = Rxdata[1];
				function_mode = Rxdata[2]; 
				Sever_Angle[0] = 180 - Rxdata[3];
				Sever_Angle[1] = 180 - Rxdata[4];
			}
			i = 0;
		}
    }

}

/*
data[0]:模式
data[1]:指令		data[1]是5的时候说明是动作模式
data[2]:动作		data[2]是0的时候说明是遥控模式 
data[3]:俯仰角
data[4]:航向角
*/

#endif  // EN_USART1_RX

