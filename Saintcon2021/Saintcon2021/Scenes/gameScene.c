/*
 * gameScene.c
 *
 * Created: 9/24/2021 11:00:49 AM
 *  Author: Professor Plum
 */ 



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "flash.h"
#include "FrameBuffer.h"
#include "machine_common.h"

enum Curve { cnone, left, right, up, down };
enum TObject { POWERUP, VIRUS, DUST};

#define TUNNEL_LENGTH   10
#define POWERUP_GROUP   5
#define TUNNEL_OBJECTS ((TUNNEL_LENGTH-1)*(POWERUP_GROUP+4))
#define DEG2RAD(X) (((float)(X))* M_PI / 180.0)
#define MAXMOMENTUM 8

typedef struct {
	int z, rot;
	enum TObject type;
} tobject;

typedef struct {
	int x, y;
} point;

typedef struct {
	point center;
	int radius, ct, st;
} octogon;

typedef struct {
	int rotation;
	uint32_t score, fuel;
	int xMomentum, xInput;
	int frame;
	bool dead;
} tplayer;

static uint16_t zTraveled, level;
static enum Curve path[TUNNEL_LENGTH];
static tobject objects[TUNNEL_OBJECTS];
static tplayer player;


point octogon_calc_point(octogon o, int idx) {
	point p;
	switch (idx%8) {
		case 0:
		p.x = o.center.x - o.ct;
		p.y = o.center.y + o.st;
		break;
		case 1:
		p.x = o.center.x - o.ct;
		p.y = o.center.y - o.st;
		break;
		case 2:
		p.x = o.center.x - o.st;
		p.y = o.center.y - o.ct;
		break;
		case 3:
		p.x = o.center.x + o.st;
		p.y = o.center.y - o.ct;
		break;
		case 4:
		p.x = o.center.x + o.ct;
		p.y = o.center.y - o.st;
		break;
		case 5:
		p.x = o.center.x + o.ct;
		p.y = o.center.y + o.st;
		break;
		case 6:
		p.x = o.center.x + o.st;
		p.y = o.center.y + o.ct;
		break;
		case 7:
		p.x = o.center.x - o.st;
		p.y = o.center.y + o.ct;
		break;
	}
	return p;
}

point octogon_get_edge(octogon o, int rot) {
	float ax = DEG2RAD((rot+360)%360);
	point p = {cos(ax)*o.radius, sin(ax)*o.radius};
	p.x += o.center.x;
	p.y += o.center.y;
	return p;
}

int tunnel_get_radius(uint16_t zdist, uint8_t xdist) {
	int zt = zdist - zTraveled;
	if (zt < 0)
		return 160;
	int rad = (160-xdist)*8 / (8+zt);
	if (rad<0) return 0;
	return rad;
}

point tunnel_get_center_at(uint16_t zdist) {
	point center = {120,120};
	int zt = zdist - zTraveled;
	if (zt < 0) return center;
	int zz = (zTraveled%16);
	int end = ((zt+zz)/16);
	for (int i=0; i<=end; ++i) {
		int at = zTraveled/16 + i;
		int dd = 16;
		if (i==0)
			dd -=zz;
		if (i==end)
			dd = (zdist%16);
		
		if (at > 10 * TUNNEL_LENGTH)
			break;
		enum Curve s = path[at/10];
		if (s == left){
			center.x -= dd;
		}
		else if (s == right){
			center.x += dd;
		}
		else if (s == up){
			center.y -= dd;
		}
		else if (s == down) {
			center.y += dd;
		}
	}
	return center;
}

point tunnel_get_screen_location(uint16_t zdist, int rot) {
	point zcenter = {0,0};
	int zt = zdist - zTraveled;
	if (zt < 0) return zcenter;
	zcenter = tunnel_get_center_at(zdist);
	if (zdist > (16*6))
	return zcenter;
	return zcenter;
}

void tunnel_generate_path() {
	path[0] = none;
	for (int i=1; i<TUNNEL_LENGTH; ++i)
	switch(rand()%6) {
		case 0:
		path[i] = left;
		break;
		case 1:
		path[i] = right;
		break;
		case 2:
		path[i] = up;
		break;
		case 3:
		path[i] = down;
		break;
		default:
		path[i] = cnone;
	}
	
	for(int i=0; i<(TUNNEL_LENGTH-1); ++i) {
		int rot = rand() % 360;
		int spread = 10 + level * 10;
		if (spread > 40) spread = 40;
		int rt = (rand() % 2)?spread:-spread;
		tobject t1 = {(i+1)*160 + level * 10, rot-(90-level*2), VIRUS};
		tobject t2 = {(i+1)*160 + level * 10, rot+(90-level*2), VIRUS};
		objects[i*9] = t1;
		objects[i*9+1] = t2;
		for (int j=0; j<POWERUP_GROUP; ++j) {
			tobject t = {(i+1)*160 + j*6 + 60, rot, POWERUP};
			rot += rt;
			if (rot < 0) rot +=360;
			if (rot > 360) rot -= 360;
			objects[i*9+j+2] = t;
		}
		tobject t3 =  {(i+1)*160 + 80, rot-(90-level*2), VIRUS};
		tobject t4 =  {(i+1)*160 + 80, rot+(90-level*2), VIRUS};
		objects[i*9+7] = t3;
		objects[i*9+8] = t4;
	}
	
	return;
}

