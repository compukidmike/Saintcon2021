/*
 * nfcScene.c
 *
 * Created: 9/23/2021 10:10:32 PM
 *  Author: Professor Plum
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "flash.h"
#include "machine_common.h"
#include "FrameBuffer.h"
#include "nfc.h"

int nfc_frame, nfc_lastDraw;

#define NFC_FRAME_SPAN  250

COMPILER_ALIGNED(128)
struct sha_context context;
COMPILER_PACK_RESET();

const uint8_t nfc_hashes[][20] = {
    {0x86, 0xd3, 0x0e, 0x29, 0xfe, 0x42, 0xff, 0x6a, 0x1a, 0x9a, 0x58, 0xe2, 0xf0, 0xfd, 0xaf, 0x25, 0x59, 0xc9, 0xc6, 0x38}, //"b'Fear is the mind-killer.'"
    {0x21, 0xe3, 0xfd, 0x57, 0x67, 0x56, 0x14, 0x8c, 0x7a, 0x63, 0x4d, 0x8c, 0xf9, 0xe8, 0xba, 0x01, 0x0e, 0x5f, 0xad, 0x72}, //"b"Remember...All I'm Offering Is The Truth. Nothing More.""
    {0x43, 0xae, 0xa2, 0x78, 0x50, 0xf8, 0x91, 0x2e, 0x5c, 0x58, 0x12, 0x55, 0xc6, 0xb8, 0x1c, 0xad, 0xa2, 0xb9, 0x91, 0x5a}, //"b'I fight for the Users!'"
    {0x95, 0x52, 0xbc, 0x21, 0xfb, 0x21, 0x5e, 0xef, 0x94, 0x07, 0xbf, 0x91, 0x81, 0x9d, 0x9f, 0xad, 0x91, 0x28, 0x50, 0x21}, //"b'Their crime is curiosity.'"
    {0xff, 0x65, 0x7a, 0x6a, 0x86, 0xee, 0xcd, 0xde, 0xcc, 0xd6, 0x1f, 0x93, 0x1f, 0x2f, 0xad, 0x5a, 0x0a, 0x15, 0x8b, 0xb4}, //"b"Roads? Where we're going, we don't need roads""
    {0xe9, 0x4b, 0xe8, 0xe6, 0x9a, 0x6a, 0xb6, 0x28, 0x46, 0xf6, 0x1e, 0xfa, 0x73, 0x94, 0x16, 0xe7, 0xa6, 0x5d, 0x31, 0xac}, //"b'Too Many Secrets'"
    {0xd3, 0x4b, 0x1a, 0x64, 0x87, 0x56, 0x07, 0xb6, 0xf5, 0x2a, 0x87, 0xd8, 0x2d, 0xfe, 0xbc, 0x92, 0xe2, 0x97, 0xd0, 0x9a}, //"b'A strange game. The only winning move is not to play.'"
    {0x9b, 0x11, 0x1b, 0xbf, 0x8c, 0x1d, 0x07, 0x76, 0x52, 0x4d, 0xbd, 0x4f, 0x73, 0x62, 0xab, 0x16, 0x87, 0xa6, 0x0e, 0xf7}, //"b"Life moves pretty fast. If you don't stop and look around once in a while, you could miss it.""
    {0xe3, 0x95, 0x33, 0xc8, 0x85, 0x48, 0x82, 0xeb, 0x35, 0x0e, 0x71, 0x04, 0x96, 0xa9, 0xa8, 0x07, 0x55, 0xcf, 0x39, 0x7a}, //"b"The only privacy that's left is the inside of your head.""
    {0xeb, 0x42, 0x57, 0xf3, 0x28, 0xb3, 0x5f, 0xf3, 0x78, 0x31, 0xc0, 0x05, 0xff, 0x8d, 0x1f, 0x57, 0x49, 0xaf, 0xb0, 0xa5}, //"b'We must keep our faith in the Republic. The day we stop believing democracy can work is the day we lose it.'"
    {0xd7, 0xe4, 0x2d, 0x73, 0x6f, 0x4a, 0x76, 0xc8, 0x98, 0x4b, 0x17, 0x91, 0xab, 0xa6, 0x58, 0xae, 0x43, 0x3e, 0xdf, 0x73}, //"b"Well, if droids could think, there'd be none of us here, would there?""
    {0xcf, 0x2e, 0xb2, 0x78, 0x6c, 0x4d, 0xb9, 0x16, 0xe6, 0xe3, 0x93, 0x52, 0x51, 0x39, 0xdc, 0xa8, 0xa5, 0xdf, 0x5b, 0x3c}, //"b'One ring to rule them all, one ring to find them, one ring the bring them all, and in the darkness bind them.'"
    {0x7a, 0x83, 0x2c, 0x12, 0x17, 0x6f, 0x73, 0x26, 0x64, 0x55, 0xbf, 0xdd, 0x5b, 0xd5, 0x69, 0x97, 0x8f, 0xe2, 0x16, 0xd4}, //"b"I'm sorry, Dave. I'm afraid I can't do that.""
    {0x22, 0xd6, 0xec, 0x39, 0x7c, 0x09, 0x86, 0x02, 0x8a, 0xc8, 0x49, 0xfd, 0x8b, 0x7f, 0xd6, 0x66, 0xa7, 0x70, 0x62, 0x8a}, //"b'If we knew what it was we were doing, it would not be called research, would it?'"
    {0x93, 0x3d, 0x77, 0x34, 0xea, 0x1a, 0x7e, 0xb6, 0x60, 0xa7, 0x91, 0x46, 0xc8, 0xa6, 0xdb, 0xcb, 0xd6, 0x7b, 0x20, 0xbf}, //"b'What... is the air-speed velocity of an unladen swallow?'"
    {0x90, 0x1d, 0x03, 0x9e, 0x2d, 0x62, 0x77, 0xfd, 0x82, 0x53, 0x87, 0xcb, 0x75, 0xcd, 0x57, 0xfe, 0x19, 0xda, 0xb5, 0xc8}, //"b'I am a leaf on the wind. Watch how I soar.'"
    {0x8b, 0x95, 0x01, 0x0d, 0x6e, 0x05, 0x50, 0x16, 0x68, 0x2a, 0x30, 0xbe, 0x64, 0xf7, 0x34, 0x77, 0xf1, 0x59, 0x7a, 0xb7}, //"b'I only speak two languages: English and bad English.'"
    {0x64, 0xff, 0x62, 0xca, 0x70, 0x2d, 0xd3, 0x61, 0xb4, 0x7d, 0x04, 0x2f, 0x28, 0xf6, 0x69, 0x3a, 0x96, 0x0b, 0x45, 0xe6}, //"b'We have no names, man. No names. We are nameless!'"
    {0x04, 0x95, 0xc6, 0xee, 0x8c, 0xff, 0x39, 0x02, 0x51, 0x53, 0xcf, 0xdc, 0xfc, 0x9f, 0x9e, 0xc5, 0x37, 0xe9, 0x8d, 0x9a}, //"b'I aim to misbehave.'"
    {0xda, 0x30, 0x0a, 0x6a, 0x9d, 0x38, 0x31, 0xda, 0x87, 0xe7, 0xc1, 0x45, 0x8b, 0x9f, 0x77, 0xfe, 0x68, 0x2c, 0x07, 0x7f}, //"b'Curse your sudden but inevitable betrayal!'"
	{0xc2, 0xa1, 0x02, 0xda, 0xca, 0xc9, 0xd9, 0x9c, 0x18, 0x97, 0x31, 0xd8, 0x5d, 0x1b, 0x6b, 0x2f, 0xe4, 0x87, 0xef, 0x28}, //"b'Never gonna give you up!'"
};




int isValidCard(const char* str)
{
	uint8_t hash[20];
	sha_sync_enable(&HASH_ALGORITHM_0);
	sha_sync_sha1_compute(&HASH_ALGORITHM_0, &context, str , strlen(str), hash);
	for (int i=0; i<sizeof(nfc_hashes)/20; ++i) {
		bool match = true;
		for (int j=0; j<20; ++j) {
			if (hash[j] != nfc_hashes[i][j]) {
				match = false;
				break;
			}
		}
		if (match) {
			if(i == 20) return 2;
			uint16_t nfc_flag = (1<<i);
			if (g_state.nfc_bitmask & nfc_flag)
				return 0; // already found this one
			g_state.nfc_bitmask |= nfc_flag;
			return 1;
		}
	}
	return -1;
}


void nfc_draw() {
	const int fc[] = {0,1,2,3,2,1};
	int xf = fc[nfc_frame];
	canvas_drawImage_FromFlash_p(0, 0, 240, 240, NFC_IMG, 0, 240*xf, 240);
	canvas_blt();
}

bool parse_ndef_text_record(uint8_t* buffer) {
	ndef_header *ndef = (ndef_header*)buffer;
	if ((ndef->flags & 0x3) != 1)
		return false;
	if (ndef->type_len != 1)
		return false;
	if (ndef->payload_type != 'T')
		return false;
	uint8_t len = ndef->payload_len;
	memmove(buffer, &ndef->payload[3], ndef->payload_len);
	buffer[len-3]='\0';
	return true;
}

Scene nfc_scene_loop(bool init) { 
	uint8_t cc[3] = {0,0,255};
	char nfc_buffer[512]={0};
	if (back_event) {
		back_event=false;
		uint8_t ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','2','0','2','1','.','s','c','h','e','d','.','c','o','m'};
		ndef_well_known(ndef_data, sizeof(ndef_data));
		start_nfc_tag_emulation(true, nfc_write_cb);
		return MENU;
	}
	
	if (init) {
		nfc_frame = 0;
		nfc_lastDraw = 0;
	}
	
	led_set_color(cc);
	if (nfc_reader(nfc_buffer))  
	{
		cc[1] = 255;
		led_set_color(cc);
		uint8_t ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','2','0','2','1','.','s','c','h','e','d','.','c','o','m'};
		ndef_well_known(ndef_data, sizeof(ndef_data));
		start_nfc_tag_emulation(true, nfc_write_cb);
		if (!parse_ndef_text_record(nfc_buffer)) {
			setMessage("Not Supported NFC tag");
			return MESSAGE;
		}
		int r = isValidCard(nfc_buffer);
		if (r < 0) {
			setMessage("Invalid NFC Unlock");
			return MESSAGE;
		}
		else if (r==0) {
			setMessage("NFC card already scanned");
			return MESSAGE;
		}
		else if (r==2){
			return RICK;
		}
		else {
			return REWARD;
		}
	}
	led_off();
	
	uint32_t now = millis();
	int dt = now - nfc_lastDraw;
	if (dt >= NFC_FRAME_SPAN) {
		if (++nfc_frame >= 6)
		nfc_frame %= 6;
		nfc_draw();
		nfc_lastDraw = now;
	}
	return NFCREADER;
}