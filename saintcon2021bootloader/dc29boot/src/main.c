/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2011-2014, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following condition is met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 * --------------------
 * SAM-BA Implementation on SAMD21 and SAMD51
 * --------------------
 * Requirements to use SAM-BA :
 *
 * Supported communication interfaces (SAMD21):
 * --------------------
 *
 * SERCOM5 : RX:PB23 TX:PB22
 * Baudrate : 115200 8N1
 *
 * USB : D-:PA24 D+:PA25
 *
 * Pins Usage
 * --------------------
 * The following pins are used by the program :
 * PA25 : input/output
 * PA24 : input/output
 * PB23 : input
 * PB22 : output
 * PA15 : input
 *
 * The application board shall avoid driving the PA25,PA24,PB23,PB22 and PA15
 * signals
 * while the boot program is running (after a POR for example)
 *
 * Clock system
 * --------------------
 * CPU clock source (GCLK_GEN_0) - 8MHz internal oscillator (OSC8M)
 * SERCOM5 core GCLK source (GCLK_ID_SERCOM5_CORE) - GCLK_GEN_0 (i.e., OSC8M)
 * GCLK Generator 1 source (GCLK_GEN_1) - 48MHz DFLL in Clock Recovery mode
 * (DFLL48M)
 * USB GCLK source (GCLK_ID_USB) - GCLK_GEN_1 (i.e., DFLL in CRM mode)
 *
 * Memory Mapping
 * --------------------
 * SAM-BA code will be located at 0x0 and executed before any applicative code.
 *
 * Applications compiled to be executed along with the bootloader will start at
 * 0x2000 (samd21) or 0x4000 (samd51)
 *
 */
#define SAMD51
#undef SAMD21
//#define __SAME53J18A__
#include "main.h"
#include "same53.h"
#include "uf2.h"
#include "uf2format.h"
#include "flash.h"
#include "FrameBuffer.h"
#include "ILI9331.h"
#include "st25r95.h"
//#include "hal_delay.h"
//#include "utils_assert.h"


#define NVM_FUSE_ADDR ((uint32_t *)NVMCTRL_FUSES_BOOTPROT_ADDR)

uint8_t bootloader_page_buf[FLASH_ROW_SIZE];

static void check_start_application(void);

static volatile bool main_b_cdc_enable = false;
extern int8_t led_tick_step;

#if defined(SAMD21)
    #define RESET_CONTROLLER PM
#elif defined(SAMD51)
    #define RESET_CONTROLLER RSTC
#endif

/**
 * \brief Check the application startup condition
 *
 */
static void check_start_application(void) {
	
	// Check if there is an IO which will hold us inside the bootloader.
	//#if defined(HOLD_PIN) && defined(HOLD_STATE)
	PORT_PINCFG_Type pincfg = {0};
	pincfg.bit.PMUXEN = false;
	pincfg.bit.INEN   = true;
	pincfg.bit.DRVSTR = true;

	PINOP(HOLD_PIN, DIRCLR);        // Pin is an input

	//#if defined(HOLD_PIN_PULLUP)
	pincfg.bit.PULLEN = true;
	PINOP(HOLD_PIN, OUTSET); // Pin is pulled up.
	//#elif defined(HOLD_PIN_PULLDOWN)
	//  pincfg.bit.PULLEN = true;
	//  PINOP(HOLD_PIN, OUTCLR); // Pin is pulled up.
	//#endif
	PINCFG(HOLD_PIN) = pincfg.reg;
	
	delay(10);

	if (PINIP(HOLD_PIN) == HOLD_STATE) {
		/* Stay in bootloader */
		return;
	}
	//#endif
	
    uint32_t app_start_address;

    /* Load the Reset Handler address of the application */
    app_start_address = *(uint32_t *)(APP_START_ADDRESS + 4);

    /**
     * Test reset vector of application @APP_START_ADDRESS+4
     * Sanity check on the Reset_Handler address
     */
    if (app_start_address < APP_START_ADDRESS || app_start_address > FLASH_SIZE) {
        /* Stay in bootloader */
        return;
    }
/*
#if USE_SINGLE_RESET
    if (SINGLE_RESET()) {
        if (RESET_CONTROLLER->RCAUSE.bit.POR || *DBL_TAP_PTR != DBL_TAP_MAGIC_QUICK_BOOT) {
            // the second tap on reset will go into app
            *DBL_TAP_PTR = DBL_TAP_MAGIC_QUICK_BOOT;
            // this will be cleared after successful USB enumeration
            // this is around 1.5s
            resetHorizon = timerHigh + 50;
            return;
        }
    }
#endif

    if (RESET_CONTROLLER->RCAUSE.bit.POR) {
        *DBL_TAP_PTR = 0;
    }
    else if (*DBL_TAP_PTR == DBL_TAP_MAGIC) {
        *DBL_TAP_PTR = 0;
        return; // stay in bootloader
    }
    else {
        if (*DBL_TAP_PTR != DBL_TAP_MAGIC_QUICK_BOOT) {
            *DBL_TAP_PTR = DBL_TAP_MAGIC;
            delay(500);
        }
        *DBL_TAP_PTR = 0;
    }

    LED_MSC_OFF();
*/
#if defined(BOARD_RGBLED_CLOCK_PIN)
    // This won't work for neopixel, because we're running at 1MHz or thereabouts...
    RGBLED_set_color(COLOR_LEAVE);
#endif

    /* Rebase the Stack Pointer */
    __set_MSP(*(uint32_t *)APP_START_ADDRESS);

    /* Rebase the vector table base address */
    SCB->VTOR = ((uint32_t)APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);

    /* Jump to application Reset Handler in the application */
    asm("bx %0" ::"r"(app_start_address));
}

