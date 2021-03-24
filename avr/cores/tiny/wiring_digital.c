/* wiring_analog.c - ADC and PWM-related functionality
 * Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
 *   (c) 2005-2006 David A. Mellis as part of Arduino
 *   Modified  28-08-2009 for attiny84 R.Wiersma
 *   Modified  14-10-2009 for attiny45 Saposoft
 *   Modified extensively 2016-2021 Spence Konde for ATTinyCore
 *   Free Software - LGPL 2.1, please see LICENCE.md for details
 *---------------------------------------------------------------------------*/

#define ARDUINO_MAIN
#include "wiring_private.h"
#include "pins_arduino.h"

inline __attribute__((always_inline)) void check_valid_digital_pin(uint8_t pin) {
  if(__builtin_constant_p(pin))
    if (pin >= NUM_DIGITAL_PINS && pin != NOT_A_PIN)
    // Exception made for NOT_A_PIN - code exists which relies on being able to pass this and have nothing happen.
    // While IMO very poor coding practice, these checks aren't here to prevent lazy programmers from intentionally
    // taking shortcuts we disapprove of, but to call out things that are virtually guaranteed to be a bug.
    // Passing -1/255/NOT_A_PIN to the digital I/O functions is most likely intentional.
      badArg("Digital pin is constant, but not a valid pin");
}

void pinMode(uint8_t pin, uint8_t mode) {
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  check_valid_digital_pin(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *reg, *out;

  if (port == NOT_A_PIN) return;

  reg = portModeRegister(port);
  out = portOutputRegister(port);

  if (mode == INPUT) {
    uint8_t oldSREG = SREG;
    cli();
    *reg &= ~mask;
    *out &= ~mask;
    SREG = oldSREG;
  } else if (mode == INPUT_PULLUP) {
    uint8_t oldSREG = SREG;
    cli();
    *reg &= ~mask;
    *out |= mask;
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
    cli();
    *reg |= mask;
    SREG = oldSREG;
  }
}

static void turnOffPWM(uint8_t timer) {
  #if defined(TCCR0A) && defined(COM0A1)
    if( timer == TIMER0A){
      TCCR0A &= ~(1 << COM0A1);
      // TCCR0A &= ~(1 << COM0A0); /* no user cleanup */
    } else
  #endif
  #if defined(TCCR0A) && defined(COM0B1)
    if( timer == TIMER0B){
      TCCR0A &= ~(1 << COM0B1);
      // TCCR0A &= ~(1 << COM0B0); /* no user cleanup */
    } else
  #endif
  #ifdef __AVR_ATtinyX7__
    // Timer1 on x7
    if (timer & 0x10) {
      TCCR1D &= ( ~(1 << (timer & 0x07)));
    }
  #elif defined(TCCR1E)
    // ATtiny x61
    // This can be recoded to use the OCOEn bits in TCCR1E
    // This would be much better - then we'd leave COM bits at 0, and just switch on and off the OCOEn bits
    // In this case, we would use WGM10 or WGM11 (PWM6 mode). Only one duty cycle could be output on each of the three pairs of
    // PWM pins, but it gives you more choice on which pins you use. Would implement it like we do on x7, ie, if you analogWrite()
    // both pins, and didn't turn off PWM between with digitalWrite(), you'd have identical waveform on the two pins.
    if( timer == TIMER1A){
      // disconnect pwm to pin on timer 1, channel A
      TCCR1C &= ~(1<<COM1A1S);
      // cbi(TCCR1C,COM1A0S); /* no user cleanup */
    } else if (timer == TIMER1B){
      // disconnect pwm to pin on timer 1, channel B
      TCCR1C &= ~(1<<COM1B1S);
      // cbi(TCCR1C,COM1B0S); /* no user cleanup */
    } else if (timer == TIMER1D){
      // disconnect pwm to pin on timer 1, channel D
      TCCR1C &= ~(1<<COM1D1);
      // cbi(TCCR1C,COM1D0); /* no user cleanup */
    } else
  #else
    // Timer1 for non-x61/x7
    #if defined(TCCR1) && defined(COM1A1) // x5
      if(timer == TIMER1A){
        TCCR1 &= ~(1<<COM1A1);
        // TCCR1 &= ~(1 << COM1A0); /* no user cleanup */
      } else
    #endif
    #if defined(TCCR1) && defined(COM1B1) // x5
      if( timer == TIMER1B){
        GTCCR &= ~(1<<COM1B1);
        // GTCCR &= ~(1 << COM1B1); /* no user cleanup */
      } else
    #endif
    #if defined(TCCR1A) && defined(COM1A1) // civilized parts
      if( timer == TIMER1A){
        TCCR1A &= ~(1<<COM1A1);
        // TCCR1A &= ~(1 << COM1A0); /* no user cleanup */
      } else
    #endif

    #if defined(TCCR1A) && defined(COM1B1) // civilized parts
      if( timer == TIMER1B){
        TCCR1A &= ~(1<<COM1B1);
        // TCCR1A &= ~(1 << COM1B0); /* no user cleanup */
      } else
    #endif
  #endif
  {
    // dummy block to fill in the else statement
  }
}

void digitalWrite(uint8_t pin, uint8_t val) {
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  check_valid_digital_pin(pin);
  uint8_t timer = digitalPinToTimer(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;

  if (port == NOT_A_PIN) return;

  // If the pin that support PWM output, we need to turn it off
  // before doing a digital write.
  if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  out = portOutputRegister(port);

  if (val == LOW) {
    uint8_t oldSREG = SREG;
    cli();
    *out &= ~mask;
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
    cli();
    *out |= mask;
    SREG = oldSREG;
  }
}

inline __attribute__((always_inline)) void digitalWriteFast(uint8_t pin, uint8_t val) {
  check_constant_pin(pin);
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  check_valid_digital_pin(pin);
  if (pin==NOT_A_PIN) return; // sigh... I wish I didn't have to catch this... but it's all compile time known so w/e
  // Mega-0, Tiny-1 style IOPORTs
  // Assumes VPORTs exist starting at 0 for each PORT structure
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;
  out = portOutputRegister(port);

  if (val == LOW)
    *out &= ~mask;
  /*    CHANGE is 1, same as HIGH so it can be assigned directly to the registers.
  else if (val == CHANGE)
    *in |= mask;
  */
  else // HIGH
    *out |= mask;

}

int8_t digitalRead(uint8_t pin)
{
  check_valid_digital_pin(pin);
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  //uint8_t timer = digitalPinToTimer(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PORT) return NOT_A_PIN;

  uint8_t mask = digitalPinToBitMask(pin);

  // There is no need to turn off PWM on a pin before doing digitalRead().
  // "read" should *NEVER* change the behavior of the thing you're using it on.
  // That's why it's called "read" not "write". As an added bonus, sets the
  // stage for auto-fast-digitalRead() for compile time known pins.
  // if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  if (*portInputRegister(port) & mask) return HIGH;
  return LOW;
}

inline __attribute__((always_inline)) int8_t digitalReadFast(uint8_t pin) {
  check_constant_pin(pin);
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  check_valid_digital_pin(pin);

  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PORT) return NOT_A_PIN;

  return !!(*portInputRegister(port) & mask);
}
