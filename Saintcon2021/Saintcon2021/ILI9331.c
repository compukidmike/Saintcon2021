/*
 * ILI9331.cpp
 *
 *  Created on: May 27, 2020
 *      Author: compukidmike
 */
#include <atmel_start.h>
#include <stdint.h>
#include "ILI9331.h"

#define gpio_sethigh(pin)	(REG_PORT_OUTSET1 = 1 << (pin))
//(gpio_set_pin_level(pin, true) )
#define gpio_setlow(pin)	(REG_PORT_OUTCLR1 = 1 << (pin))
//gpio_set_pin_level(pin, false) )


void LCD_Select(){
	gpio_setlow(LCD_CS);
}

void LCD_Deselect(){
	//gpio_sethigh(LCD_CS);
}

void LCD_WriteData(uint16_t data){

	//gpio_sethigh(RD);
	
	/*gpio_set_port_level(GPIO_PORTA, 0xFF, false);
	gpio_set_port_level(GPIO_PORTA, data & 0xFF, true);
	gpio_set_port_level(GPIO_PORTB, 0xFF, false);
	gpio_set_port_level(GPIO_PORTB, (data>>8) & 0xFF, true);*/
	REG_PORT_OUTCLR0 = 0xFF;
	REG_PORT_OUTSET0 = data & 0xFF;
	REG_PORT_OUTCLR1 = 0xFF;
	REG_PORT_OUTSET1 = (data>>8) & 0xFF;

	gpio_setlow(LCD_WR);
	delay_us(10);
	gpio_sethigh(LCD_WR);
}

void LCD_WriteCommand(uint16_t command){

	gpio_setlow(LCD_RS);
	//gpio_sethigh(LCD_RD);

	/*gpio_set_port_level(GPIO_PORTA, 0xFF, false);
	gpio_set_port_level(GPIO_PORTA, command & 0xFF, true);
	gpio_set_port_level(GPIO_PORTB, 0xFF, false);
	//gpio_set_port_level(GPIO_PORTB, (command>>8) & 0xFF, true);*/
	REG_PORT_OUTCLR0 = 0xFF;
	REG_PORT_OUTSET0 = command & 0xFF;
	REG_PORT_OUTCLR1 = 0xFF;
	/*REG_PORT_OUTSET1 = (command>>8) & 0xFF;*/

	gpio_setlow(LCD_WR);
	delay_us(10);
	gpio_sethigh(LCD_WR);
	
	gpio_sethigh(LCD_RS);
}

void LCD_DrawPixel(uint8_t x, uint8_t y, uint16_t color){

	uint16_t AD0_7, AD8_16;
	uint32_t address;
	address = y << 8;
	address |= x;
	AD0_7 = address & 0x00FF;
	AD8_16 = (address >> 8) & 0x01FF;

	LCD_Select();
	LCD_WriteCommand(0x0020);
	LCD_WriteData(AD0_7);
	LCD_Deselect();
	LCD_Select();
	LCD_WriteCommand(0x0021);
	LCD_WriteData(AD8_16);
	LCD_Deselect();

	LCD_Select();
	LCD_WriteCommand(0x0022);
	LCD_WriteData(color);
	LCD_Deselect();
}

void LCD_DrawImage(int x, int y, int w, int h, uint16_t *data) {
	uint16_t AD0_7, AD8_16;
	uint32_t address;
	address = y << 8;
	address |= x;
	AD0_7 = address & 0x00FF;
	AD8_16 = (address >> 8) & 0x01FF;

	LCD_Select();
	LCD_WriteCommand(0x0020);
	LCD_WriteData(AD0_7);
	LCD_WriteCommand(0x0021);
	LCD_WriteData(AD8_16);

	LCD_WriteCommand(0x0022);
	for (int i=0; i<w*h; ++i)
		LCD_WriteData(data[i]);
	LCD_Deselect();
}