extern char _etext;
extern char _end;

static inline void nvmctrl_wait_ready(void) {
	#if defined(SAMD21)
	while (NVMCTRL->INTFLAG.bit.READY == 0) { }
	#elif defined(SAMD51)
	while (NVMCTRL->STATUS.bit.READY == 0) { }
	#endif
}

static inline void nvmctrl_set_addr(const uint32_t *addr) {
	#if defined(SAMD21)
	NVMCTRL->ADDR.reg = (uint32_t)addr / 2;
	#elif defined(SAMD51)
	NVMCTRL->ADDR.reg = (uint32_t)addr;
	#endif
}

static inline void nvmctrl_exec_cmd(uint32_t cmd) {
	#if defined(SAMD21)
	NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;  // Clear error status bits.
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | cmd;
	nvmctrl_wait_ready();
	#elif defined(SAMD51)
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | cmd;
	#endif
	nvmctrl_wait_ready();
}

void set_fuses_and_bootprot(uint32_t new_bootprot) {
	#if defined(SAMD21)
	uint32_t fuses[2];
	#elif defined(SAMD51)
	uint32_t fuses[128];    // 512 bytes (whole user page)
	#endif
	nvmctrl_wait_ready();

	memcpy(fuses, (uint32_t *)NVM_FUSE_ADDR, sizeof(fuses));

	// If it appears the fuses page was erased (all ones), replace fuses with reasonable values.

	#if defined(SAMD21)
	bool repair_fuses = (fuses[0] == 0xffffffff ||
	fuses[1] == 0xffffffff);
	#elif defined(SAMD51)
	bool repair_fuses = (fuses[0] == 0xffffffff ||
	fuses[1] == 0xffffffff ||
	fuses[4] == 0xffffffff);
	#endif

	if (repair_fuses) {
		// These canonical fuse values taken from working Adafruit boards.
		// BOOTPROT is set to nothing in these values.
		#if defined(SAMD21)
		fuses[0] = 0xD8E0C7FF;
		fuses[1] = 0xFFFFFC5D;
		#elif defined(SAMD51)
		fuses[0] = 0xFE9A9239;
		fuses[1] = 0xAEECFF80;
		fuses[2] = 0xFFFFFFFF;
		// fuses[3] is for user use, so we don't change it.
		fuses[4] = 0x00804010;
		#endif
	}

	uint32_t current_bootprot = (fuses[0] & NVMCTRL_FUSES_BOOTPROT_Msk) >> NVMCTRL_FUSES_BOOTPROT_Pos;

	logval("repair_fuses", repair_fuses);
	logval("current_bootprot", current_bootprot);
	logval("new_bootprot", new_bootprot);

	// Don't write if nothing will be changed.
	if (current_bootprot == new_bootprot && !repair_fuses) {
		return;
	}

	// Update fuses BOOTPROT value with desired value.
	fuses[0] = (fuses[0] & ~NVMCTRL_FUSES_BOOTPROT_Msk) | (new_bootprot << NVMCTRL_FUSES_BOOTPROT_Pos);

	// Write the fuses.

	#if defined(SAMD21)
	NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;
	nvmctrl_set_addr(NVM_FUSE_ADDR);  // Set address to auxiliary row (fuses).
	nvmctrl_exec_cmd(NVMCTRL_CTRLA_CMD_EAR);  // Erase auxiliary row.
	nvmctrl_exec_cmd(NVMCTRL_CTRLA_CMD_PBC);  // Clear page buffer (64 bytes).
	// Writes must be 16 or 32 bits at a time.
	NVM_FUSE_ADDR[0] = fuses[0];
	NVM_FUSE_ADDR[1] = fuses[1];
	nvmctrl_exec_cmd(NVMCTRL_CTRLA_CMD_WAP);
	#elif defined(SAMD51)
	NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;
	nvmctrl_set_addr(NVM_FUSE_ADDR);  // Set address to user page.
	nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_EP);   // Erase user page.
	nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_PBC);  // Clear page buffer.
	for (size_t i = 0; i < sizeof(fuses) / sizeof(uint32_t); i += 4) {
		// Copy a quadword, one 32-bit word at a time. Writes to page
		// buffer must be 16 or 32 bits at a time, so we use explicit
		// word writes
		NVM_FUSE_ADDR[i + 0] = fuses[i + 0];
		NVM_FUSE_ADDR[i + 1] = fuses[i + 1];
		NVM_FUSE_ADDR[i + 2] = fuses[i + 2];
		NVM_FUSE_ADDR[i + 3] = fuses[i + 3];
		nvmctrl_set_addr(&NVM_FUSE_ADDR[i]); // Set write address to the current quad word.
		nvmctrl_exec_cmd(NVMCTRL_CTRLB_CMD_WQW); // Write quad word.
	}
	#endif

	resetIntoBootloader();
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
	//ASSERT(io);

	struct spi_m_sync_descriptor *spi = CONTAINER_OF(io, struct spi_m_sync_descriptor, io);
	struct spi_xfer               xfer;

	xfer.rxbuf = rxbuf;
	xfer.txbuf = txbuf;
	xfer.size  = length;

	return spi_m_sync_transfer(spi, &xfer);
}

