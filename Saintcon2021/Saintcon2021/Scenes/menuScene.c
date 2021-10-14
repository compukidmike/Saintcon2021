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
#include "machine_common.h"
#include "nfc.h"

#define MINIBADGE_DELAY	(30*1000)

static int menu_rotation, menu_selected, menu_lastLocation;
static uint8_t menu_frame;
static bool menu_scrolling, vcard_enabled;
static uint8_t minibadge_button;
static char minibadge_message[256];
uint32_t minibadge_delay;
static uint8_t minibadge_addr;

const char* menu_options[] = {"SAINTCON", "Build", "Trading", "Combo Lock", "Game", "The Machine", "Inventory", "Read Tag"};
struct io_descriptor *I2C_0_io;


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
		int p2x = p1x*1.469f;
		int p2y = p1y*1.469f;

		canvas_drawLine(p1x+120, p1y+120, p2x+120, p2y+120, 0x8a52);
	}
	
	//draw center
	int idx = menu_frame%4;
	canvas_drawImage_FromFlash_pt(40,30,160,170,MENU_IMG, 0, 170*idx, 160, RGB(255,0,255));
	
	if (minibadge_button) {
		canvas_drawImage_FromFlash(136, 142, 36, 36, WHAMMY_IMG);
		minibadge_button--;
	}
	else if(!vcard_enabled)
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

void minibagde_holder_init() {
	i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
	i2c_m_sync_enable(&I2C_0);
	
	for (int addr=32; addr<40; ++addr) {
		i2c_m_sync_set_slaveaddr(&I2C_0, addr, I2C_M_SEVEN);
		io_write(I2C_0_io, (uint8_t *)"\x03\x00", 2);
		io_write(I2C_0_io, (uint8_t *)"\x01\xFF", 2);
	}
}

bool menu_check_minibadge(uint8_t addr) {
	uint8_t b;
	bool found = false;
	
	i2c_m_sync_set_slaveaddr(&I2C_0, addr, I2C_M_SEVEN);
	io_write(I2C_0_io, (uint8_t *)"\x00\x01", 2);
	if (io_read(I2C_0_io, &b, 1)==1) {
		found = true;
		if (b==1) {
			minibadge_button = 128;
			return;
		}
		else if (b==2) {
			if (io_read(I2C_0_io, &b, 1) == 1) {
				uint8_t l = io_read(I2C_0_io, minibadge_message, b);
				if (l == b) {
					minibadge_message[b] = 0;
					minibadge_delay = millis() + MINIBADGE_DELAY;
				}
			}
		}
	}
	return found;
}

Scene menu_scene_loop(bool init) {
	back_event=false;
	uint32_t now = millis();

	if (init) {
		menu_rotation =0;
		menu_selected =0;
		menu_lastLocation =0;
		menu_scrolling = false;
		vcard_enabled = false;
		minibadge_button = 0;
	}
	if (claspopen != vcard_enabled) {
		if (claspopen) {
			char vcard[512];
			flash_read_vcard(vcard);
			ndef_vcard(vcard);
		}
		else {
			uint8_t ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','.','o','r','g'};
			ndef_well_known(ndef_data, sizeof(ndef_data));
		}
		vcard_enabled = claspopen;
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
	if (true) {
		if (menu_check_minibadge(minibadge_addr++))
			if (newUnlock(UNLOCK_MINI))
				return REWARD;
		if (minibadge_addr >=128)
			minibadge_addr = 40;
	}
	menu_draw();
	return MENU;
}