/*
 * tradeScene.c
 *
 * Created: 10/8/2021 4:04:32 PM
 *  Author: Professor Plum
 */ 

#include "main.h"
#include "FrameBuffer.h"
#include "flash.h"
#include "machine_common.h"
#include <stdio.h>

static requirement outgoing[4], received[4];
static uint8_t trade_idx, trade_slot;
static bool trade_more, trade_less, trade_complete, trade_touching;
static uint8_t trade_frame;
static bool trade_btn_dwn[4];


const uint8_t trade_bits[3][128] = {
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xE0, 0x03, 0xC0, 0x07, 0xF0, 0x07, 0xE0, 0x0F,
		0x38, 0x0E, 0x70, 0x1C, 0x1C, 0x1C, 0x38, 0x38, 0x0C, 0x18, 0x18, 0x30,
		0x0C, 0x18, 0x18, 0x30, 0x0C, 0x18, 0x18, 0x30, 0x0C, 0x18, 0x18, 0x30,
		0xEC, 0x1B, 0xD8, 0x37, 0xFC, 0x1F, 0xF8, 0x3F, 0xFC, 0x1F, 0xF8, 0x3F,
		0x3E, 0x3E, 0x7C, 0x7C, 0x0E, 0x38, 0x1C, 0x70, 0x0F, 0x78, 0x1E, 0xF0,
		0x07, 0x70, 0x0E, 0xE0, 0x07, 0x70, 0x0E, 0xE0, 0x07, 0x70, 0x0E, 0xE0,
		0x0F, 0x78, 0x1E, 0xF0, 0x0E, 0x38, 0x1C, 0x70, 0x3E, 0x3E, 0x7C, 0x7C,
		0xFC, 0x1F, 0xF8, 0x3F, 0xF8, 0x0F, 0xF0, 0x1F, 0xE0, 0x03, 0xC0, 0x07,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0xE0, 0x01, 0xE0, 0x07, 0xF0, 0x03,
		0xE0, 0x07, 0xF0, 0x03, 0x70, 0x0E, 0x38, 0x07, 0x30, 0x0C, 0x18, 0x06,
		0x30, 0x0C, 0x18, 0x06, 0x30, 0x0C, 0x18, 0x06, 0x30, 0x0C, 0x18, 0x06,
		0xF0, 0x0F, 0xF8, 0x07, 0xF0, 0x0F, 0xF8, 0x07, 0xF0, 0x0F, 0xF8, 0x0F,
		0xF8, 0x1E, 0xBC, 0x0F, 0x38, 0x18, 0x0C, 0x1E, 0x3C, 0x38, 0x0E, 0x1E,
		0x3C, 0x38, 0x0E, 0x1E, 0x3C, 0x38, 0x0E, 0x1E, 0x3C, 0x38, 0x0E, 0x1E,
		0x3C, 0x38, 0x0E, 0x1E, 0x38, 0x18, 0x0C, 0x0E, 0xF8, 0x1E, 0xBC, 0x0F,
		0xF0, 0x0F, 0xF8, 0x07, 0xE0, 0x07, 0xF0, 0x03, 0xC0, 0x03, 0xE0, 0x01,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00,
		0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00,
		0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, }	
	};

void calc_trade_options() {
	for (trade_slot=0; trade_slot<4; ++trade_slot) {
		if (outgoing[trade_slot].part ==  trade_idx) {
			break;
		}
		else if (outgoing[trade_slot].part == none) {
			break;
		}
	}
	if ((trade_slot<4) && (outgoing[trade_slot].part != none) && (outgoing[trade_slot].count)) {
		trade_less = true;
	}
	else
		trade_less = false;
		
	if ((trade_slot<4) && (outgoing[trade_slot].count < g_state.part_count[trade_idx])) {
		trade_more = true;
	}
	else
		trade_more = false;
}

void free_slot() {
	for (int i=0; i<3; ++i) {
		if (outgoing[i].count == 0) {
			outgoing[i].part = outgoing[i+1].part;
			outgoing[i].count = outgoing[i+1].count;
			outgoing[i+1].part = none;
			outgoing[i+1].count = 0;
		}
	}
	if (outgoing[3].count == 0)
		outgoing[3].part = none;
}

