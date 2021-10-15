/*
 * platform.c
 *
 */ 
#include "main.h"
#include "platform.h"
#include "ILI9331.h"
#include "FrameBuffer.h"
#include "hal_delay.h"

extern uint32_t millis();

void platformGpioSet(uint32_t inport, uint32_t inpin){
	gpio_set_pin_level(inpin, true);
}

void platformGpioClear(uint32_t inport, uint32_t inpin){
	gpio_set_pin_level(inpin, false);
}

void platformDelay(uint32_t ms){
	delay_ms(ms);
}

void platformSpiSelect(void){
	gpio_set_pin_level(NFC_CS_PIN, false);
}

void platformSpiDeselect(void){
	gpio_set_pin_level(NFC_CS_PIN, true);
	delay_ms(1);
}

void platformErrorHandle(void){
	//while(1);
}

void platformSpiTxRx(uint8_t *txBuf, uint8_t *rxBuf, const uint16_t len){
	spi_m_sync_io_readwrite(io, rxBuf, txBuf, len);
}

bool platformGpioIsHigh(uint32_t inport, uint32_t inpin){
	return gpio_get_pin_level(inpin);
}

bool platformGpioIsLow(uint32_t inport, uint32_t inpin){
	return !gpio_get_pin_level(inpin);
}

uint32_t platformTimerCreate(uint32_t timeout){
	//return millis() + timeout;
	return timeout;
}

bool platformTimerIsExpired(uint32_t timer){
	//if(millis() > timer){
		//return true;
		
	//} else {
		return false;
	//}
}

void platformTimerDestroy(uint32_t timer){
	
}

void platformProtectWorker(void){
	
}

void platformUnprotectWorker(void){
	
}

void platformLog(char *data){
	//NO_WARNING(data);
	//canvas_fillRect(80,140,100,20,RGB(10,10,200));
	//canvas_drawText(80,140,data,RGB(255,255,255));
	//canvas_blt();
}
