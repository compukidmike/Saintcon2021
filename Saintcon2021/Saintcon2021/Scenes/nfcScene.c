/*
 * nfcScene.c
 *
 * Created: 9/23/2021 10:10:32 PM
 *  Author: Professor Plum
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "flash.h"
#include "machine_common.h"
#include "FrameBuffer.h"

int nfc_frame, nfc_lastDraw;

#define NFC_FRAME_SPAN  250

void nfc_draw() {
	const int fc[] = {0,1,2,3,2,1};
	int xf = fc[nfc_frame];
	canvas_drawImage_FromFlash_p(0, 0, 240, 240, NFC_IMG, 0, 240*xf, 240);
	canvas_blt();
}

Scene nfc_scene_loop(bool init) { 
	if (back_event) {
		back_event=false;
		//TODO: turn off NFC field
		return MENU;
	}
	
	if (init) {
		nfc_frame = 0;
		nfc_lastDraw = 0;
		//TODO: turn on NFC field
	}
	
	uint32_t now = millis();
	int dt = now - nfc_lastDraw;
	if (dt >= NFC_FRAME_SPAN) {
		if (++nfc_frame >= 6)
		nfc_frame %= 6;
		nfc_draw();
		nfc_lastDraw = now;
	}
	return NFCREADER;
}