void LCD_FillRect(int x, int y, int w, int h, uint16_t color) {
	uint16_t AD0_7, AD8_16;
	uint32_t address;
	address = y << 8;
	address |= x;
	AD0_7 = address & 0x00FF;
	AD8_16 = (address >> 8) & 0x01FF;

	LCD_Select();
	LCD_WriteCommand(0x0020);
	LCD_WriteData(AD0_7);
	LCD_WriteCommand(0x0021);
	LCD_WriteData(AD8_16);

	LCD_WriteCommand(0x0022);
	for (int i=0; i<w*h; ++i)
		LCD_WriteData(color);
	LCD_Deselect();
}

/*void LCD_DrawChar(char *text, int x, int y, uint16_t color){
	int textlength = strlen(text);;
	for(int n=0; n<textlength; n++){
		for(int i=0; i<5; i++){
			for (int j=0; j<7; j++){
				if((font5x7[(((int)text[n])-32)*5+i]>>j)&1){
					LCD_DrawPixel(x+n*6+i,y+j,color);
				}
			}
		}
	}
}*/

void LCD_DrawLine (int x0, int y0, int x1, int y1, uint16_t color){
  int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2; /* error value e_xy */

  for (;;){  /* loop */
    LCD_DrawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

void LCD_Init(){
	
	REG_PORT_DIR0 |= 0xFF;
	REG_PORT_DIR1 |= 0xC04300FF;
	

	LCD_Reset();

	gpio_sethigh(LCD_CS);
	gpio_sethigh(LCD_RS);
	//gpio_sethigh(LCD_RD);
	gpio_sethigh(LCD_WR);

	//************* Start Initial Sequence ===========================//
	LCD_Select();LCD_WriteCommand(0x00E7);LCD_WriteData(0x1014);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0001);LCD_WriteData(0x0100);LCD_Deselect();         // set SS bit
	LCD_Select();LCD_WriteCommand(0x0002);LCD_WriteData(0x0200);LCD_Deselect();         // set 1 line inversion
	//LCD_Select();LCD_WriteCommand(0x0003);LCD_WriteData(0x1030);LCD_Deselect();         // set GRAM write direction and BGR=1
	LCD_Select();LCD_WriteCommand(0x0008);LCD_WriteData(0x0207);LCD_Deselect();         // set the back porch and front porch
	LCD_Select();LCD_WriteCommand(0x0009);LCD_WriteData(0x0000);LCD_Deselect();         // set non-display area refresh cycle ISC[3:0]
	LCD_Select();LCD_WriteCommand(0x000A);LCD_WriteData(0x0000);LCD_Deselect();                // FMARK function
	LCD_Select();LCD_WriteCommand(0x000C);LCD_WriteData(0x0000);LCD_Deselect();        // RGB interface setting
	LCD_Select();LCD_WriteCommand(0x000D);LCD_WriteData(0x0000);LCD_Deselect();                // Frame marker Position
	LCD_Select();LCD_WriteCommand(0x000F);LCD_WriteData(0x0000);LCD_Deselect();        // RGB interface polarity
	//*************Power On sequence =======================================//
	LCD_Select();LCD_WriteCommand(0x0010);LCD_WriteData(0x0000);LCD_Deselect();        // SAP,BT[3:0], AP, DSTB, SLP, STB
	LCD_Select();LCD_WriteCommand(0x0011);LCD_WriteData(0x0007);LCD_Deselect();        // DC1[2:0], DC0[2:0], VC[2:0]
	LCD_Select();LCD_WriteCommand(0x0012);LCD_WriteData(0x0000);LCD_Deselect();        // VREG1OUT voltage
	LCD_Select();LCD_WriteCommand(0x0013);LCD_WriteData(0x0000);LCD_Deselect();        // VDV[4:0] for VCOM amplitude
	delay_ms(200);                                                      // Dis-charge capacitor power voltage
	LCD_Select();LCD_WriteCommand(0x0010);LCD_WriteData(0x1690);LCD_Deselect();        // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_Select();LCD_WriteCommand(0x0011);LCD_WriteData(0x0007);LCD_Deselect();        // DC1[2:0], DC0[2:0], VC[2:0]
	delay_ms(50);                                                             // Delay 50ms
	LCD_Select();LCD_WriteCommand(0x0012);LCD_WriteData(0x000C);LCD_Deselect();        // Internal reference voltage= Vci;
	delay_ms(50);                                                             // Delay 50ms
	LCD_Select();LCD_WriteCommand(0x0013);LCD_WriteData(0x0700);LCD_Deselect();        // Set VDV[4:0] for VCOM amplitude
	LCD_Select();LCD_WriteCommand(0x0029);LCD_WriteData(0x0005);LCD_Deselect();        // Set VCM[5:0] for VCOMH
	LCD_Select();LCD_WriteCommand(0x002B);LCD_WriteData(0x000D);LCD_Deselect();      // Set Frame Rate
	delay_ms(50);                                                        // Delay 50ms
	LCD_Select();LCD_WriteCommand(0x0020);LCD_WriteData(0x0000);LCD_Deselect();        // GRAM horizontal Address
	LCD_Select();LCD_WriteCommand(0x0021);LCD_WriteData(0x0000);LCD_Deselect();        // GRAM Vertical Address
	// ----------- Adjust the Gamma    Curve ----------//
	LCD_Select();LCD_WriteCommand(0x0030);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0031);LCD_WriteData(0x0207);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0032);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0035);LCD_WriteData(0x0007);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0036);LCD_WriteData(0x0508);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0037);LCD_WriteData(0x0707);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0038);LCD_WriteData(0x0005);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0039);LCD_WriteData(0x0707);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x003C);LCD_WriteData(0x0202);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x003D);LCD_WriteData(0x0A09);LCD_Deselect();
	//------------------ Set GRAM area ---------------//
	LCD_Select();LCD_WriteCommand(0x0050);LCD_WriteData(0x0000);LCD_Deselect();         // Horizontal GRAM Start Address
	LCD_Select();LCD_WriteCommand(0x0051);LCD_WriteData(0x00EF);LCD_Deselect();        // Horizontal GRAM End Address
	LCD_Select();LCD_WriteCommand(0x0052);LCD_WriteData(0x0000);LCD_Deselect();         // Vertical GRAM Start Address
	LCD_Select();LCD_WriteCommand(0x0053);LCD_WriteData(0x00EF);LCD_Deselect();         // Vertical GRAM Start Address
	LCD_Select();LCD_WriteCommand(0x0060);LCD_WriteData(0x2700);LCD_Deselect();        // Gate Scan Line
	LCD_Select();LCD_WriteCommand(0x0061);LCD_WriteData(0x0001);LCD_Deselect();         // NDL,VLE, REV
	LCD_Select();LCD_WriteCommand(0x006A);LCD_WriteData(0x0000);LCD_Deselect();                  // set scrolling line
	//-------------- Partial Display Control ---------//
	LCD_Select();LCD_WriteCommand(0x0080);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0081);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0082);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0083);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0084);LCD_WriteData(0x0000);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0085);LCD_WriteData(0x0000);LCD_Deselect();
	//-------------- Panel Control -------------------//
	LCD_Select();LCD_WriteCommand(0x0090);LCD_WriteData(0x0010);LCD_Deselect();
	LCD_Select();LCD_WriteCommand(0x0092);LCD_WriteData(0x0600);LCD_Deselect();

	LCD_Select();LCD_WriteCommand(0x0007);LCD_WriteData(0x0133);LCD_Deselect();         // 262K color and display ON
}

void LCD_Reset(){

	gpio_setlow(LCD_RST);
	delay_ms(10);
	gpio_sethigh(LCD_RST);
	delay_ms(50);
}

void LCD_Sleep(){

}

void LCD_Wake(){

}


