#ifndef __DOG_H
#define __DOG_H
#include "sys.h"
#include <math.h>
#include "timer.h"
#include "pwm.h"

#define Pi 3.1415926//¦ÐµÄÈ¡Öµ 
extern u8 PY; //Ì§½Å±¶Êý
extern u8 HIGH; //Õ¾Á¢ÒÆ¶¯Ê±»úÌåµÄ¸ß¶È

extern u8 XH[8];//ÑÓÊ±ÆôÍ££
extern u8 datatemp[8];

void gait(int reverse);
void Yuan_Print(void);
void coordinate(u8 num,float x1,float y1);
void TB_Print(int ii);
// ³õÊ¼×ø±ê£¬È«¾Ö±äÁ¿
extern double L_current_x;
extern double L_current_y;
extern int count;
int LUJIN_X(u8 t);
int LUJIN_Y(u8 t);
int LUJIN_X1(u8 t);
int LUJIN_Y1(u8 t);
int LUJIN_X2(u8 t);
int LUJIN_Y2(u8 t);
int LUJIN_X3(u8 t);
int LUJIN_Y3(u8 t);
int LUJIN_X4(u8 t);
int LUJIN_Y4(u8 t);
void WZ(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
 
#endif
