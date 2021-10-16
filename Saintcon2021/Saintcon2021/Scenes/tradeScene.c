/*
 * tradeScene.c
 *
 * Created: 10/8/2021 4:04:32 PM
 *  Author: Professor Plum
 */ 

#include "main.h"
#include "FrameBuffer.h"
#include "flash.h"
#include "machine_common.h"
#include "nfc.h"
#include <stdio.h>
#include <string.h>

static requirement outgoing[4], received[4];
static uint8_t trade_idx, trade_slot;
static bool trade_more, trade_less, trade_complete, trade_touching;
static uint8_t trade_frame;
static bool trade_btn_dwn[4];
struct sha_context context;
uint16_t mynonce;
static bool waiting_for_fin;



#define TRADEMAGIC	0x6575

#pragma pack(1)
typedef struct trade_message{
	uint16_t magic, msg_id, nonce1, nonce2;
	requirement reqparts[4];
} trade_message;

const uint8_t trade_key[16] = {0x19, 0x7d, 0x51, 0xdf, 0xcb, 0x0e, 0x47, 0x3b, 0x58, 0x01, 0xb5, 0x7c, 0x41, 0x22, 0x20, 0x25 };


const uint8_t trade_bits[3][128] = {
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xE0, 0x03, 0xC0, 0x07, 0xF0, 0x07, 0xE0, 0x0F,
		0x38, 0x0E, 0x70, 0x1C, 0x1C, 0x1C, 0x38, 0x38, 0x0C, 0x18, 0x18, 0x30,
		0x0C, 0x18, 0x18, 0x30, 0x0C, 0x18, 0x18, 0x30, 0x0C, 0x18, 0x18, 0x30,
		0xEC, 0x1B, 0xD8, 0x37, 0xFC, 0x1F, 0xF8, 0x3F, 0xFC, 0x1F, 0xF8, 0x3F,
		0x3E, 0x3E, 0x7C, 0x7C, 0x0E, 0x38, 0x1C, 0x70, 0x0F, 0x78, 0x1E, 0xF0,
		0x07, 0x70, 0x0E, 0xE0, 0x07, 0x70, 0x0E, 0xE0, 0x07, 0x70, 0x0E, 0xE0,
		0x0F, 0x78, 0x1E, 0xF0, 0x0E, 0x38, 0x1C, 0x70, 0x3E, 0x3E, 0x7C, 0x7C,
		0xFC, 0x1F, 0xF8, 0x3F, 0xF8, 0x0F, 0xF0, 0x1F, 0xE0, 0x03, 0xC0, 0x07,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0xE0, 0x01, 0xE0, 0x07, 0xF0, 0x03,
		0xE0, 0x07, 0xF0, 0x03, 0x70, 0x0E, 0x38, 0x07, 0x30, 0x0C, 0x18, 0x06,
		0x30, 0x0C, 0x18, 0x06, 0x30, 0x0C, 0x18, 0x06, 0x30, 0x0C, 0x18, 0x06,
		0xF0, 0x0F, 0xF8, 0x07, 0xF0, 0x0F, 0xF8, 0x07, 0xF0, 0x0F, 0xF8, 0x0F,
		0xF8, 0x1E, 0xBC, 0x0F, 0x38, 0x18, 0x0C, 0x1E, 0x3C, 0x38, 0x0E, 0x1E,
		0x3C, 0x38, 0x0E, 0x1E, 0x3C, 0x38, 0x0E, 0x1E, 0x3C, 0x38, 0x0E, 0x1E,
		0x3C, 0x38, 0x0E, 0x1E, 0x38, 0x18, 0x0C, 0x0E, 0xF8, 0x1E, 0xBC, 0x0F,
		0xF0, 0x0F, 0xF8, 0x07, 0xE0, 0x07, 0xF0, 0x03, 0xC0, 0x03, 0xE0, 0x01,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00,
		0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00,
		0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0xF0, 0x3C, 0x00,
		0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00, 0x00, 0xC0, 0x0C, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, }	
	};

