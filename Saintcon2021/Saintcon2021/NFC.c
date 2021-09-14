
#include "nfc.h"
#include "main.h"
#include "string.h"


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


// 	delay_ms(0.2);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
// 	delay_ms(0.2);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	delay_ms(20);

	nfc_test();
	
	uint8_t rxbuff[20] = {};
	uint8_t txbuff[20] = {};
	uint8_t size;	
	size = nfc_comm(rxbuff, txbuff, "\x00\x02\x02\x12\x08", 5);
	
	size = nfc_comm(rxbuff, txbuff, "\x00\x05\x00", 3);

	if(size == 0){		poll_nfc();		size = nfc_read(rxbuff);	}	
}

void poll_nfc(){

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
	uint8_t rxbuff[20] = {};
	uint8_t txbuff[3] = {};
			
	uint8_t size = nfc_comm(rxbuff, txbuff, "\x00\x01\x00", 3);
	
	if (size == 15){
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
	spi_m_sync_io_readwrite(io, &buff, &tx, 1);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
}

uint8_t nfc_comm(uint8_t * rx, uint8_t * tx, uint8_t * command, uint8_t size){
	memcpy(tx, command, size);
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, rx, tx, size);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
	poll_nfc();
	
	return nfc_read(rx);	
}