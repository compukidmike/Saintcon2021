/*
 * ILI9331.h
 *
 * Created: 8/16/2021 9:43:16 AM
 *  Author: Professor Plum
 */ 


#ifndef ILI9331_H_
#define ILI9331_H_

#define LCD_WR	16
#define LCD_CS	31
#define LCD_RS	30
#define LCD_BL	17
#define LCD_RST 22

#define RGB(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_DrawImage(int x, int y, int w, int h, uint16_t *data);
void LCD_FillRect(int x, int y, int w, int h, uint16_t color);
void LCD_DrawChar(char *text, int x, int y, uint16_t color);
void LCD_DrawLine (int x0, int y0, int x1, int y1, uint16_t color);
void LCD_Init();
void LCD_Reset();
void LCD_Sleep();
void LCD_Wake();


#endif /* ILI9331_H_ */
