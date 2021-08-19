#include <atmel_start.h>

/**
 * Initializes MCU, drivers and middleware in the project
 **/
void atmel_start_init(void)
{
	system_init();

	//This causes enough problems that the chip won't get to main
	touch_init();
}
