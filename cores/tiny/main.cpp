#include <Arduino.h>

int main(void)
{
  #ifdef TUNED_OSCCAL_VALUE
  OSCCAL = TUNED_OSCCAL_VALUE; //set the oscillator calibration value based on the pins_arduino.h file. If this is not set, it will be optimised away
  #endif
  init();

  setup();

  for (;;)
    loop();

  return 0;
}
