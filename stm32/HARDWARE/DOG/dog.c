#include "dog.h"
#include "pca9685.h"
#include "myiic.h"
#include "stm32f10x.h"                  // Device header
#include "Myinclude.h"                  // Device header



float l1 = 66.0;  // 第一段臂长
float l2 = 140.0;  // 第二段臂长

//姿态解算（逆运动）
void coordinate(u8 num,float x1,float y1)
{
	if(num == 3||num == 4){
		x1 = -x1;
	}
	
	float shank1, fai1, ham1;
	// 反转 x1
	

	shank1 = acos((x1 * x1 + y1 * y1 - l1 * l1 - l2 * l2) / (-2 * l1 * l2));
	fai1 = acos((l1 * l1 + x1 * x1 + y1 * y1 - l2 * l2) / (2 * l1 * sqrt(x1 * x1 + y1 * y1)));
	if (x1 > 0) {
		ham1 = fabs(atan(y1 / x1)) - fai1;
	}
	else if (x1 < 0) {
		ham1 = Pi - fabs(atan(y1 / x1)) - fai1;
	}
	else {
		ham1 = Pi - 1.5707 - fai1;  // Pi/2 ~ 1.5707
	}

	// 将角度转换为度数
	shank1 = 180 * shank1 / Pi;
	ham1 = 180 * ham1 / Pi;
	shank1 = 180 - shank1;
	set_let_angle(num,ham1,shank1);
}

#define M_PI 3.1415926
// 步态参数，全局变量
double Myperiod = 0.2;             // 步态周期（秒）
int Mysubdivisions = 10;           // 步态细分度（每个周期分成多少小步伐）
double height = 30;             // 步态的最大抬腿高度（米）
double step__length = 90.0f;      // 每步水平位移（米）

// 初始坐标，全局变量
double L_current_x = 100.0;   // 当前的左腿水平坐标
double L_current_y = 150.0;  // 当前的左腿垂直坐标


// 步态函数，模拟机器人交替行走
void gait(int reverse) {
	double R_current_x = L_current_x;   // 当前的右腿水平坐标
	double R_current_y = L_current_y;  // 当前的右腿垂直坐标     

    double period = Myperiod / 2;
    int subdivisions = Mysubdivisions / 2;
    double dt = period / subdivisions;
    double step_length = step__length / 2; // 确保步长是每半周期的大小

    // 打印步态的每个阶段，交替显示两条腿的坐标
    for (int i = 0; i < subdivisions; i++) {
        double t = i * dt; // 当前时间点
        double x_left = 0.0, x_right_current = 0.0;
        double y_left = 0.0, y_right = 0.0;

        if (reverse) {
            // 反向行走
            x_left = step_length - 2 * step_length * t / period;  // 左腿反向走
            y_left = height * sin(M_PI * t / period);  // 左腿跨步，y 按照 sin 正半周期变化
            x_right_current = -step_length + 2 * step_length * t / period; // 右腿反向走
            y_right = 0.0; // 右腿退步，y 永远为 0
        }
        else {
            // 正向行走
            x_left = -step_length + 2 * step_length * t / period;  // 左腿前进
            y_left = height * sin(M_PI * t / period);  // 左腿跨步，y 按照 sin 正半周期变化
            x_right_current = step_length - 2 * step_length * t / period; // 右腿前进
            y_right = 0.0; // 右腿退步，y 永远为 0
        }

        coordinate(1, L_current_x + x_left, L_current_y - y_left);//
		coordinate(4, (-1)*L_current_x + x_left, L_current_y - y_left);
        coordinate(2, R_current_x + x_right_current, R_current_y - y_right);
		coordinate(3, (-1)*R_current_x + x_right_current, R_current_y - y_right);//
		//delay_ms((Myperiod*1000)/Mysubdivisions);
		delay_ms(20);
    }

    // 打印步态的每个阶段，交替显示两条腿的坐标
    for (int i = 0; i < subdivisions; i++) {
        double t = i * dt; // 当前时间点
        double x_left = 0.0, x_right_current = 0.0;
        double y_left = 0.0, y_right = 0.0;  // 右腿跨步，左腿退步

        if (reverse) {
            // 反向行走
            x_right_current = step_length - 2 * step_length * t / period; // 右腿反向走
            y_right = height * sin(M_PI * t / period);  // 右腿跨步，y 按照 sin 正半周期变化
            x_left = -step_length + 2 * step_length * t / period;  // 左腿反向走
            y_left = 0.0; // 左腿退步，y 永远为 0
        }
        else {
            // 正向行走
            x_right_current = -step_length + 2 * step_length * t / period; // 右腿前进
            y_right = height * sin(M_PI * t / period);  // 右腿跨步，y 按照 sin 正半周期变化
            x_left = step_length - 2 * step_length * t / period;  // 左腿前进
            y_left = 0.0; // 左腿退步，y 永远为 0
        }

        coordinate(1, L_current_x + x_left, L_current_y - y_left);
		coordinate(4, (-1)*L_current_x + x_left, L_current_y - y_left);
        coordinate(2, R_current_x + x_right_current, R_current_y - y_right);
		coordinate(3,(-1)*R_current_x + x_right_current, R_current_y - y_right);
		//delay_ms((Myperiod*1000)/Mysubdivisions);
		delay_ms(20);
    }
}



