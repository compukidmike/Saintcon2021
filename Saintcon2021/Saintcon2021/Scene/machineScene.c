/*
 * machineScene.c
 *
 * Created: 8/21/2021 9:47:48 PM
 *  Author: Professor Plum
 */ 

#include "main.h"
#include "machine_common.h"
#include "FrameBuffer.h"
#include "flash.h"

int machine_frame, machine_lastDraw;

#define FRAME_SPAN  100

void machine_draw() {
	canvas_drawImage(0, 0, 240, 240, machine_bkg_img);
	
	for (int i=0; i< MODULE_COUNT; ++i) {
		module a = module_info[i];
		if (g_state.modules_bitmask & a.id) {
			a.src_y += machine_frame*240;
			canvas_drawImage_FromFlash_pt(a.dest_x, a.dest_y, a.src_w, a.src_h, machine_img, a.src_x, a.src_y, 240, RGB(255,0,255));
		}
	}

	canvas_blt();
}

Scene machine_scene_loop(bool init) {
	if (back_event) return MENU;
	if (init) {
		machine_frame = 0;
	}
	uint32_t now = millis();
	int dt = now - machine_lastDraw;
	if (dt >= FRAME_SPAN) {
		if (++machine_frame >= 20)
		machine_frame %= 20;
		machine_draw();
		machine_lastDraw = now;
	}
	return MACHINE;
}