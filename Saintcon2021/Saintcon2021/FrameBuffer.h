//
//  FrameBuffer.hpp
//
//  Created by Professor Plum on 12/10/19.
//  Copyright © 2019 Professor Plum. All rights reserved.
//

#ifndef FrameBuffer_hpp
#define FrameBuffer_hpp

#include <stdint.h>

#define WIDTH   240
#define HEIGHT  240

#define RGB(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
//#define RGB(r, g, b) ((((g) & 0x1C) << 11) | ((b & 0xF8) << 5) | ((r) & 0xF8) | (((g) & 0xE0) >> 5))

    
void canvas_clearScreen(uint16_t color);

void canvas_drawPixel(int x, int y, uint16_t color);

void canvas_drawLine(int x1, int y1, int x2, int y2, uint16_t color);
void canvas_drawHorizontalLine(int x1, int y, int x2, uint16_t color);
void canvas_drawVerticalLine(int x, int y1, int y2, uint16_t color);

void canvas_drawImage(int x, int y, int w, int h, const uint16_t *data);
void canvas_drawImage_t(int x, int y, int w, int h, const uint16_t *data, uint16_t tansparent_color);
void canvas_drawImage_p(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch);
void canvas_drawImage_pt(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch, uint16_t tansparent_color);

void canvas_drawImageAlpha(int x, int y, int w, int h, const uint16_t *data, int fx, int fy, int pitch, uint16_t tansparent_color);

void canvas_drawImage_FromFlash(int x, int y, int w, int h, const uint32_t addr);
void canvas_drawImage_FromFlash_p(int x, int y, int w, int h, const uint32_t addr, int fx, int fy, int pitch);
void canvas_drawImage_FromFlash_pt(int x, int y, int w, int h, const uint32_t addr, int fx, int fy, int pitch, uint16_t tansparent_color);

void canvas_drawText(int x, int y, const char* text, uint16_t color);
void canvas_drawChar(int x, int y, char c, uint16_t color);

void canvas_drawMiniWindow(int minx, int maxx, int y, char* str, int xoff, uint16_t color);

void canvas_fillCircle(int x, int y, int radius, uint16_t color);

void canvas_fillRect(int x, int y, int w, int h, uint16_t color);

void canvas_floodfill(int x, int y, uint16_t color, uint16_t old_color);

void canvas_drawRect(int x, int y, int w, int h, uint16_t color);

void canvas_mask(int px, int py, int rad1, int rad2, int rad3);

void canvas_drawBitmask(int x, int y, int w, int h, const uint8_t *data, uint16_t color, float rotation);

void canvas_blt();


#endif /* FrameBuffer_hpp */
