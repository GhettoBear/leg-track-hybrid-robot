#include "pca9685.h"
#include "myiic.h"
#include "delay.h"
#include "math.h"
#include "Myinclude.h"

void pca_write(u8 adrr,u8 data)//向PCA写数据,adrrd地址,data数据
{ 
	IIC_Start();
	
	IIC_Send_Byte(pca_adrr);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(adrr);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(data);
	IIC_Wait_Ack();
	
	IIC_Stop();
}

u8 pca_read(u8 adrr)//从PCA读数据
{
	u8 data;
	IIC_Start();
	
	IIC_Send_Byte(pca_adrr);
	IIC_Wait_Ack();
	
	IIC_Send_Byte(adrr);
	IIC_Wait_Ack();
	
	IIC_Start();
	
	IIC_Send_Byte(pca_adrr|0x01);
	IIC_Wait_Ack();
	
	data=IIC_Read_Byte(0);
	IIC_Stop();
	
	return data;
}


void pca_setfreq(float freq)//设置PWM频率
{
		u8 prescale,oldmode,newmode;
		double prescaleval;
		freq *= 0.92; 
		prescaleval = 25000000;
		prescaleval /= 4096;
		prescaleval /= freq;
		prescaleval -= 1;
		prescale =floor(prescaleval + 0.5f);

		oldmode = pca_read(pca_mode1);
	
		newmode = (oldmode&0x7F) | 0x10; // sleep
	
		pca_write(pca_mode1, newmode); // go to sleep
	
		pca_write(pca_pre, prescale); // set the prescaler
	
		pca_write(pca_mode1, oldmode);
		delay_ms(2);
	
		pca_write(pca_mode1, oldmode | 0xa1); 
}

void pca_setpwm(u8 num, u32 on, u32 off)
{
		off=(u32)(102+off*2.28);
		pca_write(LED0_ON_L+4*num,on);
		pca_write(LED0_ON_H+4*num,on>>8);
		pca_write(LED0_OFF_L+4*num,off);
		pca_write(LED0_OFF_H+4*num,off>>8);
}

void Mypca_setpwm(u8 num, u32 on, u32 off)
{
		pca_write(LED0_ON_L+4*num,on);
		pca_write(LED0_ON_H+4*num,on>>8);
		pca_write(LED0_OFF_L+4*num,off);
		pca_write(LED0_OFF_H+4*num,off>>8);
}

// 函数：根据角度设置舵机
// 参数：
//   num  - 舵机号，0~15
//   angle - 目标角度，范围为0.0~180.0（浮动角度）
void pca_set_angle(u8 num, float angle) {
    // 确保目标角度在合理范围内（0.0~180.0）
    if (angle > 180.0f) {
        angle = 180.0f;
    } else if (angle < 0.0f) {
        angle = 0.0f;
    }

    // 计算脉冲宽度（单位：ms），角度范围为0.0°到180.0°
    // 0° 对应 1ms，180° 对应 2ms
    float pulse_width_ms = 0.5f + (angle / 180.0f)*2.0; // 1ms到2ms

    // 计算占空比 (pulse_width / 20ms) 
    float duty_cycle = pulse_width_ms / 20.0f;  // 计算占空比，范围0.05到0.10
	
    // 将占空比转换为 `off` 值
    u32 on = 0;  // `on` 为0表示PWM信号从0开始
    u32 off = (u32)(duty_cycle * 4096);  // 最大值4096，对应100%占空比
	OLED_ShowNum(0,32,off,4,OLED_6X8);
	OLED_Update();
    // 设置舵机的PWM
    Mypca_setpwm(num, on, off);
}

//pca_set_angle(0,45);  
//	pca_set_angle(1,150);  
//	
//	pca_set_angle(2,135);  
//	pca_set_angle(3,30);  
//	
//	pca_set_angle(4,135);  
//	pca_set_angle(5,30);  
//	
//	pca_set_angle(6,45);  
//	pca_set_angle(7,150);


//设置哪条腿的上关节和下关节角度
void set_let_angle(u8 num,float angle1,float angle2){
	if(num == 1){
		angle1 = 90.0 + angle1-20;
		angle2 = 90.0 + angle2-4;
		pca_set_angle(0,angle1);
		pca_set_angle(1,angle2);
	}else if(num == 2){
		angle1 = 90 - angle1;
		angle2 = 90 - angle2;
		pca_set_angle(2,angle1);
		pca_set_angle(3,angle2);
	}else if(num == 3){
		angle1 = 90 - angle1+10;
		angle2 = 90 - angle2-8.5;
		pca_set_angle(4,angle1);
		pca_set_angle(5,angle2);
	}else if(num == 4){
		angle1 = 90.0 + angle1 + 8;
		angle2 = 90.0 + angle2;
		pca_set_angle(6,angle1);
		pca_set_angle(7,angle2);
	}


}

void pca_down(u8 num)
{
		pca_write(LED0_ON_L+4*num,0);
		pca_write(LED0_ON_H+4*num,0>>8);
		pca_write(LED0_OFF_L+4*num,0);
		pca_write(LED0_OFF_H+4*num,0>>8);
}
/*num:舵机PWM输出引脚0~15，on:PWM上升计数值0~4096,off:PWM下降计数值0~4096
一个PWM周期分成4096份，由0开始+1计数，计到on时跳变为高电平，继续计数到off时
跳变为低电平，直到计满4096重新开始。所以当on不等于0时可作延时,当on等于0时，
off/4096的值就是PWM的占空比。*/

/*
	函数作用：初始化舵机驱动板
	参数：1.PWM频率
		  2.初始化舵机角度
*/
void PCA_MG9XX_Init(float hz,u8 angle)
{
	IIC_Init();
	pca_write(pca_mode1,0x0);
	pca_setfreq(hz);//设置PWM频率
	pca_down(0);pca_down(1);pca_down(2);pca_down(3);pca_down(4);pca_down(5);pca_down(6);pca_down(7);
	pca_down(8);pca_down(9);pca_down(10);pca_down(11);pca_down(12);pca_down(13);pca_down(14);pca_down(15);
//	pca_setpwm(0,0,off);pca_setpwm(1,0,off);pca_setpwm(2,0,off);pca_setpwm(3,0,off);
//	pca_setpwm(4,0,off);pca_setpwm(5,0,off);pca_setpwm(6,0,off);pca_setpwm(7,0,off);
//	pca_setpwm(8,0,off);pca_setpwm(9,0,off);pca_setpwm(10,0,off);pca_setpwm(11,0,off);
//	pca_setpwm(12,0,off);pca_setpwm(13,0,off);pca_setpwm(14,0,off);pca_setpwm(15,0,off);
	delay_ms(500);
}