void hardware_test(void){
	system_init2();
	
	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
	
	gpio_set_pin_direction(PIN_PB17,GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PIN_PB17,true);
	LCD_Wake();
	LCD_Init();
	LCD_FillRect(0,0,240,240,RGB(64,64,64));
	LCD_DrawChar("BOOTLOADER V1.0",((240-(strlen("BOOTLOADER V1.1")*6))/2), 10, RGB(255,255,255));
	LCD_DrawChar("HARDWARE TEST:",((240-(strlen("HARDWARE TEST:")*6))/2), 25, RGB(255,255,255));
	//LCD_DrawChar("TOUCH: Test Touch Wheel",80, 50, RGB(0,255,255));
	LCD_DrawChar("BUTTON: Press Button...",60, 55, RGB(255,255,0));
	LCD_DrawChar("LCD: Check Colors Below",60, 65, RGB(255,255,0));
	//TODO - Draw Gradients
	for(int x=0;x<128;x+=2){
		for(int y=0; y<32; y++){
			LCD_DrawPixel(x+56,y+75,x/4<<11);
			LCD_DrawPixel(x+56+1,y+75,x/4<<11);
			LCD_DrawPixel(x+56,y+75+32,x/2<<5);
			LCD_DrawPixel(x+56+1,y+75+32,x/2<<5);
			LCD_DrawPixel(x+56,y+75+64,x/4);
			LCD_DrawPixel(x+56+1,y+75+64,x/4);
		}
	}
	
	LCD_DrawChar("To Reprogram Badge:",((240-(strlen("To Reprogram Badge:")*6))/2), 180, RGB(255,255,255));
	LCD_DrawChar("Connect USB to Computer",((240-(strlen("Connect USB to Computer")*6))/2), 190, RGB(255,255,255));
	LCD_DrawChar("Drag/Drop new Firmware",((240-(strlen("Drag/Drop new Firmware")*6))/2), 200, RGB(255,255,255));
	LCD_DrawChar("file onto Badge.",((240-(strlen("file onto Badge.")*6))/2), 210, RGB(255,255,255));
	
	spi_m_sync_get_io_descriptor(&SPI_1, &io);
	spi_m_sync_enable(&SPI_1);
	//NFC Test
	st25r95Initialize();
	delay_ms(1);
	if(st25r95CheckChipID()){
		LCD_DrawChar("NFC: PASS",60, 35, RGB(0,255,0));
		//canvas_drawText(80,120,"NFC: PASS",RGB(255,255,255));
		//canvas_blt();
	} else {
		LCD_DrawChar("NFC: FAIL",60, 35, RGB(255,0,0));
		//canvas_drawText(80,120,"NFC: FAIL", RGB(255,0,0));
		//canvas_blt();
	}
	//End NFC Test
	
	flash_init();
	uint8_t id[4];
	flash_read_id(id);
	if(id[0]==0xf8 && id[1]==0x80 && id[2]==0x11){
		LCD_DrawChar("SPI FLASH: PASS",60, 45, RGB(0,255,0));
	} else {
		LCD_DrawChar("SPI FLASH: FAIL",60, 45, RGB(255,0,0));
	}
}

