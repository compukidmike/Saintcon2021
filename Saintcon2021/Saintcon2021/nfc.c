
#include "nfc.h"
#include "main.h"
#include "string.h"
#include "platform.h"
#include "stdlib.h"
#include "hal_ext_irq.h"

volatile char TAG_BUFF[TAG_BUFF_LEN] = {0};
volatile bool NFC_BADGE_READ = false;
volatile uint8_t NFC_BADGE_WRITE = NWRITE_IDLE;


uint8_t no_field_overflow = 0;

void nfc_init(){

	spi_m_sync_get_io_descriptor(&SPI_1, &io);

	spi_m_sync_enable(&SPI_1);

	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);


	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);

	nfc_reset();
}

void start_nfc_tag_emulation(bool setup_irq){
	uint8_t rxbuff[20] = {};

	bool validFrame = false;

	//Card emulation mode
	nfc_comm(rxbuff, "\x00\x02\x02\x12\x08", true);

	// Set modulation
	nfc_comm(rxbuff, "\x00\x09\x03\x68\x00\x04", true);
	nfc_comm(rxbuff, "\x00\x09\x04\x68\x01\x04\x15", true);

	// Setup chip to handle collision commands
	char cmd[14] = {0,0x0D,0x0B,0x44,0x00,0x00,0x88};
	memcpy(&cmd[7], UID, 7);
	nfc_comm(rxbuff, cmd, true);

	if(setup_irq){
		nfc_comm(rxbuff, "\x00\x05\x00", true);
		ext_irq_register(NFC_IRQ_OUT_PIN, nfc_tag_emulation_irq);
		ext_irq_enable(NFC_IRQ_OUT_PIN);
	}
	NFC_BADGE_WRITE = NWRITE_IDLE;
}


static void nfc_tag_emulation_irq(){
	uint8_t rxbuff[25] = {0};

	if(gpio_get_pin_level(NFC_IRQ_OUT_PIN))
		return;
	ext_irq_disable(NFC_IRQ_OUT_PIN);
	nfc_read(rxbuff);

	if(rxbuff[1] == 0x80){
		bool valid_cmd = true;
		if(rxbuff[3] == 0x30 && rxbuff[2] == 5 && (rxbuff[7] & 0x3F) == 0x08){ // A read request command of proper length without errors

			char buff[] = {0,6,17, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0x28};
			if(rxbuff[4] < TAG_BUFF_LEN/4){ // Tag buffer from memory
				memcpy(&buff[3], &TAG_BUFF[rxbuff[4]*4], 16);
				NFC_BADGE_READ = true;
			}else if(rxbuff[4] >= 0x82 && rxbuff[4] < 0x87){ // Tail memory locations
				char end_TAG_BUFF[] = {0,0,0,0xBD, 0x04,0,0,0xFF, 0,0x05,0,0, 0,0,0,0, 0,0,0,0};
				uint8_t amnt = 0x87 - rxbuff[4];
				if(amnt > 4){amnt = 4;}
				memcpy(&buff[3], &end_TAG_BUFF[(rxbuff[4]-0x82)*4], amnt);
			}
			nfc_comm(rxbuff, buff, true);

		}else if(rxbuff[3] == 0x60){ // Tell the reader this is a NTAG215
			nfc_comm(rxbuff, "\0\x6\x9\x0\x4\x4\x2\x1\x0\x11\x3\x28", true);

		}else if(rxbuff[3] == 0xA2 && rxbuff[2] == 9 && (rxbuff[11] & 0x3F) == 0x08){// A write request command of proper length without errors
			unsigned char buff[] = {0,6,2, NFC_NAK,0x14};
			if(rxbuff[4] < TAG_BUFF_LEN/4 && rxbuff[4] >= 0x04){

				// Keep track of NDEF writes
				if(rxbuff[4] == 0x04 && rxbuff[5] == 0x03){
					NFC_BADGE_WRITE = NWRITE_END;
				}else if(rxbuff[4] > 0x04){
					NFC_BADGE_WRITE = NWRITE_ACTIVE;
				}

				for(uint8_t i = 0; i < 4; i++)
					TAG_BUFF[(rxbuff[4]*4)+i] = rxbuff[5+i];
				buff[3] = NFC_ACK;
			}
			nfc_comm(rxbuff, buff, true);
		}else{
			valid_cmd = false;
		}
		if(valid_cmd)
			no_field_overflow = 0;
	}else{
		no_field_overflow++;
		if(no_field_overflow > RESTART_NO_FIELD_CMD){
			nfc_reset();
			start_nfc_tag_emulation(false);
			no_field_overflow = 0;
		}
	}

	nfc_comm(rxbuff, "\x00\x05\x00", true);

	ext_irq_enable(NFC_IRQ_OUT_PIN);
}

