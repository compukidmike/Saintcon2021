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
#include "flash.h"

static int menu_rotation, menu_selected, menu_lastLocation;
static uint8_t menu_frame;
static bool menu_scrolling, vcard_enabled;

const char* menu_options[] = {"SAINTCON", "Build", "Trading", "Combo Lock", "Testing", "The Machine", "Inventory", "Read Tag"};

uint32_t menu_last;
void menu_draw() {
	uint32_t now = millis();
	canvas_clearScreen(RGB(42,19,44));
	
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
	int idx = menu_frame%4;
	canvas_drawImage_FromFlash_pt(40,30,160,170,MENU_IMG, 0, 170*idx, 160, RGB(255,0,255));
	
	if(!vcard_enabled)
		canvas_drawBitmask(138, 144, 32, 32, no_icon, RGB(200,0,0), 0);//rad);
	
	const char* str_line = menu_options[menu_selected];
	int offset = (int)strlen(str_line) * 4;
	
	canvas_drawText(120-offset, 54, str_line, 0xFFFF);
	
	char fps[10];
	sprintf(fps, "%d", 1000/(now-menu_last));
	//canvas_drawText(110, 80, fps, 0xFFFF);
	
	
	canvas_blt();
	menu_last = now;
}

Scene menu_scene_loop(bool init) {
	back_event=false;

	if (init) {
		menu_rotation =0;
		menu_selected =0;
		menu_lastLocation =0;
		menu_scrolling = false;
		vcard_enabled = false;
		menu_draw();
	}
	if (claspopen != vcard_enabled) {
		//TODO: enable or disable NFC vcard
		vcard_enabled = claspopen;
		menu_draw();
	}
	if (scroller_status) {
		int touchAt = getTouchLocation();
		if (menu_scrolling) {
			int diff = touchAt - menu_lastLocation;
			if (diff > 0)
				menu_frame++;
			else if (diff < 0)
				menu_frame--;
			menu_rotation += diff;
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