void calc_trade_options() {
	for (trade_slot=0; trade_slot<4; ++trade_slot) {
		if (outgoing[trade_slot].part ==  trade_idx) {
			break;
		}
		else if (outgoing[trade_slot].part == none) {
			break;
		}
	}
	if ((trade_slot<4) && (outgoing[trade_slot].part != none) && (outgoing[trade_slot].count)) {
		trade_less = true;
	}
	else
		trade_less = false;
		
	if ((trade_slot<4) && (outgoing[trade_slot].count < g_state.part_count[trade_idx])) {
		trade_more = true;
	}
	else
		trade_more = false;
}

void free_slot() {
	for (int i=0; i<3; ++i) {
		if (outgoing[i].count == 0) {
			outgoing[i].part = outgoing[i+1].part;
			outgoing[i].count = outgoing[i+1].count;
			outgoing[i+1].part = none;
			outgoing[i+1].count = 0;
		}
	}
	if (outgoing[3].count == 0)
		outgoing[3].part = none;
}

void trade_scene_draw() {
	char line[21];
	if (trade_complete) {
		canvas_drawImage_FromFlash(0,0, 240, 240, TRADE2_IMG);
		for (int i = 0; i<4; ++i) {
			if (outgoing[i].part != none) {
				snprintf(line, 21, "-%2dx    %s", outgoing[i].count, part_names[outgoing[i].part]);
				canvas_drawText(36, 32 + 16*i, line, RGB(200,0,0));
				int px = (outgoing[i].part % 4)*16;
				int py = (outgoing[i].part / 4)*16;
				canvas_drawImage_FromFlash_pt(72,  32 + 16*i, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
			}
		}
		for (int i = 0; i<4; ++i) {
			if (received[i].part != none) {
				snprintf(line, 21, "+%2dx    %s",  received[i].count, part_names[received[i].part]);
				canvas_drawText(60, 146 + 16*i, line, RGB(0,200,0));
				int px = (received[i].part % 4)*16;
				int py = (received[i].part / 4)*16;
				canvas_drawImage_FromFlash_pt(96,  146 + 16*i, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
			}
		}
	}
	else {
		canvas_drawImage_FromFlash(0,0, 240, 240, TRADE_IMG);
		for (int i = 0; i<4; ++i) {
			if (outgoing[i].part != none) {
				snprintf(line, 21, "%2dx    %s", outgoing[i].count, part_names[outgoing[i].part]);
				canvas_drawText(52, 72 + 16*i, line, RGB(200,200,200));
				int px = (outgoing[i].part % 4)*16;
				int py = (outgoing[i].part / 4)*16;
				canvas_drawImage_FromFlash_pt(84,  72 + 16*i, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
			}
		}
		snprintf(line, 21, "%2dx    %s", g_state.part_count[trade_idx], part_names[trade_idx]);
		canvas_drawText(52, 164, line, RGB(200,200,200));
		int px = (trade_idx % 4)*16;
		int py = (trade_idx / 4)*16;
		canvas_drawImage_FromFlash_pt(84,  164, 16, 16, PARTS_IMG, px, py, 64, RGB(242, 170, 206));
		
		if (trade_more == false) {
			canvas_fillRect(96,0,48,25, RGB(73,61,74));
		}
		else if(trade_btn_dwn[0]) {
			canvas_drawImage_FromFlash_p(96, 0 , 48, 25, TRADE_IMG, 96, 264, 240);
		}
		if (trade_less == false) {
			canvas_fillRect(96,216,48,24, RGB(73,61,74));
		}
		else if(trade_btn_dwn[1]) {
			canvas_drawImage_FromFlash_p(96, 215, 48, 25, TRADE_IMG, 96, 240, 240);
		}
		if (trade_btn_dwn[2]) {
			canvas_drawImage_FromFlash_p(0, 96, 24, 48, TRADE_IMG, 0, 240, 240);
		}
		if (trade_btn_dwn[3]) {
			canvas_drawImage_FromFlash_p(216, 96, 24, 48, TRADE_IMG, 216, 240, 240);
		}
		int fc[] = {0,0,1,2,2,2,2,2};
		canvas_drawBitmask(164, 24, 32, 32, trade_bits[fc[(trade_frame++)/4 % 8]], RGB(200,200,200), 0);
	
	}
	canvas_blt();
}

void update_trade_tag()
{
	uint8_t enc[16];
	trade_message message;
	
	message.magic = TRADEMAGIC;
	message.msg_id = 1;
	message.nonce1 = mynonce;
	message.nonce2 = 0;
	for (int i=0; i<4; ++i)
		message.reqparts[i] = outgoing[i];
		
	aes_sync_enable(&CRYPTOGRAPHY_0);
	aes_sync_set_encrypt_key(&CRYPTOGRAPHY_0, trade_key, AES_KEY_128);
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, (uint8_t*)&message, enc);
	
	ndef_mime_card(NDEF_TYPE_P2P, enc, 16, NULL);
	
}

void trade_tag_event() {
	
}

void nfc_trade_write_callback(uint8_t * enc) {
	uint8_t buf[32];
	trade_message *message = (trade_message*)buf;
	uint8_t       sha_output[20] = {0x00};
	
	aes_sync_enable(&CRYPTOGRAPHY_0);
	aes_sync_set_encrypt_key(&CRYPTOGRAPHY_0, trade_key, AES_KEY_128);
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_DECRYPT, enc, buf);
	
	if (message->magic != TRADEMAGIC)
		return;
	
	if (message->msg_id != 2)
		return;
	
	if (message->nonce1 != mynonce)
		return;
	
	for (int i=0; i<4; ++i)
		received[i] = message->reqparts[i];
		
	((uint16_t*)buf)[0] = message->nonce1;
	((uint16_t*)buf)[1] = message->nonce2;
	memcpy(&buf[4], outgoing, 8);
	memcpy(&buf[12], received, 8);
	
	sha_sync_enable(&HASH_ALGORITHM_0);
	sha_sync_sha1_compute(&HASH_ALGORITHM_0, &context, buf, 20, sha_output);
	
	ndef_mime_card(NDEF_TYPE_P2P, sha_output, 16, NULL);
	
	NFC_BADGE_READ=false;
	waiting_for_fin = true;
}

bool attempt_trade() {
	waiting_for_fin = false;
	uint8_t tag[512]={0};
	uint8_t enc[16], buf[32];
	trade_message *message = (trade_message*)buf;
	uint8_t       sha_output[20] = {0x00};
	
	if (!nfc_reader(tag))
		return false;
		
	char* tagptr = strstr(tag, "application/encrypted");
	if (tagptr == NULL)
		return false;
		
	tagptr += 21;
	
	aes_sync_enable(&CRYPTOGRAPHY_0);
	aes_sync_set_encrypt_key(&CRYPTOGRAPHY_0, trade_key, AES_KEY_128);
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_DECRYPT, tagptr, buf);
	
	if (message->magic != TRADEMAGIC)
		return false;
		
	if (message->msg_id != 1)
		return false;
		
	for (int i=0; i<4; ++i)
		received[i] = message->reqparts[i];	
	
	message->nonce2 = mynonce;
	message->msg_id = 2;
	for (int i=0; i<4; ++i)
		message->reqparts[i] = outgoing[i];
		
	//we're going to reuse the same tag struct that we recieved 
	aes_sync_set_encrypt_key(&CRYPTOGRAPHY_0, trade_key, AES_KEY_128);
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, buf, tagptr);

	
	if(!nfc_ndef_tag_writer(tag))
		return false;
	
	((uint16_t*)buf)[0] = message->nonce1;
	((uint16_t*)buf)[1] = message->nonce2;
	memcpy(&buf[4], received, 8);
	memcpy(&buf[12], outgoing, 8);
	
	sha_sync_enable(&HASH_ALGORITHM_0);
	sha_sync_sha1_compute(&HASH_ALGORITHM_0, &context, buf, 20, sha_output);
	delay_ms(2); // give other badge some time to prep
	
	if (!nfc_reader(tag))
		return false;
		
	tagptr = strstr(tag, "application/encrypted");
	if (tagptr == NULL)
		return false;
	tagptr += 21;
		
	if (memcmp(tagptr, sha_output, 16))
		return false;
	
	uint8_t cc[]={0,255,0};
	led_set_color(cc);
	return true;
}

