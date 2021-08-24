/*
 * main.h
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>


//NOTE: if you modify this update the eeprom save/load functions
typedef struct _badgestate {
	uint32_t modules_bitmask;
	uint32_t combos_bitmask;
	uint32_t nfc_bitmask;
	uint32_t badge_bitmask;
	uint8_t part_count[12];
} badgestate;

extern badgestate g_state;

void touch_status_display(void);
uint32_t millis();

#endif /* MAIN_H_ */