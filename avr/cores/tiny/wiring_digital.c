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

inline __attribute__((always_inline)) uint8_t check_valid_digital_pin(uint8_t pin) {
  if(__builtin_constant_p(pin)) {
    if (pin >= NUM_DIGITAL_PINS && pin != NOT_A_PIN)
    // Exception made for NOT_A_PIN - code exists which relies on being able to pass this and have nothing happen.
    // While IMO very poor coding practice, these checks aren't here to prevent lazy programmers from intentionally
    // taking shortcuts we disapprove of, but to call out things that are virtually guaranteed to be a bug.
    // Passing -1/255/NOT_A_PIN to the digital I/O functions is most likely intentional.
      badArg("Digital pin is constant, but not a valid pin");
    return pin != NOT_A_PIN;
  }
  return 1;
}

void pinMode(uint8_t pin, uint8_t mode) {
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  check_valid_digital_pin(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN) return;

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

inline __attribute__((always_inline)) void pinModeFast(uint8_t pin, uint8_t mode) {
  check_constant_pin(pin);
  if (!__builtin_constant_p(mode))
    badArg("pinModeFast requires the mode to be compile time known");
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    return;
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN) return;

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


inline __attribute__((always_inline)) void openDrainFast(uint8_t pin, uint8_t mode) {
  check_constant_pin(pin);
  if (!__builtin_constant_p(mode))
    badArg("openDrainFast requires the mode to be compile time known");
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    return;
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN) return;

  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);
  out = portOutputRegister(port);
  if (mode == FLOATING) {
    *out &= ~mask;
    *reg &= ~mask;
  } else if (mode == LOW) {
    *out |= mask;
    *reg &= ~mask;
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
      if (timer & 0xF1) { // It's one of the flex pins on timer1
      // Timer1 on x7                   // Likely implementation:
      uint8_t bitmask = timer & 0xF0;   // mov, andi
      if (!(timer & 0x04)){           // sbrs
        _SWAP(bitmask);               // swp
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

  if (port == NOT_A_PIN) return;

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

#if defined(__AVR_ATtinyX41__)
  inline __attribute__((always_inline)) void digitalWriteFaster(uint8_t pin, uint8_t val) {
    // "Pullup enable? Never heard if it"
    // writes the output register without touching the PUE register. Avail. on 841 and 441 only because that's the only place that the
    check_constant_pin(pin);
    if (pin > 127) {
      pin = analogInputToDigitalPin((pin & 127));
    }
    if (!check_valid_digital_pin(pin)) {
      // check_valid_digital_pin returns 0 if the pin is explicitly NOT_A_PIN, 1 if it is a pin, and errors with badArg if it's not a pin.
      return;
    }
    uint8_t mask = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    volatile uint8_t *out;
    if (port == NOT_A_PIN) return;
    out = portOutputRegister(port);
    if (val == LOW) {                  // 2 instruction
      *out &= ~mask;                   // 1/1 atomic
    } else {                           //
      *out |= mask;                    // 1/1
    }
  }
#endif

inline __attribute__((always_inline)) void digitalWriteFast(uint8_t pin, uint8_t val) {
  check_constant_pin(pin);
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    // check_valid_digital_pin returns 0 if the pin is explicitly NOT_A_PIN, 1 if it is a pin, and errors with badArg if it's not a pin.
    return;
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;
  if (port == NOT_A_PIN) return;
  out = portOutputRegister(port);
  #if defined(__AVR_ATtinyX41__)      //Uniquely bad
    volatile uint8_t *pue, *mode;     //
    pue = portPullupRegister(port);   //
    mode = out+1;
    if (val == LOW) {                 // 2 instructions

      if (*mode & mask) {
        *out &= ~mask;                // 1/1 atomic
      } else {                        // 1/1 +1
        uint8_t oldSREG = SREG;       // 1/1
        cli();                        // 1/1
        *pue &= ~mask;                // 3/5 in 5
        SREG = oldSREG;               // 1/1
      }
    } else {                          // total subtotal 8/10 in 5/9
      if (*mode & mask) {             // 1/1
        *out &= ~mask;                // 1/1
      } else {                        // 1/1 in 2
        uint8_t oldSREG = SREG;       // 1/1
        cli();                        // 1/1
        *pue &= ~mask;                // 3/5 in 5
        SREG = oldSREG;               // 1/1
      }                               // 17 instruction, 21 word, and and execution time of 8-12 clocks?
                                      // A whole new definition of fast (now it means "slow") - who wants to inline that!?
    }
  #elif defined(PUEA)
    if (val == LOW) {
      *pue &= ~mask;                  // 1/1
      *out &= ~mask;                  // 1/1
    } else {                          // 1/1
      *pue &= ~mask;                  // 1/1
      *out &= ~mask;                  // 1/1
    }
      // constant pin -> constant out register.
      // constant val -> constant mask. Combined with above means we will get CBI/SBI (only one bit at a time will be set in the mask
  }   // and we know which one
  #else                               // total 13 and 13 and execution time of ~8 eitherway
    if (val == LOW) {
      *out &= ~mask;                  // 1/1
    } else {                          // 1/1
      *out &= ~mask;                  // 1/1
    }   // and we know which one at compile time, so
  #endif
}



int8_t digitalRead(uint8_t pin)
{
  check_valid_digital_pin(pin);
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
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
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  check_valid_digital_pin(pin);

  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  //if (port == NOT_A_PORT) return NOT_A_PIN;  // This check is not needed, as we reject non-constant pins but constant pins that are never valid get rejected by check valid digital pin!

  return !!(*portInputRegister(port) & mask);
}
