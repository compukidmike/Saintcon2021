#include "main.h"
#include <atmel_start.h>
#include "ILI9331.h"
#include "math.h"
//#include "touch_example.h"

volatile uint8_t measurement_done_touch;
uint8_t key_status = 0;

uint8_t  scroller_status   = 0;
uint16_t scroller_position = 0;

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
	

	//LCD_FillRect(0, 0, 240, 240, RGB(10,10,200));
	LCD_FillRect(0, 0, 240, 240, 0x8800);

	canvas_clearScreen(RGB(10,10,200));
	canvas_drawText(80,100, "Magic", RGB(255,255,255));
	canvas_blt();

	
	while (1) {
		touch_process();
		/*if (measurement_done_touch == 1) {
			measurement_done_touch = 0;
			touch_status_display();
		}*/
		
		if(gpio_get_pin_level(PIN_PA27)){
			//gpio_set_pin_level(PIN_PA21,true);
			//pwm_disable(&PWM_0);
		} else {
			//gpio_set_pin_level(PIN_PA21,false);
			pwm_enable(&PWM_0);
		}
		
		scroller_status   = get_scroller_state(0);
		scroller_position = get_scroller_position(0);
		//touchWheel = getTouchWheelPostion();
		static int x1=0,x2=0,y1=0,y2=0;
		double angle = (((double)(scroller_position)/256)*2*M_PI) + (1.5*M_PI);
		LCD_DrawLine(x1,y1,x2,y2,0x8800);
		x1 = 80*cos(angle)+120;
		y1 = 80*sin(angle)+120;
		x2 = 100*cos(angle)+120;
		y2 = 100*sin(angle)+120;
		LCD_DrawLine(x1,y1,x2,y2,0xFFFF);
	}
}


void touch_status_display(void)
{
	key_status = get_sensor_state(0) & KEY_TOUCHED_MASK;
	if (0u != key_status) {
		// LED_ON
		//pwm_enable(&PWM_0);
		} else {
		// LED_OFF
		//pwm_disable(&PWM_0);
	}
	key_status = get_sensor_state(1) & KEY_TOUCHED_MASK;
	if (0u != key_status) {
		// LED_ON
		//pwm_enable(&PWM_0);
		} else {
		// LED_OFF
		//pwm_disable(&PWM_0);
	}
	key_status = get_sensor_state(2) & KEY_TOUCHED_MASK;
	if (0u != key_status) {
		// LED_ON
		//pwm_enable(&PWM_0);
		} else {
		// LED_OFF
		//pwm_disable(&PWM_0);
	}

	scroller_status   = get_scroller_state(0);
	scroller_position = get_scroller_position(0);
	// Example: 8 bit scroller resolution. Modify as per requirement.
	scroller_position = scroller_position >> 5;
	// LED_OFF
	if (0u != scroller_status) {
		switch (scroller_position) {
			case 0:
			// LED0_ON
			pwm_enable(&PWM_0);
			break;

			case 1:
			// LED1_ON
			//pwm_enable(&PWM_0);
			break;

			case 2:
			// LED2_ON
			//pwm_enable(&PWM_0);
			break;

			case 3:
			// LED3_ON
			//pwm_enable(&PWM_0);
			break;

			case 4:
			// LED4_ON
			pwm_enable(&PWM_0);
			break;

			case 5:
			// LED5_ON
			//pwm_enable(&PWM_0);
			break;

			case 6:
			// LED6_ON
			//pwm_enable(&PWM_0);
			break;

			case 7:
			// LED7_ON
			//pwm_enable(&PWM_0);
			break;

			default:
			// LED_OFF
			
			break;
		}
	} else {
		pwm_disable(&PWM_0);
	}
}
