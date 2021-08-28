/*
 * main.h
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <atmel_start.h>

typedef enum {
	MENU,
	BUILD,
	TRADING,
	COMBO,
	TEST,
	MACHINE,
	INVENTORY,
	VCARD
} Scene;

//NOTE: if you modify this update the eeprom save/load functions
typedef struct _badgestate {
	uint32_t modules_bitmask;
	uint32_t combos_bitmask;
	uint32_t nfc_bitmask;
	uint32_t badge_bitmask;
	uint8_t part_count[12];
} badgestate;

extern bool back_event; //TODO: maybe make this an interrupt event
extern badgestate g_state;
extern uint8_t  scroller_status;
extern uint16_t scroller_position;

void touch_status_display(void);
int getTouchLocation(void);
uint32_t millis();

Scene test_scene_loop(bool init);
Scene menu_scene_loop(bool init);
Scene combo_scene_loop(bool init);
Scene inventory_scene_loop(bool init);
Scene machine_scene_loop(bool init);
Scene build_scene_loop(bool init);

int isValidCombo(uint8_t l1, uint8_t l2, uint8_t l3);

#endif /* MAIN_H_ */