
#include "nfc.h"
#include "main.h"
#include "string.h"
#include "platform.h"
#include <stdlib.h>

bool nfc_init(void){
	
	spi_m_sync_get_io_descriptor(&SPI_1, &io);

	spi_m_sync_enable(&SPI_1);

	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	
	nfc_reset();

	delay_ms(2); //Occasionally hangs without a delay here. Might be able to shorten it.
// 	delay_ms(0.2);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
// 	delay_ms(0.2);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	delay_ms(20);

	nfc_test();
	
	uint8_t rxbuff[20] = {};
	uint8_t txbuff[20] = {};
	uint8_t size;	bool validFrame = false;
	
	while(!validFrame){ 
		size = nfc_comm(rxbuff, txbuff, "\x00\x02\x02\x12\x08", 5);
		
		size = nfc_comm(rxbuff, txbuff, "\x00\x05\x00", 3);

		if(size == 0){			nfc_poll();			size = nfc_read(rxbuff);			char data[2] = {0};			itoa(rxbuff[1],data,16);			platformLog(data);			if(rxbuff[1] == 0x80) validFrame = true;		}	}	while(1);	
}

void nfc_poll(){

	uint8_t rbuff = 0;
	uint8_t tbuff = 3;
	gpio_set_pin_level(NFC_CS_PIN, false);
	while( (rbuff & 0xf8) != 8){
		delay_ms(1);
		spi_m_sync_io_readwrite(io, &rbuff, &tbuff, 1);	
	}
	gpio_set_pin_level(NFC_CS_PIN, true);
}

bool nfc_test(){
	uint8_t rxbuff[20] = {0};
	uint8_t txbuff[3] = {};
			
	uint8_t size = nfc_comm(rxbuff, txbuff, "\x00\x01\x00", 3);
	
	if (size == 15){
		platformLog((char*)rxbuff+3);
		return true;
	}
	return false;
	
}

uint8_t nfc_read(uint8_t* rxbuff){
	uint8_t txbuff[3] = {2};
	uint8_t size = 0;
	uint8_t buff = 0;
	
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, rxbuff, txbuff, 3);
	if(rxbuff[1] == 0){
		size = rxbuff[2];
		for(uint8_t i = 0; i < size; i++){
			spi_m_sync_io_readwrite(io, &buff, txbuff, 1);
			rxbuff[3+i] = buff;
		}
	}
	gpio_set_pin_level(NFC_CS_PIN, true);
	return size;
}

void nfc_reset(){
	uint8_t tx[2] = {};
	uint8_t buff[2] = {};
	
 	memcpy(tx, "\x00\x55", 2);
 	gpio_set_pin_level(NFC_CS_PIN, false);
 	spi_m_sync_io_readwrite(io, buff, tx, 2);
 	gpio_set_pin_level(NFC_CS_PIN, true);
	
	memcpy(tx, "\x01", 1);
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, buff, tx, 1);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
}

uint8_t nfc_comm(uint8_t * rx, uint8_t * tx, char * command, uint8_t size){
	memcpy(tx, command, size);
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, rx, tx, size);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
	nfc_poll();
	
	return nfc_read(rx);	
}