/*
 * buildScene.c
 *
 * Created: 8/22/2021 6:43:50 AM
 *  Author: Professor Plum
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "flash.h"
#include "machine_common.h"
#include "FrameBuffer.h"

int build_index;
bool build_touching;
int build_scroll, build_lastTouch;

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
	canvas_drawImage_FromFlash(0, 0, 240, 240, BUILD_IMG);
	
	if (build_metPreReqs()) {
		int x = 120-(int)strlen(mod->name)*4;
		canvas_drawText(x, 100, mod->name, RGB(200,200,200));
		int rx = mod->src_x;
		int rw = mod->src_w;
		if (rw > 78)
		{
			rx += (rw-78)/2;
			rw = 78;
		}
		int xoff = rw /2;
		int yoff = mod->src_h /2;
		canvas_drawImage_FromFlash_pt(120-xoff,46-yoff, rw, mod->src_h, MACHINE_IMG, rx, mod->src_y, 240, RGB(255,0,255));
		
		
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
					canvas_drawImage_FromFlash_pt(90, 129 + i * 16, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
				}
			}
			if (build_hasParts()) {
				//draw unlock
			}
		}
		else {
			canvas_drawText(90, 146, "Already", RGB(200,200,200));
			canvas_drawText(98, 162, "Built", RGB(200,200,200));
			//draw check mark
			//canvas_drawImage_FromFlash_p(80, 210, 81, 30, BUILD_IMG, 80, 244, 240);
		}
		
		
	}
	else {
		canvas_drawText(96, 102, "??????", RGB(200,200,200));
		canvas_drawImage_FromFlash_p(80, 210, 81, 30, BUILD_IMG, 80, 244, 240);
	}
	
	float angle = build_scroll * M_PI / 180.0;
	int bx = 114*sin(angle)+114;
	int by = 114*cos(angle)+114;
	canvas_drawImage_FromFlash_pt(bx, by, 12, 12,BALL_IMG, 0, 0, 12, RGB(242, 170, 206));
	
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
		build_scroll = 0;
		build_draw();
	}
	
	const module *mod = &module_info[build_index];
	if (unlock_event && build_hasParts() && ((g_state.modules_bitmask & mod->id) == 0 )) {
		for (int i=0; i<4; ++i) {
			if (mod->reqparts[i].part != none) {
				g_state.part_count[mod->reqparts[i].part] -= mod->reqparts[i].count;
			}
		}
		g_state.modules_bitmask |= (1<<build_index);
		build_draw();
	}
	
	if (scroller_status) {
		int touchAt = getTouchLocation();
		if (build_touching) {
			int diff = build_lastTouch - touchAt;
			while (diff < -180) diff+=360;
			while (diff > 180) diff-=360;
			build_scroll += diff;
			if (build_scroll > 47) {
				do {
					if (++build_index == MODULE_COUNT) build_index = 0;
				} while (!build_metPreReqs());
				build_scroll -= 90;
			}
			if (build_scroll < -47) {
				do {
					if (--build_index < 0) build_index = MODULE_COUNT - 1;
				} while (!build_metPreReqs());
				build_scroll += 90;
			}
		}
		build_lastTouch = touchAt;
		build_touching = true;
		build_draw();
	}
	else if (build_touching) {
		build_touching = false;
	}
	return BUILD;
}
