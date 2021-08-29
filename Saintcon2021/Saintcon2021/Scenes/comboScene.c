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

int combo_selected;
int combo_frame;
int combo_stage;
int combo_combo[2];
int combo_lastLocation;
int combo_scroll;
bool combo_scrolling;


void combo_draw() {
	canvas_clearScreen(0);
    
    //draw numbers
    for (int i=0; i<40; i+=5) {
        int a = (i*9 - combo_selected*9 + 360) % 360;
        float b = ((i-combo_selected)*M_PI/20);
        
		int px = 80*sinf(b) + 106;
		int py = -80*cosf(b) + 106;

        canvas_drawBitmask(px, py, 32, 32, num_bits[i/5], 0xFFFF, a * M_PI / 180.0);
        
    }
    
    //draw fingers
    for (int i=0; i<40; ++i) {
        float ang = i*M_PI/20;
		int p1x = 90*sinf(ang);
		int p1y = -90*cosf(ang);

        /*Point p1(sine_tbl[i*9], sine_tbl[(i*9+270)%360]);
        p1 -= Point(90,90);*/
        int p2x = p1x *1.3f;
		int p2y = p1y *1.3f;
        if (((i+combo_selected)%5)==0) {
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
	int a = (360-combo_selected*9) % 360;
	canvas_drawBitmask(96, 96, 48, 48, saint_bits, RGB(160,160,160), a * M_PI / 180.0);
    
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
    canvas_drawText(90, 130, cmb, RGB(255,0,0));
    canvas_blt();
}

Scene combo_scene_loop(bool init) {
	if (back_event) {
		back_event=false;
		return MENU;
	}
	if (init) {
		combo_selected = 0;
		combo_stage = 0;
		combo_scroll = 0;
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
	
	combo_scroll += -1 * (getTouchLocation() - combo_lastLocation);
	combo_lastLocation = getTouchLocation();
	while (combo_scroll < -180) combo_scroll+=360;
	while (combo_scroll > 180) combo_scroll-=360;
	
	switch(combo_stage) {
		case 0: //First number selected
		if (combo_scroll > 10) {
			combo_combo[0] = combo_selected;
			combo_stage++;
		}
		break;
		case 1:
		if (combo_scroll < -10) //didn't make full turn reset
		combo_stage = -1;
		else if (combo_selected == combo_combo[0])
		combo_stage++;
		break;
		case 2:
		if (combo_selected != combo_combo[0])
		combo_stage++;
		break;
		case 3: //Second number selected
		if (combo_scroll < -10) {
			combo_combo[1] = combo_selected;
			combo_stage++;
		}
		else if (combo_selected == combo_combo[0]) //went around twice
		combo_stage = -1;
		break;
		case 4:
		if (combo_selected != combo_combo[1])
		combo_stage++;
		break;
		case 5:
		if (combo_scroll > 10) {//nope
			combo_stage = -1;
			combo_combo[0] = combo_selected;
		}
		else if (combo_selected == combo_combo[1]) //went all the way around, reset
		combo_stage = 0;
		break;
		default:
		if (combo_scroll > 10)
		combo_combo[0] = combo_selected;
		else if (combo_selected == combo_combo[0]) //reset
		combo_stage = 0;
	}
	if (combo_scroll) {
		combo_selected += (combo_scroll/9);
		combo_scroll%=9;

		if (combo_selected < 0)
		combo_selected += 40;
		if (combo_selected >= 40)
		combo_selected -= 40;
		combo_draw();
	}
	
	return COMBO;
}