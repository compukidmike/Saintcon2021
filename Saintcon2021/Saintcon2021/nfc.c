
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


 	delay_ms(1);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
 	delay_ms(1);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	delay_ms(10);

	nfc_test();
	
	uint8_t rxbuff[20] = {};
	uint8_t txbuff[20] = {};
// 	uint8_t cmd;
	bool validFrame = false;
	
	nfc_comm(rxbuff, txbuff, "\x00\x02\x02\x12\x0a", 5, true);

	nfc_comm(rxbuff, txbuff, "\x00\x09\x03\x68\x00\x04", 6, true);
	nfc_comm(rxbuff, txbuff, "\x00\x09\x04\x68\x01\x04\x2f", 7, true);

	//nfc_comm(rxbuff, txbuff, "\x00\x09\x04\x3A\x00\x58\x04", 7, true);
	//nfc_comm(rxbuff, txbuff, "\x00\x09\x03\x68\x00\x04", 6, true);
	//nfc_comm(rxbuff, txbuff, "\x00\x08\x03\x69\x01\x00", 6, true);
	nfc_comm(rxbuff, txbuff, "\0\xd\xB\x44\0\0\0SAINTCN", 14, true);
	nfc_comm(rxbuff, txbuff, "\0\x0d\x1\x84", 4, true);
	//nfc_comm(rxbuff, txbuff, "\0\x0d\0", 3, true);

	//nfc_comm(rxbuff, txbuff, "\0\x0D\x0B\x44\x03\x20\x88\x02\x51\x74\x4A\xEF\x22\x80", 14, true);
	
	nfc_comm(rxbuff, txbuff, "\x00\x0D\x02\0\0", 5, true);


	
	while(!validFrame){ 

		nfc_comm(rxbuff, txbuff, "\x00\x05\x00", 3, true);

		if(rxbuff[1] == 0){
			nfc_poll();
			nfc_read(rxbuff);
			if(rxbuff[1] ==0x80){
				char data[20] = {0};
				for(int i = 0; i < (rxbuff[2]+1); i++){
					if((rxbuff[2+i] & 0x0F) == rxbuff[2+i]){
						data[i*2] = '0';
						itoa(rxbuff[2+i],&data[1+(2*i)],16);
					}else{
						itoa(rxbuff[2+i],&data[2*i],16);
					}
				}

				platformLog(data);
//  			if(rxbuff[1] == 0x80 && rxbuff[2] == 0x02 && rxbuff[3] == 0x26){
//  				//nfc_comm(&rxbuff[15], txbuff, "\x00\x06\x0c\0\x1\x2\x3\x4\x5\x6\x7\x8\x9\xa\0", 16, true);
//  				nfc_comm(&rxbuff[15], txbuff, "\0\x6\x3\0\x44\x8", 6, true);
//  				rxbuff[0] = 0x50;
  			}
			
		}
	}
	while(1);
	

}

void nfc_poll(){

// 	uint8_t rbuff = 0;
// 	uint8_t tbuff = 3;
// 	gpio_set_pin_level(NFC_CS_PIN, false);
// 	
// 	while( (rbuff & 0xf8) != 8){
// 		delay_ms(1);
// 		spi_m_sync_io_readwrite(io, &rbuff, &tbuff, 1);	
// 	}
// 	gpio_set_pin_level(NFC_CS_PIN, true);
	while(gpio_get_pin_level(NFC_IRQ_OUT_PIN));
}

bool nfc_test(){
	uint8_t rxbuff[20] = {0};
	uint8_t txbuff[3] = {};
			
	nfc_comm(rxbuff, txbuff, "\x00\x01\x00", 3, true);
	
	if (rxbuff[2] == 15){
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
	//if(rxbuff[1] == 0){
		size = rxbuff[2];
		for(uint8_t i = 0; i < size; i++){
			spi_m_sync_io_readwrite(io, &buff, txbuff, 1);
			rxbuff[3+i] = buff;
		}
	//}
	gpio_set_pin_level(NFC_CS_PIN, true);
	return size;
}

void nfc_reset(){
	uint8_t tx[2] = {};
	uint8_t buff[2] = {};
	
	// send echo just to make sure card emulation has exited
 	memcpy(tx, "\x00\x55", 2);
 	gpio_set_pin_level(NFC_CS_PIN, false);
 	spi_m_sync_io_readwrite(io, buff, tx, 2);
 	gpio_set_pin_level(NFC_CS_PIN, true);
	
	//send reset control bit
	memcpy(tx, "\x01", 1);
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, buff, tx, 1);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
}

void nfc_comm(uint8_t * rx, uint8_t * tx, char * command, uint8_t size, bool read){
	memcpy(tx, command, size);
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, rx, tx, size);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
	if(read){
		nfc_poll();
		nfc_read(rx);
	}
}