void tunnel_init() {
	level = 0;
	zTraveled = 0;
	
	player.frame=0;
	player.rotation=0;
	player.xMomentum=0;
	player.xInput=0;
	player.score=0;
	player.fuel=250;
	player.dead=false;
	
	tunnel_generate_path();
}

void tunnel_draw() {
	canvas_clearScreen(RGB(32,40,40));
	octogon lastring = {0,0,0,0};
	int zz = zTraveled%16;
	point center = {120, 120};
	for (int i=9; i>=0; --i) { //draw 6 rings
		uint8_t adj = ((i+1)*16 - zz);
		uint16_t color = RGB(100 - adj/2,200 - adj, 200 - adj);
		uint16_t rad = tunnel_get_radius(zTraveled-zz+i*16, 0);
		
		center = tunnel_get_center_at(zTraveled-zz + i*16);
		octogon ring= {center, rad, .924 * rad, .383 * rad};
		
		for (int ii=0; ii<8; ++ii) {
			point p1 = octogon_calc_point(ring, ii);
			point p2 = octogon_calc_point(ring, ii+1);
			if (i<9) {
				point l1 = octogon_calc_point(lastring, ii);
				canvas_drawLine(p1.x, p1.y, l1.x, l1.y, color);
			}
			canvas_drawLine(p1.x, p1.y, p2.x, p2.y, color);
		}
		lastring = ring;
	}
	
	for (int i=TUNNEL_OBJECTS-1; i>=0; --i) {
		tobject t = objects[i];
		if (t.z < zTraveled) continue;
		if (t.z > (zTraveled+128)) continue;
		int xt = tunnel_get_radius(t.z, 48);
		int x2 = tunnel_get_radius(t.z, 32);
		
		int rad= x2-xt;
		octogon o = {tunnel_get_center_at(t.z), x2, .924 * x2, .383 * x2};
		point c = octogon_get_edge(o, t.rot);
		if (t.type == POWERUP) {
			if (rad < 2)
			canvas_fillCircle(c.x, c.y, rad, RGB(120,100,160));
			else if (rad < 4) {
				canvas_fillCircle(c.x, c.y, rad, RGB(168,126,229));
			}
			else {
				int rad4 = rad/4;
				canvas_fillCircle(c.x, c.y, rad, RGB(94,71,127));
				canvas_fillCircle(c.x-rad4, c.y-rad4, rad-rad4, RGB(168,126,229));
				canvas_fillCircle(c.x-rad4-rad4, c.y-rad4-rad4, rad4, RGB(209,202,220));
			}
		}
		else if (t.type == VIRUS) {
			uint16_t spike = RGB(200,80,80);
			if (rad < 4) {
				canvas_fillCircle(c.x, c.y, rad, spike);
				canvas_drawPixel(c.x, c.y, spike);
				canvas_drawPixel(c.x-rad, c.y-rad, spike);
				canvas_drawPixel(c.x-rad, c.y+rad, spike);
				canvas_drawPixel(c.x+rad, c.y-rad, spike);
				canvas_drawPixel(c.x+rad, c.y+rad, spike);
			}
			else {
				int rad4 = rad/4;
				canvas_drawLine(c.x-rad, c.y-rad, c.x+rad, c.y+rad, spike);
				canvas_drawLine(c.x+rad, c.y-rad, c.x-rad, c.y+rad, spike);
				canvas_fillCircle(c.x, c.y, rad, spike);
				canvas_fillCircle(c.x-rad4, c.y-rad4, rad-rad4, RGB(180,120,120));
				canvas_fillCircle(c.x-rad4-rad4, c.y-rad4-rad4, rad4, RGB(220,160,160));
				canvas_fillCircle(c.x, c.y, rad4/2, spike);
				canvas_drawLine(c.x-rad, c.y, c.x-rad+rad4, c.y, spike);
				canvas_drawLine(c.x+rad, c.y, c.x+rad-rad4, c.y, spike);
				canvas_drawLine(c.x, c.y-rad, c.x, c.y-rad+rad4, spike);
				canvas_drawLine(c.x, c.y+rad, c.x, c.y+rad-rad4, spike);
				canvas_fillCircle(c.x, c.y+rad, 1, spike);
				canvas_fillCircle(c.x, c.y-rad, 1, spike);
				canvas_fillCircle(c.x+rad, c.y, 1, spike);
				canvas_fillCircle(c.x-rad, c.y, 1, spike);
				canvas_fillCircle(c.x+rad, c.y+rad, 1, spike);
				canvas_fillCircle(c.x-rad, c.y-rad, 1, spike);
				canvas_fillCircle(c.x-rad, c.y+rad, 1, spike);
				canvas_fillCircle(c.x+rad, c.y-rad, 1, spike);
			}
		}
		else {
			canvas_fillCircle(c.x-5, c.y-9, 1, RGB(168,126,229));
			canvas_fillCircle(c.x+2, c.y-5, 1, RGB(168,126,229));
			canvas_fillCircle(c.x+10, c.y-2, 1, RGB(168,126,229));
			canvas_fillCircle(c.x-8, c.y+3, 1, RGB(168,126,229));
			canvas_fillCircle(c.x, c.y+10, 1, RGB(168,126,229));
		}
	}
	
}

