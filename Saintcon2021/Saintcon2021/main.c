#include "main.h"
#include "ILI9331.h"
#include "math.h"
#include "st25r95.h"
#include "FrameBuffer.h"
#include <stdlib.h>


volatile uint8_t measurement_done_touch;
uint8_t  scroller_status   = 0;
uint16_t scroller_position = 0;

volatile uint32_t MS_Timer = 0;
void SysTick_Handler(void) {
	MS_Timer++;                // Increment global millisecond timer
}

uint32_t millis() {
	return MS_Timer;
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	//touch init is disabled in this function because it's causing problems. Needs work.
	atmel_start_init();
	SysTick_Config(48000000/1000);

	/* Replace with your application code */
	//Super basic init and button/LED test
	gpio_set_pin_direction(PIN_PA27,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(PIN_PA27,GPIO_PULL_UP);
	pwm_set_parameters(&PWM_0, 10000, 5000);
	
	gpio_set_pin_direction(NFC_CS_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_CS_PIN, true);
	gpio_set_pin_direction(NFC_IRQ_IN_PIN, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(NFC_IRQ_IN_PIN, true);
	
	
	gpio_set_pin_direction(PIN_PB17,GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PIN_PB17,true);
	LCD_Init();
	

	LCD_FillRect(0, 0, 240, 240, RGB(10,10,200));

	canvas_clearScreen(RGB(10,10,200));
	canvas_drawText(80,100, "Magic", RGB(255,255,255));
	canvas_blt();
	
	gpio_set_pin_level(NFC_IRQ_IN_PIN, false);
	
	
	spi_m_sync_get_io_descriptor(&SPI_0, &io);

	spi_m_sync_enable(&SPI_0);

	gpio_set_pin_direction(NFC_IRQ_OUT_PIN,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(NFC_IRQ_OUT_PIN,GPIO_PULL_UP);
	
	//NFC Test - Remove in final code
	st25r95Initialize();
	delay_ms(1);
	if(st25r95CheckChipID()){
		canvas_drawText(80,120,"NFC: PASS",RGB(255,255,255));
		canvas_blt();
	} else {
		canvas_drawText(80,120,"NFC: FAIL", RGB(255,0,0));
		canvas_blt();
	}
	//End NFC Test
	
	NFC_init();
	
	
	while (1) {
		exampleRfalPollerRun(); //NFC
		//exampleNFCARun();
		touch_process();
		/*if (measurement_done_touch == 1) {
			measurement_done_touch = 0;
			touch_status_display();
		}*/
		
		if(gpio_get_pin_level(PIN_PA27)){
			//gpio_set_pin_level(PIN_PA21,true);
			pwm_disable(&PWM_0);
		} else {
			//gpio_set_pin_level(PIN_PA21,false);
			pwm_enable(&PWM_0);
		}
		
		scroller_status   = get_scroller_state(0);
		scroller_position = get_scroller_position(0);
		//touchWheel = getTouchWheelPostion();
		static int x1=0,x2=0,y1=0,y2=0;
		double angle = (((double)(scroller_position)/256)*2*M_PI) + (1.5*M_PI);
		LCD_DrawLine(x1,y1,x2,y2,RGB(10,10,200));
		x1 = 80*cos(angle)+120;
		y1 = 80*sin(angle)+120;
		x2 = 100*cos(angle)+120;
		y2 = 100*sin(angle)+120;
		LCD_DrawLine(x1,y1,x2,y2,0xFFFF);
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

