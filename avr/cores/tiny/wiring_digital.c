/* wiring_analog.c - ADC and PWM-related functionality
 * Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
 *   (c) 2005-2006 David A. Mellis as part of Arduino
 *   Modified  28-08-2009 for attiny84 R.Wiersma
 *   Modified  14-10-2009 for attiny45 Saposoft
 *   Modified extensively 2016-2022 Spence Konde for ATTinyCore
 *   Free Software - LGPL 2.1, please see LICENCE.md for details
 *---------------------------------------------------------------------------*/

#define ARDUINO_MAIN
#include "wiring_private.h"
#include "pins_arduino.h"

void pinMode(uint8_t pin, uint8_t mode) {
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  check_valid_digital_pin(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PORT) return;

  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);

#if defined(PUEA)
  out = portPullupRegister(port);
#else
  out = portOutputRegister(port);
#endif

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

void turnOffPWM(uint8_t timer) {
  #if defined(TOCPMCOE)
    // No need to change the timer, just flip the bit in TOCPMCOE
    if (timer) {                          //          All PWM on these parts is controlled like this (unlike the x7)
      uint8_t bitmask = timer & 0xF0;     // mov andi Copy the portion that contains the bitmask;
      if (!(timer & 0x08)) {              // sbrs     0x08 is set for the higher 4 bits, so we want to swap only if it's not set
        _SWAP(bitmask);                   // swp
      }
      TOCPMCOE &= ~bitmask;               // com, sts
      timer &= 0x07;
    }
  #else

    #if defined(TCCR0A) && defined(COM0A1)
      if( timer == TIMER0A) {
        TCCR0A &= ~(1 << COM0A1);
        // TCCR0A &= ~(1 << COM0A0); /* We do not clean up after user code that may have shit on a timer. */

      } else
    #endif
    #if defined(TCCR0A) && defined(COM0B1)
      if( timer == TIMER0B){
        TCCR0A &= ~(1 << COM0B1);
        // TCCR0A &= ~(1 << COM0B0); /* We do not clean up after user code that may have shit on a timer. */
      } else
    #endif
    #if defined(__AVR_ATtinyX7__)
      if (timer & 0xF1) {               // It's one of the flex pins on timer1
        // Timer1 on x7                   // Likely implementation:
        uint8_t bitmask = timer & 0xF0;   // mov, andi
        if (!(timer & 0x04)){             // sbrs
          _SWAP(bitmask);                 // swp
        }
        TCCR1D &= (~bitmask);           // com, sts
      }
      // Using the swp instruction results in MUCH better code. The old way had a variable shift, which gets implemented as a tiny loop.
      // Not terrible on code size or anything, but it can add two microseconds to execution time - unlike the modern AVRs where this
      // function takes forever and a day to return, on classic parts, turnOffPWM otherwise takes **maybe** a microsecond.
    #elif defined(TCCR1E)
      // ATtiny x61
      if( timer == TIMER1A) {
        // disconnect pwm to pin on timer 1, channel A
        TCCR1C &= ~(1 << COM1A1S);
      } else if (timer == TIMER1B) {
        // disconnect pwm to pin on timer 1, channel B
        TCCR1C &= ~(1 << COM1B1S);
      } else if (timer == TIMER1D) {
        // disconnect pwm to pin on timer 1, channel D
        TCCR1C &= ~(1 << COM1D1);
      } else
    #else
      // Timer1 for non-x61/x7
      #if defined(TCCR1) && defined(COM1A1) // x5
        if(timer == TIMER1A) {
          TCCR1 &= ~(1 << COM1A1);
        } else
      #endif
      #if defined(TCCR1) && defined(COM1B1) // x5
        if(timer == TIMER1B) {
          GTCCR &= ~(1 << COM1B1);
        } else
      #endif
      #if defined(TCCR1A) && defined(COM1A1) // civilized parts
        if(timer == TIMER1A) {
          TCCR1A &= ~(1 << COM1A1);
        } else
      #endif

      #if defined(TCCR1A) && defined(COM1B1) // civilized parts
        if(timer == TIMER1B) {
          TCCR1A &= ~(1 << COM1B1);
        } else
      #endif
    #endif
    {
      // dummy block to fill in the else statement
    }
  #endif
}

void digitalWrite(uint8_t pin, uint8_t val) {
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  check_valid_digital_pin(pin);
  uint8_t timer = digitalPinToTimer(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;

  if (port == NOT_A_PIN) {
    return;
  }
  // If the pin that support PWM output, we need to turn it off
  // before doing a digital write.
  if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  out = portOutputRegister(port);
  #if defined(PUEA)
    volatile uint8_t *pue;
    pue = portPullupRegister(port);
    if (val == LOW) {
      uint8_t oldSREG = SREG;
      cli();
      *pue &= ~mask;
      *out &= ~mask;
      SREG = oldSREG;
    } else {
      uint8_t oldSREG = SREG;
      cli();
      *pue |= mask;
      *out |= mask;
      SREG = oldSREG;
    }
  #else
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
  #endif
}

void openDrain(uint8_t pin, uint8_t val) {
  if (pin > 127) {pin = analogInputToDigitalPin((pin & 127));}
  check_valid_digital_pin(pin);
  uint8_t timer = digitalPinToTimer(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t * out;
  volatile uint8_t * ddr;

  if (port == NOT_A_PIN) return;

  // If the pin that support PWM output, we need to turn it off
  // before doing a digital write.
  if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  out = portOutputRegister(port);
  ddr = portModeRegister(port);
  if (val == LOW) {
    uint8_t oldSREG = SREG;
    cli();
    *out &= ~mask;
    *ddr |= mask;
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
    cli();
    *out &= ~mask;
    *ddr &= ~mask;
    SREG = oldSREG;
  }
}


int8_t digitalRead(uint8_t pin)
{
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  check_valid_digital_pin(pin);
  //uint8_t timer = digitalPinToTimer(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PORT) return NOT_A_PIN;

  uint8_t mask = digitalPinToBitMask(pin);

  // There is no need to turn off PWM on a pin before doing digitalRead().
  // "read" should *NEVER* change the behavior of the thing you're using it on.
  // That's why it's called "read" not "write". As an added bonus, sets the
  // stage for auto-fast-digitalRead() for compile time known pins.
  // if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  if (*portInputRegister(port) & mask) {
    return HIGH;
  }
  return LOW;
}
