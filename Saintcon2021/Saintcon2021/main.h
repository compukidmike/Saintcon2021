/*
 * main.h
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#define NFC_IRQ_IN_PIN PIN_PB08
#define NFC_IRQ_OUT_PIN PIN_PB09
#define NFC_CS_PIN PIN_PA14
#define MB_CLK_PIN PIN_PB23

#define SCREEN_OFF_AFTER	1000 * 60

#include <atmel_start.h>
#include <stdint.h>

struct io_descriptor *io;
#include <stdint.h>
#include <atmel_start.h>

#define LED_COLOR_OFF (uint8_t[]){0,0,0}
#define LED_COLOR_RED (uint8_t[]){255,0,0}
#define LED_COLOR_GREEN (uint8_t[]){0,255,0}
#define LED_COLOR_BLUE (uint8_t[]){0,0,255}
#define LED_COLOR_YELLOW (uint8_t[]){127,127,0}
#define LED_COLOR_WHITE (uint8_t[]){85,85,85}

typedef enum {
	MENU,
	BUILD,
	TRADING,
	COMBO,
	GAME,
	MACHINE,
	INVENTORY,
	NFCREADER,
	REWARD,
	MESSAGE,
	TEST
} Scene;

#define REWARD_SHIM			(1<<0)
#define REWARD_GAME1		(1<<1)
#define REWARD_GAME2		(1<<2)
#define REWARD_MINIBADGE	(1<<3)
#define REWARD_VENDOR		(1<<4)
#define REWARD_HC			(1<<5)
#define REWARD_TRADE		(1<<6)

//NOTE: if you modify this update the eeprom save/load functions
typedef struct _badgestate {
	uint32_t modules_bitmask;
	uint32_t combos_bitmask;
	uint32_t nfc_bitmask;
	uint32_t badge_bitmask;
	uint8_t part_count[12];
} badgestate;

extern bool back_event, unlock_event, claspopen; 
extern badgestate g_state;
extern uint8_t  scroller_status;
extern uint16_t scroller_position;
extern bool rouge_event;

void touch_status_display(void);
int getTouchLocation(void);
uint32_t millis();

int32_t spi_m_sync_io_readwrite(struct io_descriptor *io, uint8_t *rxbuf, uint8_t *txbuf, const uint16_t length);

void NFC_init(void);
void exampleRfalPollerRun( void );
void exampleNFCARun( void );
Scene test_scene_loop(bool init);
Scene menu_scene_loop(bool init);
Scene combo_scene_loop(bool init);
Scene inventory_scene_loop(bool init);
Scene machine_scene_loop(bool init);
Scene build_scene_loop(bool init);
Scene reward_scene_loop(bool init);
Scene message_scene_loop(bool init);
Scene nfc_scene_loop(bool init);
Scene game_scene_loop(bool init);
Scene trade_scene_loop(bool init);

int isValidCombo(uint8_t l1, uint8_t l2, uint8_t l3);
void setMessage(const char* msg);

void led_set_color(uint8_t color[3]);
void led_off(void);

bool newUnlock(uint16_t unlockflag);
#endif /* MAIN_H_ */