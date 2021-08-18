#include <atmel_start.h>
#include "ILI9331.h"
#include "FrameBuffer.h"

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	//touch init is disabled in this function because it's causing problems. Needs work.
	atmel_start_init();

	/* Replace with your application code */
	//Super basic init and button/LED test
	gpio_set_pin_direction(PIN_PA27,GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(PIN_PA27,GPIO_PULL_UP);
	pwm_set_parameters(&PWM_0, 10000, 5000);
	
	
	gpio_set_pin_direction(PIN_PB17,GPIO_DIRECTION_OUT);
	gpio_set_pin_level(PIN_PB17,true);
	LCD_Init();
	
	canvas_clearScreen(RGB(10,10,200));
	canvas_drawText(80,100, "Magic", RGB(255,255,255));
	canvas_blt();
	
	while (1) {
		if(gpio_get_pin_level(PIN_PA27)){
			//gpio_set_pin_level(PIN_PA21,true);
			pwm_disable(&PWM_0);
		} else {
			//gpio_set_pin_level(PIN_PA21,false);
			pwm_enable(&PWM_0);
		}
	}
}