void init_tag(){
	memset(TAG_BUFF, 0, TAG_BUFF_LEN);
	memcpy(TAG_BUFF, UID, 3);
	memcpy(&TAG_BUFF[4], &UID[3], 4);
	TAG_BUFF[3] = 0x88^UID[0] ^UID[1]^UID[2];
	TAG_BUFF[8] = UID[3]^UID[4]^UID[5]^UID[6];
//	buff[10] = 0xFF;
//	buff[11] = 0xFF;
	TAG_BUFF[12] = 0xE1;
	TAG_BUFF[13] = 0x10;
	TAG_BUFF[14] = 0x3E;
}

void ndef_vcard(char * vcard_data){
	ndef_mime_card(NDEF_TYPE_VCARD, vcard_data, strlen(vcard_data), NULL);
}

void ndef_mime_card(char * mime_type, char * mime_data, uint8_t data_len, char * save_buff){
	if(save_buff == NULL){
		init_tag();
		save_buff = &TAG_BUFF[16];
	}
	uint8_t header_len = strlen(mime_type);

	save_buff[0] = 0x03;
	save_buff[1] = header_len+data_len+3;
	save_buff[2] = MB|ME|SR|TNF_MIME;
	save_buff[3] = header_len;
	save_buff[4] = data_len;
	strcat(save_buff, mime_type);
	memcpy(&save_buff[5+header_len], mime_data, data_len);
	save_buff[5+header_len+data_len] = NDEF_MSG_END;
}

void ndef_well_known(char * tag_data, uint8_t size){
	init_tag();
	char * buff = &TAG_BUFF[16];

	buff[0] = NDEF_MSG_BLK;
	buff[1] = size + 3;
	buff[2] = MB|ME|SR|TNF_WELL_KNOWN;
	buff[3] = NDEF_TYPE_LEN;
	buff[4] = size - 1;
	memcpy(&buff[5], tag_data, size);
	buff[size+5] = NDEF_MSG_END;
}

bool nfc_ndef_tag_writer(char * ndef_buff){
	nfc_reset();
	char uid[7] = {};
	uint8_t rxbuff[10] = {0};
	uint8_t num_bytes = ndef_buff[1] + 3;
	uint8_t page_cnt = num_bytes/4;

	nfc_comm(rxbuff, "\0\x02\x02\x02\x00", true);
	if(nfc_select_card(uid)){
		nfc_comm(rxbuff, "\0\x02\x04\x02\x00\x02\x08", true); //Write commands need a longer FDT ( 2**PP)(MM+1)(DD+128)32/13.56 micro Seconds; PP = 0x02; MM = 0x08
		for(int8_t page = page_cnt; page >= 0; page--){
			uint8_t cmd[] = {0, 0x04, 0x07,  0xA2, page+4, 0,0,0,0, 0x28};
			for(uint8_t i = 0; i < 4; i++){
				if(page+i < num_bytes){
					cmd[5+i] = ndef_buff[(page*4)+i];
				}
			}
			nfc_comm(rxbuff, cmd, true);
			if(rxbuff[1] != 0x90 || rxbuff[3] != 0x0A || rxbuff[4] != 0x24){ // did not receive an ACK. Assume write failed.
				return false;
			}
		}
		return true;
	}
	return false;
}

bool nfc_reader(char * output_buffer){
	nfc_reset();
	char uid[7] = {};
	uint8_t remaining_bytes = 0;
	uint8_t rxbuff[30] = {0};
	char cmd[20] = {0};

	nfc_comm(rxbuff, "\0\x02\x02\x02\0", false);

	if(nfc_select_card(uid)){
		memcpy(cmd, "\0\x4\x3\x30\x04\x28", 6);
		uint8_t counter = 0;
		while(1){
			nfc_comm(rxbuff, cmd, true);
			if(rxbuff[1] == 0x80 && rxbuff[2] == 0x15 && rxbuff[21] == 0x08 && (rxbuff[3] == 0x03 || remaining_bytes != 0)){
				if(rxbuff[3] == 0x03 && remaining_bytes == 0){
					remaining_bytes = rxbuff[4] + 2;
				}else{
					remaining_bytes -= 16;
				}
				memcpy(&output_buffer[(counter*16)], &rxbuff[3], 16);
				if(remaining_bytes <= 16)
					break;
				cmd[4] += 4;
			}else{break;}
			counter++;
		}
		return true;
	}
	return false;
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
	//platformLog((char*)rxbuff+3);
	return true;
}

void nfc_read(uint8_t* rxbuff){
	uint8_t txbuff[3] = {2};
	uint8_t size = 0;
	uint8_t buff = 0;

	gpio_set_pin_level(NFC_CS_PIN, false);
	spi_m_sync_io_readwrite(io, rxbuff, txbuff, 3);
	size = rxbuff[2];
	for(uint8_t i = 0; i < size; i++){
		spi_m_sync_io_readwrite(io, &buff, txbuff, 1);
		rxbuff[3+i] = buff;
	}
	gpio_set_pin_level(NFC_CS_PIN, true);
}

void nfc_reset(){
	uint8_t buff[4] = {};
	ext_irq_disable(NFC_IRQ_OUT_PIN);

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
