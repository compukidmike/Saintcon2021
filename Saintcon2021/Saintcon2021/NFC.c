
#include "nfc.h"
#include "main.h"
#include "string.h"


void nfc_init(void){
	uint8_t rxbuff[25] = {};
	uint8_t txbuff[25] = {};
			
	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	delay_ms(0.1);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
	delay_ms(0.1);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	
	gpio_set_pin_level(NFC_CS_PIN, false);
	memcpy(rxbuff, "\x00\x01\x00", 3);
	spi_m_sync_io_readwrite(io, rxbuff, txbuff, 3);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
	gpio_set_pin_level(NFC_CS_PIN, false);
	memcpy(rxbuff, "\x02\xff\xff", 3);
	spi_m_sync_io_readwrite(io, rxbuff, txbuff, 3);
	gpio_set_pin_level(NFC_CS_PIN, true);

	gpio_set_pin_level(NFC_CS_PIN, false);
	memcpy(rxbuff, "\x00\x01\x00", 3);
	spi_m_sync_io_readwrite(io, rxbuff, txbuff, 3);
	gpio_set_pin_level(NFC_CS_PIN, true);	
	
}