/***********************波动代码***********************************/

#define NUM_POINTS 4  // 坐标数量

// 定义坐标结构体
typedef struct {
    double x;
    double y;
} Point;


// 全局变量定义
Point points[NUM_POINTS]= {
    {100, 150},  // Point 1
    {100, 150},  // Point 2
    {-100, 150},  // Point 3
    {-100, 150}   // Point 4
};
double amplitude = 30.0;        // 波动幅度
int steps = 6;                // 波动细分数量（每个周期的步数）
double delay = 3;            // 滞后量
int stepCount = 0;             // 当前步数


// 更新并输出坐标
void Yuan_Print() {
    stepCount++;  // 每次调用，步数加1
    for (int i = 0; i < NUM_POINTS; ++i) {
        // 根据滞后量和当前的步数，计算每个坐标的y波动
        int adjustedStep = stepCount + i * delay;  // 根据滞后量调整步数
        int oscillation = adjustedStep % (steps * 2);  // 计算周期内的步进（0到2*steps之间）

        // 根据oscillation的值决定y的波动
        if (oscillation < steps) {
            // 向上波动（0到steps之间）
            points[i].y = amplitude * (oscillation / (double)steps);
        }
        else {
            // 向下波动（steps到2*steps之间）
            points[i].y = amplitude * ((steps * 2 - oscillation) / (double)steps);
        }

        // 输出坐标
        //printf("Point %d: (%.2f, %.2f)\n", i + 1, points[i].x, points[i].y);
		if(i == 3){i=4;}
		if(i == 4){i=3;}
		coordinate(i + 1,((i == 2||i == 3)?((-1)*L_current_x):L_current_x),points[i].y + L_current_y);
		OLED_ShowFloatNum(60,8,points[0].y + L_current_y,3,1,OLED_6X8);
		OLED_ShowFloatNum(60,16,points[1].y + L_current_y,3,1,OLED_6X8);
		OLED_ShowFloatNum(60,24,points[2].y + L_current_y,3,1,OLED_6X8);
		OLED_ShowFloatNum(60,32,points[3].y + L_current_y,3,1,OLED_6X8);
		OLED_Update();
		//delay_ms(10);
    }
}
//高低起伏
u8 nn;
double TB_amplitude = 35
	;        // 踏步幅度
int TB_steps = 1
	;                // 踏步细分数量（每个周期的步数）
double TB_delay = 1.0;            // 滞后量
int TB_stepCount = 0;             // 当前步数
float TB_y;

void TB_Print(int ii) {
    TB_stepCount++;  // 每次调用，步数加1
    for (int i = 0; i < 1; ++i) {
        // 根据滞后量和当前的步数，计算每个坐标的y波动
        int adjustedStep = TB_stepCount + i * TB_delay;  // 根据滞后量调整步数
        int oscillation = adjustedStep % (TB_steps * 2);  // 计算周期内的步进（0到2*steps之间）

        // 根据oscillation的值决定y的波动
        if (oscillation < TB_steps) {
            // 向上波动（0到steps之间）
            TB_y = TB_amplitude * (oscillation / (double)TB_steps);
        }
        else {
            // 向下波动（steps到2*steps之间）
            TB_y = TB_amplitude * ((TB_steps * 2 - oscillation) / (double)TB_steps);
        }
		int jj = 0;
        // 输出坐标
		if(ii){
        if(nn == 0){
			coordinate(1,L_current_x+jj,L_current_y-TB_y);
			coordinate(4,(-1)*L_current_x-jj,L_current_y-TB_y);
			coordinate(2,L_current_x+jj,L_current_y-TB_y);
			coordinate(3,(-1)*L_current_x-jj,L_current_y-TB_y);
		}else{
			coordinate(2,L_current_x+jj,L_current_y-TB_y);
			coordinate(3,(-1)*L_current_x-jj,L_current_y-TB_y);
			coordinate(1,L_current_x+jj,L_current_y-TB_y);
			coordinate(4,(-1)*L_current_x-jj,L_current_y-TB_y);
		}
		delay_ms(210);
		
		}else{
			if(nn){
				coordinate(1,L_current_x,L_current_y+TB_y);
				coordinate(4,(-1)*L_current_x,L_current_y-TB_y);
				coordinate(2,L_current_x,L_current_y-TB_y);
				coordinate(3,(-1)*L_current_x,L_current_y+TB_y);
			}else{
				coordinate(2,L_current_x,L_current_y+TB_y);
				coordinate(3,(-1)*L_current_x,L_current_y-TB_y);
				coordinate(1,L_current_x,L_current_y-TB_y);
				coordinate(4,(-1)*L_current_x,L_current_y+TB_y);
			}
		
		
		}
    }
	if(TB_y == 0){
		nn = !nn;
	}
}


/*************************************************************/



int count;

//定时器4中断服务程序
void TIM4_IRQHandler(void)   
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		{
			count ++;
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );   
		}
}







