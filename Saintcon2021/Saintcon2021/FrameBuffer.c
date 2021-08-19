//
//  FrameBuffer.cpp
//  2dAEngine
//
//  Created by Professor Plum on 12/10/19.
//  Copyright © 2019 Professor Plum. All rights reserved.
//

#include "FrameBuffer.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include "font8x16.h"
#include "ILI9331.h"
#include "flash.h"



#ifdef    SDL
 #include <SDL.h>
 extern SDL_Window* window;
 extern SDL_Renderer* renderer;
#endif

#if !defined(max)
 #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#if !defined(min)
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

uint16_t frame[WIDTH * HEIGHT];

void canvas_clearScreen(uint16_t color) {
	uint32_t cc = (color << 16) + color;
    for (int i=0; i<(WIDTH*HEIGHT/2); ++i)
        ((uint32_t*)frame)[i] = cc;
}

void canvas_drawPixel(int x, int y, uint16_t color) {
    if ((x<0) || (y<0) || (x>=WIDTH) || (y>=HEIGHT))
        return;
    frame[y*WIDTH+x] = color;
}

void canvas_drawHorizontalLine(int x1, int y, int x2, uint16_t color) {
    if ((y<0) || (y>=HEIGHT))
        return;
    int s1 = max(min(x1, x2), 0);
    int s2 = min(max(x1, x2), WIDTH-1);
    for (int i=s1; i<=s2; ++i)
        frame[y*WIDTH+i]=color;
}
void  canvas_drawVerticalLine(int y1, int x, int y2, uint16_t color) {
    if ((x<0) || (x>=WIDTH))
        return;
    int s1 = max(min(y1, y2), 0);
    int s2 = min(max(y1, y2), HEIGHT-1);
    for (int i=s1; i<=s2; ++i)
        frame[i*WIDTH+x]=color;
}

void canvas_drawImage(int x, int y, int w, int h, const uint16_t *data) {
    int idx=0;
    for (int j = y; j<y+h; ++j){
        if ((y>=0) && (y<HEIGHT))
            for (int i=x; i<x+w; ++i) {
            if ((i>=0) && (i<HEIGHT))
                frame[j*WIDTH+i] = data[idx++];
            else
                ++idx;
            }
        else
            idx+=w;
    }
}

void canvas_drawImage_t(int x, int y, int w, int h, const uint16_t *data, uint16_t tansparent_color) {
    int idx=0;
    for (int j = y; j<y+h; ++j){
        if ((j>=0) && (j<HEIGHT))
            for (int i=x; i<x+w; ++i) {
                if ((i>=0) && (i<HEIGHT)) {
                    uint16_t c = data[idx++];
                    if (c != tansparent_color)
                        frame[j*WIDTH+i] = c;
                }
            else
                ++idx;
            }
        else
            idx+=w;
    }
}

void canvas_drawImage_p(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch) {
    if ((x+w) >= (WIDTH-1))
        w = (WIDTH-1-x);
    if ((y+h) >= (HEIGHT-1))
        h = (HEIGHT-1-y);
    
    for (int j=max(0, -y); j<h; ++j) {
        for (int i=max(0, -x); i<w; ++i) {
            frame[(j+y)*WIDTH+i+x] = data[pitch*(fy+j)+i+fx];
        }
    }
}

void canvas_drawImage_pt(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch, uint16_t tansparent_color) {
    if ((x+w) >= (WIDTH-1))
        w = (WIDTH-1-x);
    if ((y+h) >= (HEIGHT-1))
        h = (HEIGHT-1-y);
    
    for (int j=max(0, -y); j<h; ++j) {
        for (int i=max(0, -x); i<w; ++i) {
            uint16_t c = data[pitch*(fy+j)+i+fx];
            if (c != tansparent_color)
                frame[(j+y)*WIDTH+i+x] = c;
        }
    }
}

