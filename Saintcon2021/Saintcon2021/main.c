#include "main.h"
#include "ILI9331.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "nfc.h"
#include <stdlib.h>
#include "hal_sleep.h"

volatile uint8_t measurement_done_touch;
uint8_t  scroller_status   = 0;
uint16_t scroller_position = 0;
uint8_t key_status = 0;
uint8_t keys[4] = {0};
badgestate g_state;

bool back_event = false;
bool unlock_event = false;
bool claspopen = false;
bool rouge_event = false;

#include "FrameBuffer.h"
#include "flash.h"
#include "eeprom.h"
#include "machine_common.h"

extern uint16_t bird_raw[];
extern uint32_t minibadge_delay;

static void back_button_pressed(void);
static struct timer_task Timer_task1;

#define MB_CLK_DELAY 500

const uint8_t touch_lut[256] = {
	0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 19, 20, 21, 23,
	24, 26, 27, 29, 30, 32, 33, 35, 36, 38, 39, 41, 43, 44, 46, 47, 49, 50, 52, 53, 55, 56, 58, 59, 61, 62,
	64, 65, 66, 68, 69, 70, 71, 72, 74, 75, 76, 77, 78, 78, 79, 80, 81, 81, 82, 83, 83, 84, 84, 84, 85, 85,
	85, 85, 85, 85, 85, 85, 85, 86, 86, 86, 87, 87, 88, 88, 89, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
	100, 101, 103, 104, 105, 106, 108, 109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126, 128, 129,
	131, 132, 134, 135, 137, 138, 140, 141, 143, 144, 146, 147, 149, 150, 151, 153, 154, 155, 156, 157, 159,
	160, 161, 162, 163, 163, 164, 165, 166, 166, 167, 168, 168, 169, 169, 169, 170, 170, 170, 170, 170, 170,
	170, 170, 170, 171, 171, 171, 172, 172, 173, 173, 174, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 186, 188, 189, 190, 191, 193, 194, 196, 197, 199, 200, 202, 203, 205, 206, 208, 209, 211, 213,
	214, 216, 217, 219, 220, 222, 223, 225, 226, 228, 229, 231, 232, 234, 235, 236, 238, 239, 240, 241, 242,
	244, 245, 246, 247, 248, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255
};
#define JAIL_DEVICE

static void Timer_task1_cb(const struct timer_task *const timer_task)
{
	touch_process();
}

void Timer_touch_init(void)
{
	Timer_task1.interval = 20;
	Timer_task1.cb       = Timer_task1_cb;
	Timer_task1.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&Timer, &Timer_task1);
	timer_start(&Timer);
}

void vcard_write_callback(char* vcarddata) {
	const char* magic = "PRODID:-//saintcon_2021_magic";
	size_t mlen = strlen(magic) +1;
	size_t vlen = strlen(vcarddata);
	char* dp = strstr(vcarddata, magic);
	if (dp) {
		vlen -= dp-vcarddata;
		vlen -= mlen;
		memmove(dp, dp+mlen, vlen+1);
		dp[strlen(dp)-1]='\0';
		flash_save_vcard(vcarddata);
		uint8_t cc[] = {255,255,255};
		led_set_color(cc);
		NVIC_SystemReset();
	}
}

void nfc_write_cb() {
	uint8_t lc[256];
	memcpy(lc, TAG_BUFF, 256);
	ndef_header *ndef = (ndef_header*)lc;
	if (strncmp(&lc[0x15], "text/vcard", 10) == 0) { //Lazy hacks!
		vcard_write_callback(&lc[0x15+10]);
	}
	else if (strncmp(&lc[0x15], "application/encrypted", 21)==0){
		nfc_trade_write_callback(&lc[0x15+21]);
	}
}