Scene trade_scene_loop(bool init) {
	if (init) {
		uint16_t goods=0;
		for (int i=0;i<12;++i) {
			goods += g_state.part_count[i];
		}
		if (goods == 0) {
			setMessage("You have no parts to trade");
			return MESSAGE;
		}
		trade_complete = false;
		for (int i=0; i<4; ++i) {
			outgoing[i].count = 0;
			outgoing[i].part = none;
			received[i].count = 0;
			received[i].part = none;
			trade_btn_dwn[i] = false;
			calc_trade_options();
		}
		trade_frame = 0;
		mynonce = rand_sync_read32(&RAND_0);
		update_trade_tag(); 
		led_off();
	}
	if (back_event) {
		uint8_t ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','2','0','2','1','.','s','c','h','e','d','.','c','o','m'};
		ndef_well_known(ndef_data, sizeof(ndef_data));
		
		back_event=false;
		if (trade_complete && newUnlock(UNLOCK_TRADE))
			return REWARD;
		return MENU;
	}
	
	if (!trade_complete) {
		bool touching = scroller_status != 0;
		if (touching != trade_touching) {
			if (touching) {
				uint16_t v = ((scroller_position+8)%256)/16;
				switch (v) {
					case 15:
					case 0:
					case 1:
						if (trade_more) {
							outgoing[trade_slot].count++;
							outgoing[trade_slot].part=trade_idx;
							trade_btn_dwn[0] = true;
							update_trade_tag();
						}
						break;
					case 3:
					case 4:
					case 5:
						do {
							trade_idx++;
							if (trade_idx>= 12)
								trade_idx=0;
						} while (g_state.part_count[trade_idx] == 0);
						trade_btn_dwn[3] = true;
						break;
					case 7:
					case 8:
					case 9:
						if (trade_less) {
							outgoing[trade_slot].count--;
							trade_btn_dwn[1] = true;
							if (outgoing[trade_slot].count==0)
								free_slot();
							update_trade_tag();
						}
						break;
					case 11:
					case 12:
					case 13:
						do {
							trade_idx--;
							if (trade_idx>= 12)
							trade_idx=11;
						} while (g_state.part_count[trade_idx] == 0);
						trade_btn_dwn[2] = true;
						break;
					default:
						break;
				}
				calc_trade_options();
			}
			else {
				trade_btn_dwn[0]=false;
				trade_btn_dwn[1]=false;
				trade_btn_dwn[2]=false;
				trade_btn_dwn[3]=false;
			}
			trade_touching = touching;

		}
	
		//*
		if ((trade_frame) % 100 == 0) {
			if (attempt_trade()) {
				trade_complete = true;
				eeprom_save_state();
				for (int i=0; i<4; ++i) {
					if (received[i].part != none)
						g_state.part_count[received[i].part] += received[i].count;
					if (outgoing[i].part != none)
						g_state.part_count[outgoing[i].part] -= outgoing[i].count;
				}
			}
			else {
				update_trade_tag();
				start_nfc_tag_emulation(true, nfc_write_cb);
			}
		}//*/
	}
	
	if (NFC_BADGE_READ && waiting_for_fin) {
		waiting_for_fin = false;
		trade_complete = true;
		for (int i=0; i<4; ++i) {
			if (received[i].part != none)
			g_state.part_count[received[i].part] += received[i].count;
			if (outgoing[i].part != none)
			g_state.part_count[outgoing[i].part] -= outgoing[i].count;
		}
		eeprom_save_state();
		uint8_t cc[]={0,255,0};
		led_set_color(cc);
	}
	trade_scene_draw();
	return TRADING;
}