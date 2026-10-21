#include "Myinclude.h"

u8 mode=0;//模式
u8 form_mode = 0;	//形态模式，0为履带
u8 function_mode = 2;

u8 Sever_Angle[2] = {25,25};

void GO(void);
void S_Execute_Mode(void);
void L_Execute_Mode(void);

int goflag,luzuflag = 1;

void Init_init(){
	delay_init();	    	 //延时函数初始化
	LED_Init();
	TIM2_Init();
	
	//电机
	Moto_GPIO_Init();
	TIM3_Moto_Init(7199,2);
	
	My_NVIC_Init();          // 自定义 NVIC 初始化
	OLED_Init();
	
}
#define LED_PIN  GPIO_Pin_13
#define LED_GPIO_PORT GPIOC
//132mm*49mm
int main(void)
{ 
	Init_init();
	LED_Init();
	GPIO_SetBits(LED_GPIO_PORT,LED_PIN);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	PCA_MG9XX_Init(55,0);//初始化舵机驱动
	coordinate(1,L_current_x,L_current_y);
	coordinate(2,L_current_x,L_current_y);
	coordinate(3,(-1)*L_current_x,L_current_y);
	coordinate(4,(-1)*L_current_x,L_current_y);

	delay_ms(200);//延时1s 
	TIM4_Int_Init(200-1,7200-1);//20000us	
	uart_init(115200);	 //串口初始化为38400
	delay_ms(200);//延时1s
	//OLED_ShowString(0,0,"WIFI...",OLED_6X8);
	OLED_Update();

	while(1)
	{
		GO();
//		Left_Motor_Control(3000);    // 左轮前进，PWM占空比为500
//        Right_Motor_Control(-3000);   // 右轮前进，PWM占空比为500
		OLED_ShowString(0,8,"RxDa:",OLED_6X8);
		OLED_ShowNum(30,8,Rxdata[0],1,OLED_6X8);
		OLED_ShowNum(36,8,Rxdata[1],1,OLED_6X8);
		OLED_ShowNum(42,8,Rxdata[2],1,OLED_6X8);
		OLED_ShowNum(48,8,Rxdata[3],1,OLED_6X8);
		OLED_ShowNum(54,8,Rxdata[4],1,OLED_6X8);
		OLED_ShowString(0,16,"Mode:",OLED_6X8);
		OLED_ShowNum(30,16,function_mode,1,OLED_6X8);
		OLED_ShowNum(42,16,luzuflag,1,OLED_6X8);
		//OLED_ShowNum(36,8,Rxdata[1],1,OLED_6X8);		
		OLED_ShowNum(0,56,count,3,OLED_6X8);
		OLED_ShowNum(64,56,luzuflag,1,OLED_6X8);
		OLED_Update();
	}
}



void GO(){
	// 执行模式对应的操作
	if(form_mode == 0){
		L_Execute_Mode();	//履带
		OLED_ShowString(0,0,"LuDai",OLED_6X8);
	}else{
		S_Execute_Mode();	//四足
		OLED_ShowString(0,0,"SIZhu",OLED_6X8);
	}
	
	pca_set_angle(8,Sever_Angle[1]);	//俯仰角
	pca_set_angle(9,Sever_Angle[0]);	//航向角
}
uint8_t last_mode;
void S_Execute_Mode() {

	if(mode == 1) {  			//前进
        gait(0); // 前进
		if(luzuflag){
		Left_Motor_Control(7000);    
        Right_Motor_Control(7000);   
		}
    } else if(mode == 2) {  	//后退
		if(luzuflag){
		Left_Motor_Control(-7000);    
        Right_Motor_Control(-7000);   
		}
        gait(1); // 后退
    } else if(mode == 4) {  	//左转
        Left_Motor_Control(-7000);    
        Right_Motor_Control(7000); 
    } else if(mode == 3) {  	//右转
        Left_Motor_Control(7000);    
        Right_Motor_Control(-7000); 
    } else if(mode == 0) {  	//停止
		goflag = 4;
		Left_Motor_Control(0);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(0);   // 右轮前进，PWM占空比为500
        coordinate(1,L_current_x,L_current_y);
		coordinate(2,L_current_x,L_current_y);
		coordinate(3,(-1)*L_current_x,L_current_y);
		coordinate(4,(-1)*L_current_x,L_current_y);
    }else if(mode == 5){		//动作模式
		if(function_mode == 1){			//踏步
			TB_Print(0);
		}else if(function_mode == 2){	//圆
			TB_Print(1);
		}else if(function_mode == 3){	//圆
			
			if(last_mode != 3){
				luzuflag = luzuflag?0:1;
				last_mode = function_mode;
			}
		}
		
		
	}
		if(function_mode != 3){last_mode = 0;}
}
	


void L_Execute_Mode() {
	if(function_mode == 1){
		coordinate(1,130,100);
		coordinate(2,130,100);
		coordinate(3,-130,100);
		coordinate(4,-130,100);
		
	}else if(function_mode == 2){
//		pca_set_angle(0,a1);  
//		pca_set_angle(1,180-a2);  
//		pca_set_angle(2,180-a1);  
//		pca_set_angle(3,a2);  
//		pca_set_angle(4,180-a1);  
//		pca_set_angle(5,a2);
//		pca_set_angle(6,a1);  
////		pca_set_angle(7,180-a2);
//		set_let_angle(1,0,0);
//		set_let_angle(2,0,0);
//		set_let_angle(3,0,0);
//		set_let_angle(4,0,0);
		
		coordinate(1,130,140);
		coordinate(2,130,140);
		coordinate(3,-130,140);
		coordinate(4,-130,140);
	}else if(function_mode == 3){
		coordinate(1,100,180);
		coordinate(2,100,180);
		coordinate(3,-100,180);
		coordinate(4,-100,180);
	}
	
    if(mode == 1) {  			//前进
        Left_Motor_Control(7199);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(7199);   // 右轮前进，PWM占空比为500
    } else if(mode == 2) {  	//后退
        Left_Motor_Control(-7199);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(-7199);   // 右轮前进，PWM占空比为500
    } else if(mode == 4) {  	//左转
        Left_Motor_Control(-7199);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(7199);   // 右轮前进，PWM占空比为500
    } else if(mode == 3) {  	//右转
        Left_Motor_Control(7199);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(-7199);   // 右轮前进，PWM占空比为500
    } else if(mode == 0) {  	//停止
        Left_Motor_Control(0);    // 左轮前进，PWM占空比为500
        Right_Motor_Control(0);   // 右轮前进，PWM占空比为500
    }
}



