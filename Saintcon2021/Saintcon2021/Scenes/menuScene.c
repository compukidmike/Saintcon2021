/*
 * menuScene.c
 *
 * Created: 8/21/2021 8:37:35 PM
 *  Author: Professor Plum
 */ 

#include <math.h>
#include <string.h>
#include "main.h"
#include "FrameBuffer.h"
#include "menu_icons.h"

int menu_rotation, menu_selected, menu_lastLocation;
bool menu_scrolling;

const char* menu_options[] = {"SAINTCON", "Build", "Trading", "Combo Lock", "Testing", "The Machine", "Inventory", "VCard"};

void menu_draw() {
	canvas_clearScreen(0x857a);
	
	canvas_fillCircle(120, 120, 80, 0x8a52);
	
	//draw icons
	for (int i=1; i<8; ++i) {
		int ang = menu_rotation+i*45;
		float rad = ang * M_PI / 180.0;
		int px = 100*sinf(rad) + 104;
		int py = -100*cosf(rad) + 104;

		canvas_drawBitmask(px, py, 32, 32, menu_icons[i-1], 0xFFFF, 0);//rad);
	}
	
	//draw fingers
	for (int i=0; i<8; ++i) {
		int ang = menu_rotation+i*45+22;
		float rad = ang * M_PI / 180.0;
		int p1x = 81*sinf(rad);
		int p1y = -81*cosf(rad);
		int p2x = p1x*1.5f;
		int p2y = p1y*1.5f;

		canvas_drawLine(p1x+120, p1y+120, p2x+120, p2y+120, 0x8a52);
	}
	
	//draw center
	
	const char* str_line = menu_options[menu_selected];
	int offset = (int)strlen(str_line) * 4;
	
	canvas_drawText(120-offset, 115, str_line, 0xFFFF);
	
	canvas_blt();
}

Scene menu_scene_loop(bool init) {
	back_event=false;

	if (init) {
		menu_rotation =0;
		menu_selected =0;
		menu_lastLocation =0;
		menu_scrolling = false;
		menu_draw();
	}
	if (scroller_status) {
		int touchAt = getTouchLocation();
		if (menu_scrolling) {
			menu_rotation += touchAt - menu_lastLocation;
			while (menu_rotation < 0)
			menu_rotation += 360;
			while (menu_rotation >= 360)
			menu_rotation-=360;
			menu_selected = ((menu_rotation + 22) / 45)%8;
			menu_draw();
		}
		else
		menu_scrolling = true;
		menu_lastLocation = touchAt;
	}
	else {
		if (menu_selected)
		return menu_selected;
		menu_scrolling = false;
	}
	return MENU;
}