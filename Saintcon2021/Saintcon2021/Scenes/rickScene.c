/*canvas_drawImage_FromFlash_p
 * rickScene.c
 *
 * Created: 10/16/2021 7:05:19 PM
 *  Author: compukidmike
 */ 

/*
 * machineScene.c
 *
 * Created: 8/21/2021 9:47:48 PM
 *  Author: Professor Plum
 */ 

#include "main.h"
#include "FrameBuffer.h"
#include "flash.h"

int rick_frame, rick_lastDraw;

#define FRAME_SPAN  33

void rick_draw() {
	canvas_fillRect(0,0,240,240,RGB(rick_frame,160-rick_frame,rick_frame+90));
	canvas_drawImage_FromFlash_p_double(20, 45, 100, 75, RICK_IMG, 0, rick_frame*75, 100);
	canvas_blt();
}

Scene rick_scene_loop(bool init) {
	if (back_event) {
		back_event=false;
		return MENU;
	}
	if (init) {
		rick_frame = 0;
		canvas_fillRect(0,0,240,240,RGB(0,0,0));
	}
	uint32_t now = millis();
	int dt = now - rick_lastDraw;
	if (dt >= FRAME_SPAN) {
		if (++rick_frame >= 159)
		rick_frame %= 159;
		rick_draw();
		rick_lastDraw = now;
	}
	return RICK;
}