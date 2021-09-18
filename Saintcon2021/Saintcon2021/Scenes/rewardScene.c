/*
 * rewardScene.c
 *
 * Created: 9/14/2021 4:53:39 PM
 *  Author: Professor Plum
 */ 

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "machine_common.h"
#include "FrameBuffer.h"
#include "flash.h"
#include "eeprom.h"

requirement loot[4];

static uint8_t reward_frame;

void decideLoot() {
	for (int i=0; i<4; ++i) {
		uint r = rand() % 20;
		loot[i].part = rand() % 13;
		switch(loot[i].part) {
			case none:
				loot[i].count = 0;
				break;	
			case wire:
			case gear:
			case scrap:
				if (r < 3)
					loot[i].count = 3;
				else if (r > 17)
					loot[i].count = 5;
				else 
					loot[i].count = 4;
				break;
			case circuit:
			case screen:
			case metal:
				if (r < 3)
					loot[i].count = 2;
				else if (r > 17)
					loot[i].count = 4;
				else
					loot[i].count = 3;
				break;
			case lens:
			case fluid:
			case engine:
			case flux:
			case tesla:
			case fuse:
				if (r > 17)
					loot[i].count = 2;
				else
					loot[i].count = 1;
				break;
		}
		g_state.part_count[loot[i].part] += loot[i].count;
	}
	eeprom_save_state();
}

Scene reward_scene_loop(bool init) {
	char line[20];
	if (init) {
		reward_frame = 0;
		decideLoot();
	}
	
	if (back_event) {
		back_event=false;
		return MENU;
	}
	
	canvas_clearScreen(RGB(82, 204, 153));
	canvas_drawImage_FromFlash_p(60, 0, 120, 160, CRATE_IMG, (reward_frame/2) * 120, 0, 600);
	
	if (reward_frame == 10) {
		canvas_drawText(56, 144, "You've received:", 0);
		uint8_t y=160;
		for (int i=0; i<4; ++i) {
			if (loot[i].count) {
				uint8_t p = loot[i].part;
				snprintf(line, 20, "%2dx    %s", loot[i].count, part_names[p]);
				canvas_drawText(56, y, line, 0);
				int px = (p % 4)*16;
				int py = (p / 4)*16;
				canvas_drawImage_FromFlash_pt(88, y, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
				y+=16;
			}
		}
	}
	
	canvas_blt();
	
	if (reward_frame < 10) ++reward_frame;
	return REWARD;
}