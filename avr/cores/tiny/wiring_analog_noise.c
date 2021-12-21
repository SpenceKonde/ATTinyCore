
int analogRead_NR(uint8_t pin) {
  _analogRead(pin, true)
}

EMPTY_ISR(ADC_vect) {
  //serves only to wake up the part.
}
