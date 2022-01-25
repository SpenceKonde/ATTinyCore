#include "wiring_private.h"
#include "pins_arduino.h"
#ifdef SLEEP_MODE_ADC
int analogRead_NR(uint8_t pin) {
  return _analogRead(pin, true);
}

EMPTY_INTERRUPT(ADC_vect)
#endif