void canvas_drawImage_FromFlash(int x, int y, int w, int h, const uint16_t *data){
	int ow=w;
	if ((x+w) >= (WIDTH-1))
	w = (WIDTH-1-x);
	if ((y+h) >= (HEIGHT-1))
	h = (HEIGHT-1-y);
	for (int j=max(0, -y); j<h; ++j) {
		flash_read(data+ow*j, &frame[(j+y)*WIDTH+x], w*sizeof(uint16_t));
	}
}

void canvas_drawImage_FromFlash_pt(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch, uint16_t tansparent_color) {
	if ((x+w) >= (WIDTH-1))
	w = (WIDTH-1-x);
	if ((y+h) >= (HEIGHT-1))
	h = (HEIGHT-1-y);
	
	for (int j=max(0, -y); j<h; ++j) {
		uint16_t row[w];
		flash_read(data+pitch*(fy+j)+fx, row, w*sizeof(uint16_t));
		for (int i=max(0, -x); i<w; ++i) {
			uint16_t c = row[i];
			if (c != tansparent_color)
			frame[(j+y)*WIDTH+i+x] = c;
		}
	}
}

void canvas_drawImageAlpha(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch, uint16_t tansparent_color) {
    if ((x+w) >= (WIDTH-1))
        w = (WIDTH-1-x);
    if ((y+h) >= (HEIGHT-1))
        h = (HEIGHT-1-y);

    for (int j=max(0, -y); j<h; ++j) {
        for (int i=max(0, -x); i<w; ++i) {
            uint16_t c = data[pitch*(fy+j)+i+fx];
            if (c != tansparent_color) {
                uint16_t c2 = (frame[(j+y)*WIDTH+i+x] >> 1) & 0xFBEF;
                c = (c >> 1) & 0xFBEF;
                frame[(j+y)*WIDTH+i+x] = c + c2;
            }
        }
    }
}



void canvas_fillRect(int x, int y, int w, int h, uint16_t color){
    if (x<0) {
        w+=x;
        x=0;
    }
    if (y<0) {
        h+=y;
        y=0;
    }
    for (int j = y; j<min(y+h, HEIGHT-1); ++j)
        for (int i=x; i<min(x+w, WIDTH-1); ++i)
            frame[j*WIDTH+i] = color;
}

void canvas_drawRect(int x, int y, int w, int h, uint16_t color) {
    canvas_drawHorizontalLine(x,y+h,x+w,color);
    canvas_drawHorizontalLine(x,y+h,x+w,color);
    canvas_drawVerticalLine(x,y,y+h,color);
    canvas_drawVerticalLine(x+w,y,y+h,color);
}

void canvas_drawLine(int x1, int y1, int x2, int y2, uint16_t color) {
    if (x1 == x2) {
        canvas_drawVerticalLine(y1, x1, y2, color);
        return;
    }
    if (y1 == y2) {
        canvas_drawHorizontalLine(x1, y1, x2, color);
        return;
    }
    
    int dx = abs(x2-x1), sx = x1<x2?1:-1;
    int dy = abs(y2-y1), sy = y1<y2?1:-1;
    
    int err = (dx>dy?dx:-dy)/2, e2;
    int x=x1, y=y1;
    for(;;){
      if ((x >= 0) && (y >=0) && (x < WIDTH) && (y <HEIGHT))
          frame[x+y*WIDTH]=color;
      if (x==x2 && y==y2) break;
      e2 = err;
      if (e2 >-dx) { err -= dy; x += sx; }
      if (e2 < dy) { err += dx; y += sy; }
    }
}

void canvas_fillCircle(int x, int y, int radius, uint16_t color){
    int rad2=radius*radius;
    for (int j=y-radius;j<=y+radius; ++j) {
        int yd = abs(y-j);
        int radx2 = rad2 - yd*yd;
        if (j<0) continue;
        if (j>=HEIGHT) return;
        for (int i=x-radius;i<=x+radius; ++i) {
            int xd = abs(x-i);
            int dist2 = xd*xd;
            if (i<0) continue;
            if (i>=WIDTH) break;
            if (dist2<=radx2)
                frame[i+j*WIDTH] = color;
        }
    }

}

