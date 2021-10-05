
#include "nfc.h"
#include "main.h"
#include "string.h"
#include "platform.h"
#include "stdlib.h"

char UID[] = "SAINTCN";

bool nfc_init(void){
	
	spi_m_sync_get_io_descriptor(&SPI_1, &io);

	spi_m_sync_enable(&SPI_1);

	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	
	gpio_set_pin_level(NFC_CS_PIN, true);
	delay_ms(1);
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
	
	//Card emulation mode
	nfc_comm(rxbuff, txbuff, "\x00\x02\x02\x12\x08", 5, true);

	// Boost sensitivity
	nfc_comm(rxbuff, txbuff, "\x00\x09\x03\x68\x00\x04", 6, true);	
	nfc_comm(rxbuff, txbuff, "\x00\x09\x04\x68\x01\x04\x15", 7, true);

	// Setup chip to handle collision commands
	//nfc_comm(rxbuff, txbuff, "\0\x0D\x0B\x44\x00\x00\x88\x02\x80\x74\x4A\xEF\x22\x80", 14, true);
	char cmd[14] = {0,0x0D,0x0B,0x44,0x00,0x00,0x88};
	memcpy(&cmd[7], UID, strlen(UID));
	nfc_comm(rxbuff, txbuff, cmd, sizeof(cmd), true);

	nfc_comm(rxbuff, txbuff, "\0\x0d\x1\x1", 4, true);
	char tag_buff[200] = {0};
	char ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','.','o','r','g','/'};

	ndef_vcard(tag_buff, "test", "test@example.com");
	//memset(tag_buff, 0, sizeof(tag_buff));
	//ndef_well_known(tag_buff, ndef_data, sizeof(ndef_data));
	
	
	while(!validFrame){ 

		nfc_comm(rxbuff, txbuff, "\x00\x05\x00", 3, true);

		if(rxbuff[1] == 0){
			nfc_poll();
			nfc_read(rxbuff);
			if(rxbuff[1] == 0x80){
				if(rxbuff[3] == 0x30){
					char buff[] = {0,6,5,0,0,0,0,0x28};
					if(rxbuff[4] < (sizeof(tag_buff)/4)){
						memcpy(&buff[3], &tag_buff[(rxbuff[4])*4], 4);
						//buff[3] = sizeof(ndef_buff)
// 					}else if(rxbuff[4] == (ndef_size/4)+4){
// 						memcpy(&buff[3], &ndef_buff[(rxbuff[4]-4)*4], ndef_size%4);
					}
					
					nfc_comm(&rxbuff[10], txbuff, buff, 8, true);
				}else{
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
				}
  			}
			
		}
	}
	while(1);
	

}

void init_tag(char * buff){
	memcpy(buff, UID, 3);
	memcpy(&buff[4], &UID[3], 4);
	buff[3] = 0x88^UID[0] ^UID[1]^UID[2];
	buff[8] = UID[3]^UID[4]^UID[5]^UID[6];
	buff[10] = 0xFF;
	buff[11] = 0xFF;
}

void ndef_vcard(char * tag_buff, char * fn, char* email){
	init_tag(tag_buff);
	uint8_t header_len = strlen(VCARD_TYPE);
	uint8_t data_len = strlen(VCARD_HEAD) + strlen(VCARD_FN) + strlen(fn) + strlen(VCARD_EMAIL) + strlen(email) + strlen(VCARD_END);
	char * buff = &tag_buff[16];

	buff[0] = '\x03';
	buff[1] = header_len+data_len+3;
	buff[2] = MB|ME|SR|TNF_MIME;
	buff[3] = header_len;
	buff[4] = data_len;
	strcat(buff, VCARD_TYPE);
	strcat(buff, VCARD_HEAD);
	strcat(buff, VCARD_FN);
	strcat(buff, fn);
	strcat(buff, VCARD_EMAIL);
	strcat(buff, email);
	strcat(buff, VCARD_END);
	buff[5+header_len+data_len] = NDEF_MSG_END;
}

void ndef_well_known(char * tag_buff, char * tag_data, uint8_t size){
	init_tag(tag_buff);
	char * buff = &tag_buff[16];

	buff[0] = NDEF_MSG_BLK;
	buff[1] = size + 2;
	buff[2] = MB|SR|TNF_WELL_KNOWN;
	buff[3] = NDEF_TYPE_LEN;
	buff[4] = size - 1;
	memcpy(&buff[5], tag_data, size);
	buff[size+5] = NDEF_MSG_END;
}

void nfc_reader(){
	nfc_reset();
	uint8_t rxbuff[20] = {};
	uint8_t txbuff[20] = {};
	nfc_comm(rxbuff, txbuff, "\x00\x02\x02\x02\0", 5, true);
	
	nfc_comm(rxbuff, txbuff, "\0\x04\x02\x26\x07", 5, true);
	if(rxbuff[1] == 0x80 || rxbuff[1] == 0x90){
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
	}

	
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