void trade_scene_draw() {
	char line[21];
	if (trade_complete) {
		canvas_drawImage_FromFlash(0,0, 240, 240, TRADE2_IMG);
		for (int i = 0; i<4; ++i) {
			if (outgoing[i].part != none) {
				snprintf(line, 21, "-%2dx    %s", outgoing[i].count, part_names[outgoing[i].part]);
				canvas_drawText(36, 32 + 16*i, line, RGB(200,0,0));
				int px = (outgoing[i].part % 4)*16;
				int py = (outgoing[i].part / 4)*16;
				canvas_drawImage_FromFlash_pt(68,  32 + 16*i, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
			}
		}
		for (int i = 0; i<4; ++i) {
			if (received[i].part != none) {
				snprintf(line, 21, "+%2dx    %s",  received[i].count, part_names[received[i].part]);
				canvas_drawText(60, 146 + 16*i, line, RGB(0,200,0));
				int px = (received[i].part % 4)*16;
				int py = (received[i].part / 4)*16;
				canvas_drawImage_FromFlash_pt(92,  146 + 16*i, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
			}
		}
	}
	else {
		canvas_drawImage_FromFlash(0,0, 240, 240, TRADE_IMG);
		for (int i = 0; i<4; ++i) {
			if (outgoing[i].part != none) {
				snprintf(line, 21, "%2dx    %s", outgoing[i].count, part_names[outgoing[i].part]);
				canvas_drawText(52, 72 + 16*i, line, RGB(200,200,200));
				int px = (outgoing[i].part % 4)*16;
				int py = (outgoing[i].part / 4)*16;
				canvas_drawImage_FromFlash_pt(84,  72 + 16*i, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
			}
		}
		snprintf(line, 21, "%2dx    %s", g_state.part_count[trade_idx], part_names[trade_idx]);
		canvas_drawText(52, 164, line, RGB(200,200,200));
		int px = (trade_idx % 4)*16;
		int py = (trade_idx / 4)*16;
		canvas_drawImage_FromFlash_pt(84,  164, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
		
		if (trade_more == false) {
			canvas_fillRect(96,0,48,25, RGB(73,61,74));
		}
		else if(trade_btn_dwn[0]) {
			canvas_drawImage_FromFlash_p(96, 0 , 48, 25, TRADE_IMG, 96, 264, 240);
		}
		if (trade_less == false) {
			canvas_fillRect(96,216,48,24, RGB(73,61,74));
		}
		else if(trade_btn_dwn[1]) {
			canvas_drawImage_FromFlash_p(96, 215, 48, 25, TRADE_IMG, 96, 240, 240);
		}
		if (trade_btn_dwn[2]) {
			canvas_drawImage_FromFlash_p(0, 96, 24, 48, TRADE_IMG, 0, 240, 240);
		}
		if (trade_btn_dwn[3]) {
			canvas_drawImage_FromFlash_p(216, 96, 24, 48, TRADE_IMG, 216, 240, 240);
		}
	}
	int fc[] = {0,0,0,0,0,1,1,1,1,2,2,2,2,2,2,2};
	canvas_drawBitmask(160, 24, 32, 32, trade_bits[fc[trade_frame++ % 16]], RGB(200,200,200), 0);
	canvas_blt();
}

Scene trade_scene_loop(bool init) {
	if (init) {
		uint16_t goods=0;
		for (int i=0;i<12;++i) {
			goods += g_state.part_count[i];
		}
		if (goods == 0) {
			setMessage("You have no parts to trade");
			return MESSAGE;
		}
		trade_complete = false;
		for (int i=0; i<4; ++i) {
			outgoing[i].count = 0;
			outgoing[i].part = none;
			received[i].count = 0;
			received[i].part = none;
			trade_btn_dwn[i] = false;
			calc_trade_options();
		}
		trade_frame = 0;
		//TODO: enable trade NFC 
	}
	if (back_event) {
		//TODO: disable trade NFC
		back_event=false;
		if (trade_complete && newUnlock(UNLOCK_TRADE))
			return REWARD;
		return MENU;
	}
	
	bool touching = scroller_status != 0;
	if (touching != trade_touching) {
		if (touching) {
			uint16_t v = ((scroller_position+8)%256)/16;
			switch (v) {
				case 15:
				case 0:
				case 1:
					if (trade_more) {
						outgoing[trade_slot].count++;
						outgoing[trade_slot].part=trade_idx;
						trade_btn_dwn[0] = true;
					}
					break;
				case 3:
				case 4:
				case 5:
					do {
						trade_idx++;
						if (trade_idx>= 12)
							trade_idx=0;
					} while (g_state.part_count[trade_idx] == 0);
					trade_btn_dwn[3] = true;
					break;
				case 7:
				case 8:
				case 9:
					if (trade_less) {
						outgoing[trade_slot].count--;
						trade_btn_dwn[1] = true;
						if (outgoing[trade_slot].count==0)
							free_slot();
					}
					break;
				case 11:
				case 12:
				case 13:
					do {
						trade_idx--;
						if (trade_idx>= 12)
						trade_idx=11;
					} while (g_state.part_count[trade_idx] == 0);
					trade_btn_dwn[2] = true;
					break;
				default:
					break;
			}
			calc_trade_options();
		}
		else {
			trade_btn_dwn[0]=false;
			trade_btn_dwn[1]=false;
			trade_btn_dwn[2]=false;
			trade_btn_dwn[3]=false;
		}
		trade_touching = touching;

	}
	
	if (trade_frame == 200) {
		//TODO: Try finding other badges
	}
	trade_scene_draw();
	return TRADING;
}