/**
 *  \brief  SAM-BA Main loop.
 *  \return Unused (ANSI-C compatibility).
 */
int main(void) {
    // if VTOR is set, we're not running in bootloader mode; halt
    if (SCB->VTOR)
        while (1) {
        }

#if defined(SAMD21)
    // If fuses have been reset to all ones, the watchdog ALWAYS-ON is
    // set, so we can't turn off the watchdog.  Set the fuse to a
    // reasonable value and reset. This is a mini version of the fuse
    // reset code in selfmain.c.
    if (((uint32_t *)NVMCTRL_AUX0_ADDRESS)[0] == 0xffffffff) {
        // Clear any error flags.
        NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
        // Turn off cache and put in manual mode.
        NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;
        // Set address to write.
        NVMCTRL->ADDR.reg = NVMCTRL_AUX0_ADDRESS / 2;
        // Erase auxiliary row.
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_EAR;
	while (!(NVMCTRL->INTFLAG.bit.READY)) {}
        // Clear page buffer.
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
	while (!(NVMCTRL->INTFLAG.bit.READY)) {}
        // Reasonable fuse values, including 8k BOOTPROT.
        ((uint32_t *)NVMCTRL_AUX0_ADDRESS)[0] = 0xD8E0C7FA;
        ((uint32_t *)NVMCTRL_AUX0_ADDRESS)[1] = 0xFFFFFC5D;
        // Write the fuses
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WAP;
	while (!(NVMCTRL->INTFLAG.bit.READY)) {}
        resetIntoBootloader();
    }

	//If bootloader protection is not set, set it and reset
	uint32_t nvmaux0 = ((uint32_t *)NVMCTRL_AUX0_ADDRESS)[0];
	if ((nvmaux0 & 0x7) != 0x2) { //Check for Bootprot 8k
		// Clear any error flags.
		NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
		// Turn off cache and put in manual mode.
		NVMCTRL->CTRLB.reg = NVMCTRL->CTRLB.reg | NVMCTRL_CTRLB_CACHEDIS | NVMCTRL_CTRLB_MANW;
		// Set address to write.
		NVMCTRL->ADDR.reg = NVMCTRL_AUX0_ADDRESS / 2;
		// Erase auxiliary row.
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_EAR;
		while (!(NVMCTRL->INTFLAG.bit.READY)) {}
		// Clear page buffer.
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
		while (!(NVMCTRL->INTFLAG.bit.READY)) {}
		// Reasonable fuse values, including 8k BOOTPROT.
		((uint32_t *)NVMCTRL_AUX0_ADDRESS)[0] = (nvmaux0 & 0xFFFFFFFA) | 0x00000002; //Set Bootprot to 8k
		//((uint32_t *)NVMCTRL_AUX0_ADDRESS)[1] = 0xFFFFFC5D;
		// Write the fuses
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WAP;
		while (!(NVMCTRL->INTFLAG.bit.READY)) {}
		resetIntoBootloader();
	}

    // Disable the watchdog, in case the application set it.
    WDT->CTRL.reg = 0;
    while(WDT->STATUS.bit.SYNCBUSY) {}

#elif defined(SAMD51)
    // Disable the watchdog, in case the application set it.
    WDT->CTRLA.reg = 0;
    while(WDT->SYNCBUSY.reg) {}

    // Enable 2.7V brownout detection. The default fuse value is 1.7
    // Set brownout detection to ~2.7V. Default from factory is 1.7V,
    // which is too low for proper operation of external SPI flash chips (they are 2.7-3.6V).
    // Also without this higher level, the SAMD51 will write zeros to flash intermittently.
    // Disable while changing level.

    SUPC->BOD33.bit.ENABLE = 0;
    while (!SUPC->STATUS.bit.B33SRDY) {}  // Wait for BOD33 to synchronize.
    SUPC->BOD33.bit.LEVEL = 200;  // 2.7V: 1.5V + LEVEL * 6mV.
    // Don't reset right now.
    SUPC->BOD33.bit.ACTION = SUPC_BOD33_ACTION_NONE_Val;
    SUPC->BOD33.bit.ENABLE = 1; // enable brown-out detection

    // Wait for BOD33 peripheral to be ready.
    while (!SUPC->STATUS.bit.BOD33RDY) {}

    // Wait for voltage to rise above BOD33 value.
    while (SUPC->STATUS.bit.BOD33DET) {}

    // If we are starting from a power-on or a brownout,
    // wait for the voltage to stabilize. Don't do this on an
    // external reset because it interferes with the timing of double-click.
    // "BODVDD" means BOD33.
    if (RSTC->RCAUSE.bit.POR || RSTC->RCAUSE.bit.BODVDD) {
        do {
            // Check again in 100ms.
            delay(100);
        } while (SUPC->STATUS.bit.BOD33DET);
    }

    // Now enable reset if voltage falls below minimum.
    SUPC->BOD33.bit.ENABLE = 0;
    while (!SUPC->STATUS.bit.B33SRDY) {}  // Wait for BOD33 to synchronize.
    SUPC->BOD33.bit.ACTION = SUPC_BOD33_ACTION_RESET_Val;
    SUPC->BOD33.bit.ENABLE = 1;
	
	//If bootloader protection is not set, set it and reset
	//set_fuses_and_bootprot(11); //32kB Bootloader //TODO - Re-enable this!!!!
#endif

#if USB_VID == 0x239a && USB_PID == 0x0013     // Adafruit Metro M0
    // Delay a bit so SWD programmer can have time to attach.
    delay(15);
#endif
    led_init();

    logmsg("Start");
    //assert((uint32_t)&_etext < APP_START_ADDRESS);
    // bossac writes at 0x20005000
    assert(!USE_MONITOR || (uint32_t)&_end < 0x20005000);

    assert(8 << NVMCTRL->PARAM.bit.PSZ == FLASH_PAGE_SIZE);
    assert(FLASH_PAGE_SIZE * NVMCTRL->PARAM.bit.NVMP == FLASH_SIZE);

    /* Jump in application if condition is satisfied */
    check_start_application();

    /* We have determined we should stay in the monitor. */
    /* System initialization */
    system_init();

    __DMB();
    __enable_irq();

#if USE_UART
    /* UART is enabled in all cases */
    usart_open();
#endif

    logmsg("Before main loop");

	hardware_test();

    usb_init();

    // not enumerated yet
    RGBLED_set_color(COLOR_START);
    led_tick_step = 10;

    /* Wait for a complete enum on usb or a '#' char on serial line */
    while (1) {
        if (USB_Ok()) {
            if (!main_b_cdc_enable) {
#if USE_SINGLE_RESET
                // this might have been set
                resetHorizon = 0;
#endif
                RGBLED_set_color(COLOR_USB);
                led_tick_step = 1;

#if USE_SCREEN
                screen_init();
                draw_drag();
#endif
            }

            main_b_cdc_enable = true;
        }

#if USE_MONITOR
        // Check if a USB enumeration has succeeded
        // And com port was opened
        if (main_b_cdc_enable) {
            logmsg("entering monitor loop");
            // SAM-BA on USB loop
            while (1) {
                sam_ba_monitor_run();
            }
        }
#if USE_UART
        /* Check if a '#' has been received */
        if (!main_b_cdc_enable && usart_sharp_received()) {
            RGBLED_set_color(COLOR_UART);
            sam_ba_monitor_init(SAM_BA_INTERFACE_USART);
            /* SAM-BA on UART loop */
            while (1) {
                sam_ba_monitor_run();
            }
        }
#endif
#else // no monitor
        if (main_b_cdc_enable) {
            process_msc();
        }
#endif

        /*if (!main_b_cdc_enable) {
            // get more predictable timings before the USB is enumerated
            for (int i = 1; i < 256; ++i) {
                //asm("nop");
            }
        }*/
		if(gpio_get_pin_level(PIN_PA27) == false){
			LCD_FillRect(60,55,100,20,RGB(64,64,64));
			LCD_DrawChar("BUTTON: PASS",60, 55, RGB(0,255,0));
		}
    }
}
