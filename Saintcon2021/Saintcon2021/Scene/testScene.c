/*
 * testScene.c
 *
 * Created: 8/21/2021 7:39:57 PM
 *  Author: Professor Plum
 */ 

#include <math.h>
#include "main.h"
#include "FrameBuffer.h"

int test_x=25,test_y=0,test_dx=1,test_dy=1;

Scene test_scene_loop(bool init) {
	if (back_event)
		return MENU;
	//touchWheel = getTouchWheelPostion();
	static int x1=0,x2=0,y1=0,y2=0;
	double angle = (((double)(scroller_position)/256)*2*M_PI) + (1.5*M_PI);
	//LCD_DrawLine(x1,y1,x2,y2,RGB(100,80,100));
	x1 = 80*cos(angle)+120;
	y1 = 80*sin(angle)+120;
	x2 = 100*cos(angle)+120;
	y2 = 100*sin(angle)+120;
	//LCD_DrawLine(x1,y1,x2,y2,0xFFFF);
	
	uint16_t c = test_dy>0?RGB(20,20,200):RGB(200,20,20);
	canvas_clearScreen(c);
	canvas_drawImage_FromFlash(test_x, test_y, 160, 80, (uint16_t*)0x7f8000);
	canvas_drawText(80, 125, "Bird", RGB(255,255,255));
	//canvas_fillRect(80,80,40,40,c);
	canvas_drawLine(x1, y1, x2, y2, scroller_status?0xFFFF:0);
	canvas_blt();
	
	test_x+=test_dx; test_y+=test_dy;
	if ((test_x<=0) || test_x>=80) test_dx*=-1;
	if ((test_y<=0) || test_y>=160) test_dy*=-1;
	
	return TEST;
}