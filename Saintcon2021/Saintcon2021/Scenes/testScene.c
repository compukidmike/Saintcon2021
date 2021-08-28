/*
 * testScene.c
 *
 * Created: 8/21/2021 7:39:57 PM
 *  Author: Professor Plum
 */ 

#include <math.h>
#include <stdio.h>
#include "main.h"
#include "FrameBuffer.h"

static uint32_t draw_ms=0, blt_ms=0, sys_ms=0, now, tsstep=0;
static int test_x=0, test_y=0, test_dx=1,test_dy=1;

Scene test_scene_loop(bool init) {
	if (back_event) {
		back_event=false;
		return MENU;
	}
	
			//touchWheel = getTouchWheelPostion();
	char lines[4][20];
	static int test_x1=0,test_x2=0,test_y1=0,test_y2=0;
	double angle = (((double)(scroller_position)/256)*2*M_PI) + (1.5*M_PI);
	//LCD_DrawLine(x1,y1,x2,y2,RGB(100,80,100));
	test_x1 = 80*cos(angle)+120;
	test_y1 = 80*sin(angle)+120;
	test_x2 = 100*cos(angle)+120;
	test_y2 = 100*sin(angle)+120;
	//LCD_DrawLine(x1,y1,x2,y2,0xFFFF);
	
	uint16_t c = test_dy>0?RGB(20,20,200):RGB(200,20,20);
	snprintf(lines[0], 20, "draw:%3lums", draw_ms);
	snprintf(lines[1], 20, "blt: %3lums", blt_ms);
	snprintf(lines[2], 20, "sys: %3lums", sys_ms);
	snprintf(lines[3], 20, "%3lufps", 1000/(sys_ms+blt_ms+draw_ms));
	
	
	now = millis();
	sys_ms = now - tsstep;
	tsstep = now;
	canvas_clearScreen(c);
	canvas_drawImage_FromFlash(test_x, test_y, 160, 80, 0x7f8000);
	canvas_drawText(80, 85, lines[0], RGB(255,255,255));
	canvas_drawText(80, 105, lines[1], RGB(255,255,255));
	canvas_drawText(80, 125, lines[2], RGB(255,255,255));
	canvas_drawText(90, 145, lines[3], RGB(255,255,255));
	//canvas_fillRect(80,80,40,40,c);
	canvas_drawLine(test_x1, test_y1, test_x2, test_y2, scroller_status?0xFFFF:0);
	now = millis();
	draw_ms = now - tsstep;
	tsstep = now;
	canvas_blt();
	now = millis();
	blt_ms = now - tsstep;
	tsstep = now;
	
	test_x+=test_dx; test_y+=test_dy;
	if ((test_x<=0) || test_x>=80) test_dx*=-1;
	if ((test_y<=0) || test_y>=160) test_dy*=-1;
	
	return TEST;
}