int main(void)
{
	char vcard[512];
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	SysTick_Config(48000000/1000);

	/* Replace with your application code */
	pwm_enable(&PWM_0);
	pwm_set_parameters(&PWM_0,255,100);
	
	
	gpio_set_pin_direction(PIN_PB17,GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PIN_PB17,true);
	
	rand_sync_enable(&RAND_0);
	LCD_Init();
	
	eeprom_init();
	eeprom_load_state();

	
	flash_init();
	uint8_t id[4];
	flash_read_id(id);
	
	spi_m_sync_get_io_descriptor(&SPI_1, &io);

	spi_m_sync_enable(&SPI_1);
	
	nfc_init();
	uint8_t cc[] = {0, 255, 0};
	if(!nfc_test()){
		cc[0] = 255;
		cc[1] = 0;
	}
	led_set_color(cc);
	//End NFC Test
	uint8_t ndef_data[] = {NDEF_URL, URL_HTTPS, 's','a','i','n','t','c','o','n','2','0','2','1','.','s','c','h','e','d','.','c','o','m'};
	ndef_well_known(ndef_data, sizeof(ndef_data));
	start_nfc_tag_emulation(true, nfc_write_cb);	
	
	ext_irq_register(PIN_PA27, back_button_pressed);
	Timer_touch_init();
	
	Scene scene = MENU;
	bool changed = true;
	bool lastclasp = false;
	bool screenon = true;
	uint32_t lastTouch = millis();
	uint32_t lastMinibadgeClk = 0;
	
#ifdef JAIL_DEVICE
	if (!flash_has_vard()) {
		scene = TEST;
	}
#endif
	
	
	gpio_set_pin_direction(MB_CLK_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(MB_CLK_PIN, false);
	
	minibagde_holder_init();
	while (1) {
		
		unlock_event = false;
		
		key_status = get_sensor_state(3) & KEY_TOUCHED_MASK;
		if (0u != key_status) {
			// LED_ON
			keys[0] = 1;
			} else {
			// LED_OFF
			keys[0] = 0;
		}
		key_status = get_sensor_state(4) & KEY_TOUCHED_MASK;
		if (0u != key_status) {
			// LED_ON
			keys[1] = 1;
			} else {
			// LED_OFF
			keys[1] = 0;
		}
		key_status = get_sensor_state(5) & KEY_TOUCHED_MASK;
		if (0u != key_status) {
			// LED_ON
			keys[2] = 1;
			} else {
			// LED_OFF
			keys[2] = 0;
		}
		key_status = get_sensor_state(6) & KEY_TOUCHED_MASK;
		if (0u != key_status) {
			// LED_ON
			keys[3] = 1;
			} else {
			// LED_OFF
			keys[3] = 0;
		}
		
		if (keys[0] && !keys[1] && keys[2]) {
			claspopen = true;
			if (lastclasp == false)
				unlock_event = true;
			lastclasp = true;
		}
		else if (!keys[0] && keys[1] && !keys[2]) {
			claspopen = false;
			lastclasp = false;
		}
		
		if (keys[3]){
			if(newUnlock(UNLOCK_SHIM)) {
				scene = REWARD;
				changed = true;
			} else {
				//Already unlocked
				static bool shim_message = false;
				if(shim_message == false){ //Only show this once per power cycle, in case the sensor gets messed up
					setMessage("Shim already unlocked");
					scene = MESSAGE;
					changed = true;
				}
				shim_message = true;
			}
		}
		
		uint32_t now = millis();
		if (rouge_event && newUnlock(UNLOCK_ROUGE)) {
			scene = REWARD;
			changed = true;
			if (!screenon) {
				LCD_Wake();
				screenon = true;
			}
			lastTouch = now;
		}
		
		if((now - lastMinibadgeClk)>MB_CLK_DELAY){
			lastMinibadgeClk = now;
			gpio_toggle_pin_level(MB_CLK_PIN);
		}
		

		scroller_status   = get_scroller_state(0);
		//scroller_position = get_scroller_position(0);
		scroller_position = touch_lut[get_scroller_position(0)];
		
		if (scroller_status) {
			if (!screenon) {
				LCD_Wake();
				screenon = true;
			}
			lastTouch = now;
		}
		
		if (now > (lastTouch + SCREEN_OFF_AFTER)) {
			if (screenon) {
				LCD_Sleep();
				led_off();
				screenon = false;
				//sleep(4); //Standby Mode
			}			
		}
		
		Scene ns;
		switch(scene) {
		case TEST:
			ns = test_scene_loop(changed);
			break;
		case COMBO:
			ns = combo_scene_loop(changed);
			break;
		case INVENTORY:
			ns = inventory_scene_loop(changed);
			break;
		case MACHINE:
			ns = machine_scene_loop(changed);
			break;
		case BUILD:
			ns = build_scene_loop(changed);
			break;
		case REWARD:
			ns = reward_scene_loop(changed);
			break;
		case MESSAGE:
			ns = message_scene_loop(changed);
			break;
		case NFCREADER:
			ns = nfc_scene_loop(changed);
			break;
		case GAME:
			ns = game_scene_loop(changed);
			break;
		case TRADING:
			ns = trade_scene_loop(changed);
			break;
		case RICK:
			ns = rick_scene_loop(changed);
			break;
		case MENU:
		default:
			ns = menu_scene_loop(changed);
		}
		changed = false;
		
		if (ns != scene) {
			changed = true;
			scene = ns;
		}
		
		cdcd_loop();
	}
}

/** \brief Do SPI read in polling way
 *  For SPI master, activate CS, do send 0xFFs and read data, deactivate CS.
 *
 *  It blocks until all data read or error.
 *
 *  \param[in, out] spi Pointer to the HAL SPI instance.
 *  \param[out] buf Pointer to the buffer to store read data.
 *  \param[in] size Size of the data in number of characters.
 *  \return Operation status.
 *  \retval size Success.
 *  \retval >=0 Time out, with number of characters read.
 */
int32_t spi_m_sync_io_readwrite(struct io_descriptor *io, uint8_t *rxbuf, uint8_t *txbuf, const uint16_t length)
{
	ASSERT(io);

	struct spi_m_sync_descriptor *spi = CONTAINER_OF(io, struct spi_m_sync_descriptor, io);
	struct spi_xfer               xfer;

	xfer.rxbuf = rxbuf;
	xfer.txbuf = txbuf;
	xfer.size  = length;

	return spi_m_sync_transfer(spi, &xfer);
}

static void back_button_pressed(void)
{
	back_event = true;
}

volatile uint32_t MS_Timer = 0;
void SysTick_Handler(void) {
	MS_Timer++;                // Increment global millisecond timer
}

uint32_t millis() {
	return MS_Timer;
}


int getTouchLocation() {
	int ret = scroller_position*360/256 + 270;
	if (ret > 360) ret-=360;
	return ret;
}

void led_set_color(uint8_t color[3]){
	hri_tcc_write_CC_reg(TCC0, 2, 255-color[0]);
	hri_tcc_write_CC_reg(TCC0, 1, 255-color[1]);
	hri_tcc_write_CC_reg(TCC0, 3, 255-color[2]);
}

void led_off(void){
	hri_tcc_write_CC_reg(TCC0, 2, 255);
	hri_tcc_write_CC_reg(TCC0, 1, 255);
	hri_tcc_write_CC_reg(TCC0, 3, 255);
}


bool newUnlock(uint16_t unlockflag) {
	if (g_state.badge_bitmask & unlockflag)
	return false;
	g_state.badge_bitmask |= unlockflag;
	return true;
}