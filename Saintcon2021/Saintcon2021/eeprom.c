/*
 * eeprom.c
 *
 * Created: 8/19/2021 4:48:15 PM
 *  Author: Professor Plum
 */ 

#include "eeprom.h"
#include "main.h"
#include <atmel_start.h>

volatile uint32_t *EEPROM_STATE_BITMASKS	= (uint32_t*)(SEEPROM_ADDR + 0);
volatile uint8_t  *EEPROM_STATE_PARTS		= (uint8_t*)(SEEPROM_ADDR + 64);
#define EEPROM_WAIT		while(NVMCTRL->SEESTAT.bit.BUSY);

volatile uint32_t *USER_PAGE = (uint32_t*)0x00804000;

#define EEPROM_MAGIC		0x53544152


void enable_eeprom_fuse() {
	uint32_t userword[8];
	for (int i=0; i<8; ++i)
		userword[i] = USER_PAGE[i];
	
	userword[1] |= 1;  //enable 512 bytes of EEPROM
	
	NVMCTRL->CTRLA.bit.CACHEDIS0 = 1; //disable cache (for now)
	
	NVMCTRL->ADDR.reg = (uint32_t)USER_PAGE;
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_EP | NVMCTRL_CTRLB_CMDEX_KEY; //Erase User page
	
	while(!NVMCTRL->INTFLAG.bit.DONE);
	
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_PBC | NVMCTRL_CTRLB_CMDEX_KEY; //Erase page cache
	
	while(!NVMCTRL->INTFLAG.bit.DONE);
	
	for (int i=0; i<4; ++i)
	USER_PAGE[i] = userword[i];
	NVMCTRL->ADDR.reg = (uint32_t)USER_PAGE;
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_WQW | NVMCTRL_CTRLB_CMDEX_KEY; //reprogram 1st 128bits
	
	while(!NVMCTRL->INTFLAG.bit.DONE);
	
	for (int i=4; i<8; ++i)
	USER_PAGE[i] = userword[i];
	NVMCTRL->ADDR.reg = (uint32_t)USER_PAGE + 16;
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_WQW | NVMCTRL_CTRLB_CMDEX_KEY; //reprogram 2nd 128 bits
	
	NVIC_SystemReset(); //restart for changes to take effect
	
}

void eeprom_init() {
	
	if (NVMCTRL->SEESTAT.bit.SBLK == 0) {
		//Runs once, need to change the user code by hand.
		enable_eeprom_fuse();
	}
	
	if (NVMCTRL->SEESTAT.bit.RLOCK)
		NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_USEER | NVMCTRL_CTRLB_CMDEX_KEY;
	
	NVMCTRL->SEECFG.reg = 1;
	
	if (NVMCTRL->SEESTAT.bit.LOCK)
		NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_USEE | NVMCTRL_CTRLB_CMDEX_KEY;
	

	return;
}

void eeprom_save_state() {
	EEPROM_WAIT;
	EEPROM_STATE_BITMASKS[0] = EEPROM_MAGIC;
	EEPROM_WAIT;
	EEPROM_STATE_BITMASKS[1] = g_state.modules_bitmask;
	EEPROM_WAIT;
	EEPROM_STATE_BITMASKS[2] = g_state.combos_bitmask;
	EEPROM_WAIT;
	EEPROM_STATE_BITMASKS[3] = g_state.nfc_bitmask;
	EEPROM_WAIT;
	EEPROM_STATE_BITMASKS[4] = g_state.badge_bitmask;
	
	for (int i=0; i<12; ++i) {
		EEPROM_WAIT;
		EEPROM_STATE_PARTS[i] = g_state.part_count[i];
	}
	
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_SEEFLUSH | NVMCTRL_CTRLB_CMDEX_KEY;
}


void eeprom_erase() {
	for ( int i=0; i<16; ++i) {
		EEPROM_WAIT;
		EEPROM_STATE_BITMASKS[0] = 0;
	}
	memset((uint8_t*)&g_state, 0, sizeof(g_state));
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMD_SEEFLUSH | NVMCTRL_CTRLB_CMDEX_KEY;
}

void eeprom_load_state() {
	EEPROM_WAIT;
	uint32_t magic =  EEPROM_STATE_BITMASKS[0];
	if (magic != EEPROM_MAGIC) {
		memset((uint8_t*)&g_state, 0, sizeof(g_state));
		return;
	}
	EEPROM_WAIT;
	g_state.modules_bitmask = EEPROM_STATE_BITMASKS[1];
	EEPROM_WAIT;
	g_state.combos_bitmask = EEPROM_STATE_BITMASKS[2];
	EEPROM_WAIT;
	g_state.nfc_bitmask = EEPROM_STATE_BITMASKS[3];
	EEPROM_WAIT;
	g_state.badge_bitmask = EEPROM_STATE_BITMASKS[4];
	
	for (int i=0; i<12; ++i) {
		EEPROM_WAIT;
		g_state.part_count[i] = EEPROM_STATE_PARTS[i];
	}
}
