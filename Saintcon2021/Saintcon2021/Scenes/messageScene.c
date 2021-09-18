/*
 * messageScene.c
 *
 * Created: 9/14/2021 5:53:40 PM
 *  Author: Professor Plum
 */ 

#include <math.h>
#include <stdio.h>
#include "main.h"
#include "FrameBuffer.h"
#include "flash.h"

static char msg_line[4][25];

void setMessage(const char* msg) {
	char* ptr=msg;
	
	for (int i=0;i<4; ++i) {
		if (strlen(ptr) < 24) {
			strcpy(msg_line[i++], ptr);
			while(i<4)
				msg_line[i++][0]='\0';
		}
		else {
			char *p2 = ptr+24;
			while ((*p2!=' ') && (p2>ptr)) 
				--p2;
			if (p2 == ptr) {
				strncpy(msg_line[i], ptr, 24);
				msg_line[i][24]='\0';
				ptr+=24;
			}
			else {
				uint8_t ll = p2-ptr;
				strncpy(msg_line[i], ptr, ll);
				msg_line[i][ll]='\0';
				ptr=p2+1;
			}
		}
	}

}

Scene message_scene_loop(bool init) {
	if (init) {
		canvas_clearScreen(RGB(200,25,87));
		for (int i=0; i<4; ++i) {
			int l = strlen(msg_line[i]);
			canvas_drawText(120-l*4, i*16 + 88, msg_line[i], RGB(255,255,255));
		}
		canvas_blt();
	}
	if (back_event) {
		back_event=false;
		return MENU;
	}
	return MESSAGE;
}