void canvas_floodfill(int x, int y, uint16_t color, uint16_t old_color) {
    if (color == old_color)
        return;
    if ((x<0) || (y<0) || (x>=WIDTH) || (y>=HEIGHT))
        return;
    if (frame[x+y*WIDTH] != old_color)
        return;
    
    frame[x+y*WIDTH] = color;
    canvas_floodfill(x+1, y, color, old_color);
    canvas_floodfill(x-1, y, color, old_color);
    canvas_floodfill(x, y+1, color, old_color);
    canvas_floodfill(x, y-1, color, old_color);
}

void canvas_drawText(int x, int y, const char* text, uint16_t color) {
    int l = (int)strlen(text);
    for (int i=0; i<l; ++i)
        canvas_drawChar(x + i*8, y, text[i], color);
}

void canvas_drawChar(int x, int y, char c, uint16_t color) {
    for (int j=0; j<16; ++j) {
        uint8_t b = font8x16[c-0x20][j];
        for (int i=0; i<8; ++i) {
            if ((b>>i)&1)
                canvas_drawPixel(x+i, y+j, color);
        }
    }
}

void canvas_drawBitmask(int x, int y, int w, int h, const uint8_t *data, uint16_t color, float angle) {
	int cx = w/2, cy = h/2;
    int wb = w/8;
    /*
     * Fast and cheap rotation (only do the math if the bit is white)
     */
    double cosa = cos(angle);
    double sina = sin(angle);
    for (int i=0; i< wb; ++i)
        for (int j=0; j<h; ++j) {
            uint8_t b = data[i+j*wb];
            if (b) {
                for (int k=0; k<8; ++k) {
                    int s = (b >> k) & 1;
                    if (s) {
						int ax =i*8+k - cx;
						int ay = j - cy;
                        double nx = cosa*ax - sina*ay + cx;
                        double ny = sina*ax + cosa*ay + cy;
                        nx = i*8+k;
                        ny = j;
                        canvas_drawPixel(x+nx, y+ny, color);
                        //drawPixel(x+roundf(nx), y+roundf(ny), color);
                    }
                }
            }
        }//*/

    /*
     *Slower but more correct implementation, use if we have spare cycles
     *
    float cosb = cosf(-angle);
    float sinb = sinf(-angle);
    for (int j=0; j<h; ++j)
        for (int i=0; i<w; ++i) {
            Point a(i,j);
            a-=center;
            Point n(cosb*a.x - sinb*a.y, sinb*a.x + cosb*a.y);
            n+=center;
            if ((n.x>=0) && (n.x<w) && (n.y>=0) && (n.y<h)){
                uint8_t b = data[n.x/8 + n.y*wb];
                int s = (b>> (n.x%8)) & 1;
                if (s)
                    drawPixel(x+i, y+j, color);
            }
        }//*/
}

void canvas_blt() {
    
    //Replace this with device specific routine
    
#ifndef SDL

    LCD_DrawImage(0, 0, 240, 240, frame);


#else
    //this just for artifical circle,
    for (int x=0; x<WIDTH; ++x) {
        int xx =(120-x);
        for (int y=0; y<HEIGHT; ++y) {
            int yy = (120-y);
            if ((xx*xx + yy*yy) > 14400)
                frame[x+y*WIDTH] = RGB(100,100,0);
        }
    }
    
    void* pixels;
    int pitch;

    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    SDL_LockTexture(tex, NULL, &pixels, &pitch);
    memcpy(pixels, frame, WIDTH*HEIGHT*sizeof(uint16_t));
    SDL_UnlockTexture(tex);
    SDL_RenderCopy(renderer, tex, NULL, NULL);
    SDL_DestroyTexture(tex);
    SDL_RenderPresent(renderer);
#endif
}


