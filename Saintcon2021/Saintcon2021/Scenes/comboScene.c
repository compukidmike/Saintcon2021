/*
 * comboScene.c
 *
 * Created: 8/21/2021 9:07:51 PM
 *  Author: Professor Plum
 */ 

#include <math.h>
#include <stdio.h>
#include "main.h"
#include "FrameBuffer.h"
#include "combonums.h"

static int combo_selected;
static int combo_position;
static int combo_stage;
static int combo_combo[2];
static int combo_lastLocation;
static bool combo_scrolling;
static int scroll_dist;

void combo_draw() {
	canvas_clearScreen(0);
    
    //draw numbers
    for (int i=0; i<40; i+=5) {
        float a = ((i*9 - combo_position + 360) % 360) * M_PI / 180.0;
        
		int px = 80*sinf(a) + 106;
		int py = -80*cosf(a) + 106;

        canvas_drawBitmask(px, py, 32, 32, num_bits[i/5], 0xFFFF, a);
        
    }
    
    //draw fingers
    for (int i=0; i<40; ++i) {
		float ang = (i*9 - combo_position) * M_PI / 180.0;
		int p1x = 90*sinf(ang);
		int p1y = -90*cosf(ang);

        int p2x = p1x *1.3f;
		int p2y = p1y *1.3f;
        if ((i%5)==0) {
			p1x*=1.15f;
			p1y*=1.15f;
		}
        else {
			p1x*=1.25f;
			p1y*=1.25f;
		}
        canvas_drawLine(p1x+120, p1y+120, p2x+120, p2y+120, RGB(255,255,255));
    }
    
    //draw center
    canvas_fillCircle(120, 120, 48, RGB(80,80,80));
	float ang = (360 - combo_position) * M_PI / 180.0;
	canvas_drawBitmask(96, 96, 48, 48, saint_bits, RGB(160,160,160), ang);
    
    //draw combo
    char cmb[10];
    switch (combo_stage) {
        case 0:
            snprintf(cmb, 10, "%2d", combo_selected);
            break;
        case 1:
            snprintf(cmb, 10, "%2d-", combo_combo[0]);
            break;
        case 2:
        case 3:
            snprintf(cmb, 10, "%2d-%2d", combo_combo[0], combo_selected);
            break;
        case 4:
        case 5:
            snprintf(cmb, 10, "%2d-%2d-%2d", combo_combo[0], combo_combo[1], combo_selected);
            break;
        default:
            snprintf(cmb, 10, "Invalid");
    }
		
	canvas_drawBitmask(117, 0, 8, 4, arrow, RGB(255,100,100), 0);
	
	//Draw combo (maybe remove this?, although it is nice to have)
	canvas_fillRect(60, 222, 120, 20, RGB(20,20,20));
	canvas_drawText(86, 222, cmb, RGB(120,120,80));
	snprintf(cmb, 10, "s:%d d:%d", combo_stage, scroll_dist);
	canvas_drawText(86, 200, cmb, RGB(120,120,80));
	
    canvas_blt();
}

Scene combo_scene_loop(bool init) {
	if (back_event) {
		back_event=false;
		return MENU;
	}
	if (init) {
		combo_selected = 0;
		combo_position = 0;
		combo_stage = 0;
		combo_scrolling = false;
		combo_draw();
	}
	
	if (!scroller_status) {
		combo_scrolling = false;
		return COMBO;
	}
	
	if (!combo_scrolling) {
		combo_scrolling = true;
		combo_lastLocation = getTouchLocation();
		return COMBO;
	}
	
	int combo_scroll = -1 * (getTouchLocation() - combo_lastLocation);
	combo_lastLocation = getTouchLocation();
	while (combo_scroll < -180) combo_scroll+=360;
	while (combo_scroll > 180) combo_scroll-=360;
	scroll_dist += combo_scroll;
	
	switch(combo_stage) {
		case 0: //First number selected
			if (combo_scroll > 10) {
				combo_combo[0] = combo_selected;
				combo_stage++;
				scroll_dist=0;
			}
			break;
		case 1:
			if (combo_scroll < -10) {//didn't make full turn reset
				combo_stage = -1;
				scroll_dist = 0;
			}
			else if (scroll_dist > 350)
				combo_stage++;
			break;
		case 2: //Second number selected
			if (combo_scroll < -10) {
				combo_combo[1] = combo_selected;
				combo_stage++;
				scroll_dist = 0;
			}
			else if (scroll_dist > 640) {//went around twice
				combo_stage = -1;
				scroll_dist = 0;
			}
			break;
		case 3:
			if (combo_selected != combo_combo[1])
				combo_stage++;
			break;
		case 4:
			if (combo_scroll > 10) {//nope
				combo_stage = -1;
				scroll_dist = 0;
			}
			else if (scroll_dist < -360){ //went all the way around, reset
				combo_stage = 0;
				scroll_dist = 0;
			}
			break;
		default:
			if (combo_scroll > 10)
				combo_combo[0] = combo_selected;
			else if (scroll_dist < -320) //reset
			{
				combo_stage = 0;
				scroll_dist = 0;
			}
	}
	if (combo_scroll) {
		combo_position += combo_scroll;
		combo_position %= 360;
		while(combo_position>360) combo_position-=360; 
		while(combo_position<0) combo_position+=360; 
		combo_selected = ((combo_position+4)/9)%40;
		
		combo_draw();
	}
	
	return COMBO;
}