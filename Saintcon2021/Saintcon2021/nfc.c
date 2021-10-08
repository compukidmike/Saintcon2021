
#include "nfc.h"
#include "main.h"
#include "string.h"
#include "platform.h"
#include "stdlib.h"

const char UID[] = "SAINTCN";
char tag_buff[TAG_BUFF_LEN] = {0};


void nfc_init(){
	
	spi_m_sync_get_io_descriptor(&SPI_1, &io);

	spi_m_sync_enable(&SPI_1);

	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	
	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);

	nfc_reset();
}
	
void nfc_tag_emulation(){	
	uint8_t rxbuff[20] = {};

	bool validFrame = false;
	
	//Card emulation mode
	nfc_comm(rxbuff, "\x00\x02\x02\x12\x08", true);

	// Set modulation
	nfc_comm(rxbuff, "\x00\x09\x03\x68\x00\x04", true);	
	nfc_comm(rxbuff, "\x00\x09\x04\x68\x01\x04\x15", true);

	// Setup chip to handle collision commands
	char cmd[14] = {0,0x0D,0x0B,0x44,0x00,0x00,0x88};
	memcpy(&cmd[7], UID, strlen(UID));
	nfc_comm(rxbuff, cmd, true);


	char end_tag_buff[20] = {0,0,0,0xBD, 0x04,0,0,0xFF, 0,0x05,0,0, 0,0,0,0, 0,0,0,0};
	char ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','.','o','r','g','/'};

	ndef_vcard("test", "test@example.com");
	ndef_well_known(ndef_data, sizeof(ndef_data));
	
	
	while(!validFrame){ 

		nfc_comm(rxbuff, "\x00\x05\x00", true);

		if(rxbuff[1] == 0){
			nfc_poll();
			nfc_read(rxbuff);
			if(rxbuff[1] == 0x80){
				if(rxbuff[3] == 0x30){ // A write request command
					char buff[] = {0,6,17, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0x28};
					if(rxbuff[4] < (sizeof(tag_buff)/4)){ // Tag buffer from memory
						memcpy(&buff[3], &tag_buff[rxbuff[4]*4], 16);
					}else if(rxbuff[4] >= 0x82 && rxbuff[4] < 0x87){ // Tail memory locations
						uint8_t amnt = 0x87 - rxbuff[4];
						if(amnt > 4){amnt = 4;}
						memcpy(&buff[3], &end_tag_buff[(rxbuff[4]-0x82)*4], amnt);
					}					
					nfc_comm(&rxbuff[10], buff, true);
				}else if(rxbuff[3] == 0x60){ // Tell the reader this is a NTAG215
					nfc_comm(&rxbuff[10], "\0\x6\x9\x0\x4\x4\x2\x1\x0\x11\x3\x28", true); 
				}
  			}
		}
	}
}

void init_tag(){
	memset(tag_buff, 0, TAG_BUFF_LEN);
	memcpy(tag_buff, UID, 3);
	memcpy(&tag_buff[4], &UID[3], 4);
	tag_buff[3] = 0x88^UID[0] ^UID[1]^UID[2];
	tag_buff[8] = UID[3]^UID[4]^UID[5]^UID[6];
// 	buff[10] = 0xFF;
// 	buff[11] = 0xFF;
	tag_buff[12] = 0xE1;
	tag_buff[13] = 0x10;
	tag_buff[14] = 0x3E;
}

void ndef_vcard(char * fn, char* email){
	init_tag();
	uint8_t header_len = strlen(VCARD_TYPE);
	uint8_t data_len = strlen(VCARD_HEAD) + strlen(VCARD_END);
	char * buff = &tag_buff[16];

	buff[0] = 0x03;
	buff[1] = 1; //This needs to be not zero for strcat to work;
	buff[2] = MB|ME|SR|TNF_MIME;
	buff[3] = header_len;
	buff[4] = 1; //This needs to be not zero for strcat to work;
	strcat(buff, VCARD_TYPE);
	strcat(buff, VCARD_HEAD);
	if(fn != NULL){
		data_len += strlen(VCARD_FN) + strlen(fn);
		strcat(buff, VCARD_FN);
		strcat(buff, fn);
	}
	if(email != NULL){
		data_len += strlen(VCARD_EMAIL) + strlen(email);
		strcat(buff, VCARD_EMAIL);
		strcat(buff, email);
	}
	strcat(buff, VCARD_END);
	buff[1] = header_len+data_len+3;
	buff[4] = data_len;
	buff[5+header_len+data_len] = NDEF_MSG_END;
}

