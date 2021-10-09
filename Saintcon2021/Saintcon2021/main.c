#include "main.h"
#include "ILI9331.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "st25r95.h"
#include <stdlib.h>

volatile uint8_t measurement_done_touch;
uint8_t  scroller_status   = 0;
uint16_t scroller_position = 0;
uint8_t key_status = 0;
uint8_t keys[4] = {0};
badgestate g_state;

bool back_event = false;
bool unlock_event = false;
bool claspopen = false;

#include "FrameBuffer.h"
#include "flash.h"
#include "eeprom.h"

extern uint16_t bird_raw[];

static void back_button_pressed(void);
static struct timer_task Timer_task1;

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

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	SysTick_Config(48000000/1000);

	/* Replace with your application code */
	pwm_enable(&PWM_0);
	pwm_set_parameters(&PWM_0,255,100);
	
	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	
	gpio_set_pin_direction(PIN_PB17,GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PIN_PB17,true);
	
	rand_sync_enable(&RAND_0);
	LCD_Init();
	
	eeprom_init();
	eeprom_load_state();
	memset((uint8_t*)&g_state, 0x55, sizeof(g_state)); //For testing always start over

	
	flash_init();
	uint8_t id[4];
	flash_read_id(id);
	//Testing Flash
	//(copy bird to flash if not already there)
	uint16_t buf[80] = {0x80};
	flash_read(BIRD_IMG, buf, 80*sizeof(uint16_t));
	if (memcmp(buf, bird_raw, 80*sizeof(uint16_t))) {
		flash_erase_32k(BIRD_IMG);
		for (int i=0; i<100; ++i) {
			uint32_t offset = i * 0x100;
			flash_write(BIRD_IMG + offset, (uint8_t*)bird_raw + offset, 0x100);
		}
	}

	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
	
	
	spi_m_sync_get_io_descriptor(&SPI_1, &io);

	spi_m_sync_enable(&SPI_1);

	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	
	//NFC Test - Remove in final code
	/*st25r95Initialize();
	delay_ms(1);
	if(st25r95CheckChipID()){
		canvas_drawText(80,120,"NFC: PASS",RGB(255,255,255));
		canvas_blt();
	} else {
		canvas_drawText(80,120,"NFC: FAIL", RGB(255,0,0));
		canvas_blt();
	}
	//End NFC Test
	
	NFC_init();*/
	
	ext_irq_register(PIN_PA27, back_button_pressed);
	Timer_touch_init();
	
	Scene scene = TEST;
	bool changed = true;
	bool lastclasp = false;
	bool screenon = true;
	uint32_t lastTouch = millis();
	
	gpio_set_pin_direction(MB_CLK_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(MB_CLK_PIN, false);
	
	
	while (1) {
		//NOTE: There is a 500ms delay in the NFC code that needs to be converted to non-blocking
		//comment the following line if you're not working on the NFC
		//exampleRfalPollerRun(); //NFC
		
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
		

		scroller_status   = get_scroller_state(0);
		scroller_position = get_scroller_position(0);
		
		uint32_t now = millis();
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