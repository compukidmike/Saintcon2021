/*
 * inventoryScene.c
 *
 * Created: 8/21/2021 9:35:53 PM
 *  Author: Professor Plum
 */ 

#include <stdio.h>
#include "main.h"
#include "FrameBuffer.h"
#include "machine_common.h"
#include "flash.h"

Scene inventory_scene_loop(bool init) {
	if (back_event) return MENU;
	if (init) {
		canvas_clearScreen(0);
		char line[20];
		int y = 24;
		for (int i=0; i<12; ++i) {
			if (g_state.part_count[i]) {
				snprintf(line, 20, "%2dx    %s", g_state.part_count[i], part_names[i]);
				canvas_drawText(56, y, line, RGB(200,200,200));
				int px = (i % 4)*16;
				int py = (i / 4)*16;
				canvas_drawImage_FromFlash_pt(88, y, 16, 16, parts_img, px, py, 64, RGB(242, 170, 206));
				y+=16;
			}
		}
		canvas_blt();
	}
	return INVENTORY;
}