void ndef_well_known(char * tag_data, uint8_t size){
	init_tag();
	char * buff = &tag_buff[16];

	buff[0] = NDEF_MSG_BLK;
	buff[1] = size + 3;
	buff[2] = MB|ME|SR|TNF_WELL_KNOWN;
	buff[3] = NDEF_TYPE_LEN;
	buff[4] = size - 1;
	memcpy(&buff[5], tag_data, size);
	buff[size+5] = NDEF_MSG_END;
}

void nfc_reader(){
	nfc_reset();
	char uid[7] = {};
	uint8_t rxbuff[30] = {0};
	char cmd[20] = {0};

	uint8_t ndef[100] = {0};
	nfc_comm(rxbuff, "\0\x02\x02\x02\0", false);

	if(nfc_select_card(uid)){
		memcpy(cmd, "\0\x4\x3\x30\x04\x28", 6);
		uint8_t counter = 0;
		while(1){
		nfc_comm(rxbuff, cmd, true);
		if(rxbuff[1] == 0x80 && rxbuff[2] == 0x15 && rxbuff[21] == 0x08 && (rxbuff[3] == 0x03 || ndef[0] != 0)){
			if(rxbuff[3] == 0x03 && ndef[0] == 0){
				ndef[0] = rxbuff[4] + 2;
			}else{
				ndef[0] -= 16;
			}
			memcpy(&ndef[1+(counter*16)], &rxbuff[3], 16);
			if(ndef[0] <= 16)
				break;
			cmd[4] += 4;
		}else{break;}
		counter++;
		}
		ndef[0] = 0xff;
		platformLog("Selected");

	}else{
		platformLog("No NTAG2xx");
	}
} 

bool nfc_select_card(char * buffer){
	uint8_t rxbuff[20] = {};
	char cmd[20] = {};

	memcpy(cmd, "\0\x04\x02\x26\x07", 5);

	for(uint8_t i = 0; i < 10; i++){
		nfc_comm(rxbuff, cmd, true);
		if(rxbuff[1] == 0x80 ){
			if(memcmp(&rxbuff[2], "\x05\x44\x00\x28\0\0", 6) == 0){
				memcpy(cmd, "\0\x04\x03\x93\x20\x08", 6);
			}else if(cmd[4] == 0x20 && rxbuff[8] == 0x28 && ((cmd[3] == 0x93 && rxbuff[3] == 0x88 )|| cmd[3] == 0x95)){
				cmd[2] = 8;
				cmd[4] = 0x70;
				memcpy(&cmd[5], &rxbuff[3], 5);
				cmd[10] = 0x28;
			}else if(memcmp(&cmd[3], "\x93\x70", 2) == 0 && rxbuff[6] == 0x08 && (rxbuff[3] & 0x04) == 0x04){
				memcpy(buffer, &cmd[6],3);
				memcpy(cmd, "\0\x04\x03\x95\x20\x08", 6);
			}else if(memcmp(&cmd[3], "\x95\x70", 2) == 0 && (rxbuff[3] & 0x04) == 0){
				memcpy(&buffer[3], &cmd[5], 4);
				return true;
			}else{
				break;
			}
			i = 0;
		}
	}
	nfc_comm(rxbuff, "\0\x04\x1\0", false);
	return false;
}

void nfc_poll(){
	while(gpio_get_pin_level(NFC_IRQ_OUT_PIN));
}

bool nfc_test(){
	uint8_t rxbuff[20] = {0};
		
	nfc_raw_comm(rxbuff, "\0\x55", 2, true);
	if(rxbuff[1] != 0x55 ){
		return false;
	}
			
	nfc_comm(rxbuff, "\x00\x01\x00", true);
	if (rxbuff[2] != 15){
		return false;
	}
	platformLog((char*)rxbuff+3);
	return true;
}

void nfc_read(uint8_t* rxbuff){
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
}

void nfc_reset(){
	uint8_t buff[4] = {};
	
	// send echo just to make sure card emulation has exited
	nfc_raw_comm(buff, "\0\x55", 2, false);

	//send reset control bit
	nfc_raw_comm(buff, "\x1", 1, false);
	
	//nfc_comm(buff, "\0\x2\x1\0", true);
	
	// Pulse the IRQ pin to make sure the chip is initialized from power off or idle	
	delay_ms(2);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
	delay_ms(2);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	delay_ms(12);
}

void nfc_comm(uint8_t * rx, char * command, bool read){
	uint8_t size = 3 + command[2];
	nfc_raw_comm(rx, command, size, read);
}
void nfc_raw_comm(uint8_t * rx, char * command, uint8_t size, bool read){
	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, rx, command, size);
	gpio_set_pin_level(NFC_CS_PIN, true);
	
	if(read){
		nfc_poll();
		nfc_read(rx);
	}
}