void tunnnel_think() {
	
	for (int i=TUNNEL_OBJECTS-1; i>=0; --i) {
		tobject t = objects[i];
		if (t.type == DUST) continue;
		if (t.z < zTraveled) continue;
		if (t.z > (zTraveled+6)) continue;
		int rt = abs(t.rot - player.rotation);
		while (rt > 360) rt -= 360;
		while (rt < 0) rt+= 360;
		if ((rt < 12) || (rt > 348)) {
			if (t.type == POWERUP) {
				//Player picked up ball
				player.fuel += 5;
				objects[i].type = DUST;
			}
			else if (t.type == VIRUS) {
				player.frame=0;
				player.dead = true;
			}
		}
	}
	
	zTraveled++;
	if ((zTraveled/16) > (TUNNEL_LENGTH * 10)) {
		tunnel_generate_path();
		zTraveled = 0;
		++level;
	}
}

void player_draw() {
	float angle = DEG2RAD(player.rotation);
	point p = {96*cos(angle), 96*sin(angle)};
	p.x += 96;
	p.y += 96;
	
	if (player.dead) {
		canvas_drawImage_FromFlash_pt(p.x, p.y, 48, 48, EXPLODE_IMG, (player.frame++ %8 )*48, 0, 384,  RGB(69,78,91));
		return;
	}
	
	int fx = (player.frame++ % 5) * 48;
	int fy = ((player.rotation +157)/45);
	fy = ((fy + 3) % 8) * 48;
	
	canvas_drawImage_FromFlash_pt(p.x, p.y, 48, 48, SHIP_IMG, fx, fy, 48*5,  RGB(255, 0, 255));
	
	
}

bool player_think() {
	if (player.dead) {
		return player.frame < 8;
	}
	else if (player.fuel <=0) {
		player.dead=true;
		player.frame=0;
	}
	
	player.xInput = getTouchLocation();
	int dr = player.xInput - player.rotation;
	if (dr > 180)
	dr -= 360;
	if (dr < -180)
	dr += 360;
	
	if (abs(dr-player.xMomentum)<3)
	player.xMomentum = dr;
	else if (dr < player.xMomentum)
	player.xMomentum-=4;
	else if (dr > player.xMomentum)
	player.xMomentum+=4;
	
	if (abs(player.xMomentum) > MAXMOMENTUM)
	player.xMomentum = (player.xMomentum>0)?MAXMOMENTUM:-MAXMOMENTUM;
	
	player.rotation += player.xMomentum;
	if (player.rotation < -180)
	player.rotation += 360;
	if (player.rotation >= 180)
	player.rotation -= 360;
	
	if ((zTraveled % 8)==0) {
        player.score++;
        player.fuel--;
    }
	
	return true;
}

Scene game_scene_loop(bool init) {
	if (back_event) {
		back_event=false;
		if (player.score > 500) {
			if (newUnlock(UNLOCK_TUNNEL))
				return REWARD;
		}
		return MENU;
	}
	if (init)
		tunnel_init();
	if (!player_think()) {
		canvas_drawText(80, 112, "Game Over", RGB(255,255,255));
	}
	else {
		tunnnel_think();
		tunnnel_think();
		tunnel_draw();
		player_draw();	
		}
	char line[12];
	canvas_fillRect(87, 225, player.fuel/4, 2, RGB(10,200,10));
	sprintf(line, "%3d", player.score);
	canvas_drawText(108, 226, line, RGB(200,200,200));
	canvas_blt();
	return GAME;
}