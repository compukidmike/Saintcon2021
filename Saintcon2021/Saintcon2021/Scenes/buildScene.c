/*
 * buildScene.c
 *
 * Created: 8/22/2021 6:43:50 AM
 *  Author: Professor Plum
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "flash.h"
#include "machine_common.h"
#include "FrameBuffer.h"

#define BUILDTIME   3000

int build_index;
bool build_touching;
int build_lastTouch;
uint32_t build_touchStart, build_holdtime;

bool build_hasParts() {
	const module *mod = &module_info[build_index];
	for (int i=0; i<4; ++i) {
		if (mod->reqparts[i].part != none) {
			if (g_state.part_count[mod->reqparts[i].part] < mod->reqparts[i].count)
			return false;
		}
	}
	return true;
}

bool build_metPreReqs() {
	const module *mod = &module_info[build_index];
	return (!mod->prereqs) || ((g_state.modules_bitmask & mod->prereqs) == mod->prereqs);
}

void build_draw() {
	const module *mod = &module_info[build_index];
	char line[20];
	canvas_drawImage_FromFlash(0, 0, 240, 240, build_img);
	
	if (build_metPreReqs()) {
		int x = 120-(int)strlen(mod->name)*4;
		canvas_drawText(x, 102, mod->name, RGB(200,200,200));
		int rx = mod->src_x;
		int rw = mod->src_w;
		if (rw > 78)
		{
			rx += (rw-78)/2;
			rw = 78;
		}
		int xoff = rw /2;
		int yoff = mod->src_h /2;
		canvas_drawImage_FromFlash_pt(120-xoff,46-yoff, rw, mod->src_h, machine_img, rx, mod->src_y, 240, RGB(255,0,255));
		
		
		if ((g_state.modules_bitmask & mod->id) == 0 ){
			for (int i=0; i<4; ++i) {
				if (mod->reqparts[i].part != none) {
					uint16_t color = RGB(200,200,200);
					snprintf(line, 20, "%2d/%d    %s", g_state.part_count[mod->reqparts[i].part], mod->reqparts[i].count, part_names[mod->reqparts[i].part]);
					if (g_state.part_count[mod->reqparts[i].part] < mod->reqparts[i].count)
					color = RGB(200,20,20);
					canvas_drawText(50, 132 + i * 16, line, color);
					int px = (mod->reqparts[i].part % 4)*16;
					int py = (mod->reqparts[i].part / 4)*16;
					canvas_drawImage_FromFlash_pt(90, 132 + i * 16, 16, 16, parts_img, px, py, 64, RGB(242, 170, 206));
				}
			}
			if (!build_hasParts()) {
				canvas_drawImage_FromFlash_p(80, 210, 81, 30, build_img, 80, 244, 240);
			}
		}
		else {
			canvas_drawText(90, 146, "Already", RGB(200,200,200));
			canvas_drawText(98, 162, "Built", RGB(200,200,200));
			canvas_drawImage_FromFlash_p(80, 210, 81, 30, build_img, 80, 244, 240);
		}
		
		
	}
	else {
		canvas_drawText(96, 102, "??????", RGB(200,200,200));
		canvas_drawImage_FromFlash_p(80, 210, 81, 30, build_img, 80, 244, 240);
	}
	
	if (build_touching) {
		if (build_lastTouch == 2)
		canvas_drawImage_FromFlash_p(208, 88, 32, 64, build_img, 208, 240, 240);
		else if (build_lastTouch == 6)
		canvas_drawImage_FromFlash_p(0, 88, 32, 64, build_img, 0, 240, 240);
		else if ((build_lastTouch == 4) && build_holdtime) {
			canvas_drawImage_FromFlash_p(80, 210, 81, 30, build_img, 80, 274, 240);
			int h = build_holdtime * 68 / BUILDTIME;
			canvas_fillRect(86, 218, h, 2, RGB(180,180,255));
		}
	}
	
	canvas_blt();
}

Scene build_scene_loop(bool init) {
	if (back_event) {
		back_event=false;
		return MENU;
	}
	
	if (init) {
		build_index=0;
		build_touching = false;
		build_holdtime = 0;
		build_draw();
	}
	if (scroller_status) {
		int xx = getTouchLocation();
		build_lastTouch = ((xx+ 22) / 45)%8;
		if (build_touching) {
			uint32_t now = millis();
			const module *mod = &module_info[build_index];
			if (build_hasParts() && ((g_state.modules_bitmask & mod->id) == 0 ) && (build_lastTouch == 4)) {
				if ((now - build_touchStart) > BUILDTIME) { //build it!
					for (int i=0; i<4; ++i) {
						if (mod->reqparts[i].part != none) {
							g_state.part_count[mod->reqparts[i].part] -= mod->reqparts[i].count;
						}
					}
					g_state.modules_bitmask |= (1<<build_index);
					build_touchStart = now;
				}
				build_holdtime = now - build_touchStart;
			}
		}
		if (!build_touching) {
			const module *mod = &module_info[build_index];
			build_holdtime = 0;
			if (build_hasParts() && ((g_state.modules_bitmask & mod->id) == 0 ) && (build_lastTouch == 4))
			build_touchStart = millis();
		}
		build_touching = true;
		build_draw();
	}
	else if (build_touching) {
		build_touching = false;
		if (build_lastTouch == 2) {//next
			do {
				if (++build_index == MODULE_COUNT) build_index = 0;
			} while (!build_metPreReqs());
		}
		else if (build_lastTouch == 6) {//prev
			do {
				if (--build_index < 0) build_index = MODULE_COUNT - 1;
			} while (!build_metPreReqs());
		}
		build_draw();
	}
	return BUILD;
}
