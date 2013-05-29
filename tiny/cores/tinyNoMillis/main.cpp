#include <Arduino.h>

int main(void)
{
	OSCCAL = TUNED_OSCCAL_VALUE; //set the oscillator calibration value based on the pins_arduino.h file. If this is not set, it will be optimised away - it would boil down to 1 = 1;
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

