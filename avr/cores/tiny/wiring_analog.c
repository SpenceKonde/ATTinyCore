/* wiring_analog.c - ADC and PWM-related functionality
 * Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
 *   (c) 2005-2006 David A. Mellis as part of Arduino
 *   Modified  28-08-2009 for attiny84 R.Wiersma
 *   Modified  14-10-2009 for attiny45 Saposoft
 *   Corrected 17-05-2010 for ATtiny84 B.Cook
 *   Modified extensively 2016-2021 Spence Konde for ATTinyCore
 *   Free Software - LGPL 2.1, please see LICENCE.md for details
 *---------------------------------------------------------------------------*/

#include "wiring_private.h"
#include "pins_arduino.h"
#ifdef SLEEP_MODE_ADC
  #include <avr/sleep.h>
#endif
#ifndef DEFAULT
//For those with no ADC, need to define default.
#define DEFAULT (0)
#endif

/* These are pre-shifted, as are the constants
 * Don't calculate at runtime what you could have
 * calculated at compile time! */
#if defined(REFS2)
  #if defined(ADMUXB)
    #define ADMUX_REFS_MASK (0xE0) /* 841/441 */
  #else
    #define ADMUX_REFS_MASK (0xD0) /* x5/x61 - not used for x61 */
  #endif
#elif defined(REFS1)
  #define ADMUX_REFS_MASK (0xC0) /* 3 or 4 options, generally internal 1.1v, Vcc, and external */
#elif defined(REFS0)
  #define ADMUX_REFS_MASK (0x40) /* Everything with just Vcc and internal 1.1v */
#else
  #define ADMUX_REFS_MASK (0x20) /* 828 , whose sole REFS bit has no number*/
#endif

#if (defined(MUX5) && !defined(REFS0)) /* need to exclude the 828, which has it's vestigal MUX5 bit on ADMUXB */
  #define ADMUX_MUX_MASK (0x3f)
#elif defined(MUX4)
  #define ADMUX_MUX_MASK (0x1f)
#elif defined(MUX3)
  #define ADMUX_MUX_MASK (0x0f)
#else
  #define ADMUX_MUX_MASK (0x07)
#endif

// global settings
uint8_t analog_reference = DEFAULT;

#if defined(GSEL0)
  /* ATtiny841/441 have too many gain + channel combos to stuff it all into the channel
   * so we provide an analogGain() function that is analogous to analogReference.
   * I don't think there's any advantage to setting it earlier though. */
  uint8_t analog_gain = 0;
  void analogGain(uint8_t gain) {
    if (gain > 0xFC) {
      analog_gain = 0xFF-gain;
    } else if (gain == 20) {
      analog_gain = 0x01;
    } else if (gain == 100) {
      analog_gain = 0x02;
    } else {
      analog_gain = 0x00;
    }
  }
#elif defined(__AVR_ATtinyX4__) || defined(__AVR_ATtinyX5__) || defined(__AVR_ATtinyX61__) || defined(__AVR_ATtinyX7__) || defined(__AVR_ATtiny26__)
  void analogGain( __attribute__ ((unused)) uint8_t gain) {
    badCall("analog gain is set through channel used on this part - see the part-specific documentation.");
  }
#else
  void analogGain( __attribute__ ((unused)) uint8_t gain) {
    badCall("This part does not have a differential ADC.")
  }
#endif



void analogReference(uint8_t mode) {
  /* In the stock core, this doesn't actually set the reference.
   * But here, there is no reason we can't set the reference. Certainly
   * on an ATtiny167/87 if XREF is set, we want and need to. It's unclear
   * why we *wouldn't* set the analog reference at the time
   */
  if (__builtin_constant_p(mode)) { /* check for invalid constants passed to analogReference() */
    #if !defined(__AVR_ATtinyX7__)
      if ((mode & ADMUX_REFS_MASK) != mode ) {
        badArg("That is not a reference, use the named constants listed in part-specific documentation");
      }
      #if defined(REFS2) && !defined(GSEL0) // x5 or x61
        if (mode == ADC_REF(3) || mode == ADC_REF(4) || mode == ADC_REF(5)) {
          badArg("The requested reference is not a valid reference option, use named cosntants from part-specific documentation");
        }
      #elif defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny1634__)
        if (mode == 0xC0) {
          badArg("The requested reference is not a valid reference option, use named cosntants from part-specific documentation")
        }
      #endif
    #else // is an x7 - this gets more interesting...
      if ((mode & 0xC6) != mode ) {
        badArg("That is not a reference, use the named constants listed in part-specific documentation");
      } else if (mode & 0xC0 == 0x40) {
        badArg("Invalid reference voltage: REFS=01, only REFS=10 or 11 valid - use named constants from part-specific documentation")
      } else
      #if !defined(ADC_ALLOW_BOGUS_XREF_AREF)
        // Make it easier if someone doesn't take my word for them not doing anything...
        if (mode & 0x08 && mode & 0xC4) {
          badArg("Invalid reference - AREFEN only valid when all other bits are zero - use named constants from part-specific documentation")
        } else if (mode & 0x04 && !(mode & 0xC0)) {
          badArg("Invalid reference - XREFEN only valid with internal reference - use named constants from part-specific documentation")
        }
      #endif
    #endif
  }

  analog_reference = mode;
  #if defined(AMISCR)
    /* Unique to ATtiny167, ATtiny87 - Analog MIScellaneous Control Register
     * External reference is selected by AREF bit in AMISCR - but only if internal reference not selected...
     * Reference is output on AREF - oh, pardon me, the "XREF pin" which happens to be the same pin, if
     * XREF bit is set - but only if internal reference is selected..
     * Why these couldn't have been one bit is a mystery to me - it doesn't look like AREF bit does anything
     * with internal reference set nor XREF with external/Vcc ref; if it does anything, it's not immediately
     * obvious. */
    ADMUX = (mode & ADMUX_REFS_MASK); // first set the reference...
    AMISCR = (AMISCR & 0x01) | (mode & 0x06); // then AREF/XREF configuration (if any); preserve only lowest bit.
  #else
    #if !defined(ADC_ONLY_SET_REFS_ON_READ)
      #if defined(__AVR_ATtinyX61__)
        // This one is wacky - with pre-shifting analog reference looks like:
        // REFS1 | REFS0 | - | REFS2 | - | - | - | -
        ADMUX = ((mode & 0xC0));
        // 1 ref bit, 1 mux bit, and the gain sel bit are scattered in ADCSRB, and we can't blow away it's contents because thats where we store unipolar vs bipolar mode setting too...
        // ADCSRB     BIN | GSEL | - | REFS2 | MUX5 | ADTS2 | ADTS1 | ADTS0
        // BIN stands for Bipolar INput, not BINary
        ADCSRB = (ADCSRB & 0xA7) | (mode & 0x10);
        // They could have made that harder to deal with if they really tried, maybe...
      #elif defined(ADMUXB) /* 828, 841, or 441 */
        //#if defined(GSEL0)
          // x41
          ADMUXB = mode & ADMUX_REFS_MASK;
        //#else
          // 828
        //  ADMUXB = mode & ADMUX_REFS_MASK;
        //#endif
      #elif defined(ADMUX)
        ADMUX = (mode & ADMUX_REFS_MASK);
      #else
        badCall("analogReference() cannot be used on a part without an ADC");
      #endif
    #endif
  #endif // end of non-x7 reference setting.
}


#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtinyX5__) || defined(__AVR_ATtinyX61__) || defined(__AVR_ATtinyX7__)
  inline __attribute__((always_inline)) void setADCBipolarMode(bool bipolar) {
    ADCSRB = (ADCSRB & (0x7F) ) | (bipolar ? 0x80 : 0);
  }
#elif defined(__AVR_ATtinyX41__)
  inline __attribute__((always_inline)) void setADCBipolarMode(bool bipolar) {
    if (!__builtin_constant_p(bipolar) || !bipolar) {
      badCall("This part only supports bipolar mode.");
    }
  }
#elif defined(__AVR_ATtiny26__)
  inline __attribute__((always_inline)) void setADCBipolarMode(bool bipolar) {
    if (!__builtin_constant_p(bipolar) || bipolar) {
      badCall("This part only supports unipolar mode.");
    }
  }
#else
  inline __attribute__((always_inline)) void setADCBipolarMode(__attribute__((unused))bool bipolar) {
    badCall("This part does not have a differential ADC, much less an option for bipolar vs unipolar mode");
  }
#endif


inline int analogRead(uint8_t pin) {
  if (!(pin & 0x80)) {
    pin = digitalPinToAnalogInput(pin);
  }
  if (__builtin_constant_p(pin)) {
    //the rest if this is all done by the optimizser only where pin is a constant. Otherwise, you get no
    // compile time error check, just runtime check yielding large negative value for invalid pin/options
    if (pin==NOT_A_PIN) {
      badArg("ADC channel does not exist, or analog input on that pin not supported");
    }
    #if defined(__AVR_ATtinyX61__)
      if ((pin & 0x3F) < 32 && (pin & 0x40))
        badArg("ADC channel with gain selection set does not support gain selection");
    #elif defined(__AVR_ATtinyX5__)
      if ((pin & 0x5F) > ADMUX_MUX_MASK)
        badArg("ADC channel does not exist on this part");
      if ((pin < 4 || pin > 11 ) && (pin & 0x20))
        badArg("Compile-time known ADC channel not valid: IPR bit only valid for differential channels");
    #else
      if ((pin & 0x7F) > ADMUX_MUX_MASK)
        badArg("Compile-time known ADC channel does not exist on this part");
    #endif
  }
  #ifdef SLEEP_MODE_ADC
    return _analogRead(pin, false);
  #else
    return _analogRead(pin);
  #endif
}

/* in wiring_analog_noise.c so the ISR is only defined if using this
int analogReadNR(uint8_t pin) {
  _analogRead(pin, true)
}
*/

#ifdef SLEEP_MODE_ADC
  int _analogRead(uint8_t pin, bool use_noise_reduction) {
#else
  int _analogRead(uint8_t pin) {
#endif
  #ifndef ADCSRA
    badCall("analogRead() cannot be used on a part without an ADC");
    /* if a device does not have an ADC, instead of giving a number we know is
     * wrong AND that isn't unique to error conditions, let's just refuse to
     * compile it - if they want some other function substituted in, that's
     * what #ifdefs are for, otherwise, we assume they have the wrong part
     * selected, or didn't know that the ATtiny4313/2313 don't have an ADC. */
    return -32768;
  #else
    #if !defined(ADC_NO_CHECK_PINS)
      #if defined(__AVR_ATtinyX61__)
        if ((pin & 0x3F) < 32 && (pin & 0x40))  return ADC_ERROR_NOT_A_CHANNEL;

      #elif defined(__AVR_ATtinyX5__)
        if ((pin & 0x5F) > ADMUX_MUX_MASK)      return ADC_ERROR_NOT_A_CHANNEL;
        if ((pin & 0x20) && ((pin & 0x0F) < 4 || (pin & 0x0F) > 11 ))
                                                return ADC_ERROR_SINGLE_END_IPR;
      #else
        if ((pin & 0x7F) > ADMUX_MUX_MASK)      return ADC_ERROR_NOT_A_CHANNEL;
      #endif
    #endif
    /* Whether checking that the ADC is enabled and not in use makes sense
     * is arguable. Will be one of the "flash saving" options */
    #if !defined(ADC_NO_CHECK_STATUS)
      uint8_t t = (ADCSRA & ((1 << ADEN) | (1 << ADSC)));
      if (!(t == (1 << ADEN))) return (t == (1 << ADSC)) ? ADC_ERROR_BUSY : ADC_ERROR_DISABLED;
    #endif

    /* Important difference from stock core: reference constants like DEFAULT,
     * INTERNAL1V1, and so on have the reference bits PRESHIFTED to their
     * final positions. Thus, we just OR them with the pin or w/e else goes in
     * the register. New in ATTinyCore 2.0.0  - saves around 30 clock cycles
     * and 24 bytes of flash, more on some parts; turns out a << 6 takes more
     * work than you'd think; integer promotion hurts. */
    #if defined(__AVR_ATtinyX61__)
      // This one is wacky - with pre-shifting analog reference looks like:
      // REFS1 | REFS0 | - | REFS2 | - | - | - | -

      ADMUX = ((analog_reference & 0xC0) | (pin & 0x1F));
      // 1 ref bit, 1 mux bit, and the gain sel bit are scattered in ADCSRB, and we can't blow away it's contents because thats where we store unipolar vs bipolar mode setting too...
      // ADCSRB     BIN | GSEL | - | REFS2 | MUX5 | ADTS2 | ADTS1 | ADTS0
      // BIN stands for Bipolar INput, not BINary
      ADCSRB = (ADCSRB & (0xA7)) | (analog_reference & 0x10) | (pin & 0x40) | ((pin & 0x20) >> 2);
      // ADCSRB = (ADCSRB & (0xA7)) | (analog_reference & 0x10) | (pin & 0x40) | ((pin & 0x20)?0x08:0);
    #elif defined(ADMUXB)
      #if defined(GSEL0)
        // x41
        ADMUXA = pin & 0x3f;
        ADMUXB = (analog_reference & ADMUX_REFS_MASK) | analog_gain;
      #else
        // 828
        ADMUXA = pin & 0x1f;
        ADMUXB = analog_reference & ADMUX_REFS_MASK;
      #endif
    #elif defined(ADMUX)
      ADMUX = ((analog_reference & ADMUX_REFS_MASK) | (pin & ADMUX_MUX_MASK)); //select the channel and reference
      #ifdef AMISCR
        //ATtiny87/167 external reference is selected by AREFEN bit in AMISCR. Internal reference is output if XREFEN bit is set.
        // | - | - | - | - | - | AREFEN | XREFEN | ISRCEN |
        AMISCR = (AMISCR & 0x01) | (analog_reference & 0x06);
      #endif
      #if defined(__AVR_ATtinyX5__)
        ADCSRB = (ADCSRB & (~(1 << IPR))) | (pin & (1 << IPR));
      #endif
    #endif
    #if defined(SLEEP_MODE_ADC)
      if (use_noise_reduction) {
        ADCSRA |= (1 << ADIE);
        set_sleep_mode(SLEEP_MODE_ADC);
        sleep_mode();
        ADCSRA &= ~(1 << ADIE); // Is it worth unsetting this?
      } else {
        ADCSRA |= (1<<ADSC);    // Start conversion
      }
    #else
      ADCSRA |= (1<<ADSC);      // Start conversion
    #endif
    while(ADCSRA & (1<<ADSC));  // Wait for conversion to complete.
    // The hell! There is already an ADCW defined that just reads it directly, doing it this way causes the compiler
    // to (despite the freedom to do otherwise) reading them into r24 and r25 in the wrong order, then using 3 eor's
    // to fix that. That is probably the single stupidest thing I've seen the compiler do yet. There is literally no
    // reason it couldn't have read them into the correct registers to begin with!
    // uint8_t low = ADCL;
    // uint8_t high = ADCH;
    // return (high << 8) | low;
    // On further study, I think this was due to the core authors reading the part about how you have o read ADCL before
    // ADCH and not having faith that the compiler would generate code that does that when asked to read ADCW.
    // I *do* have faith, however, for three reasons
    // 1. That the header supplied by microchip defines ADCW and deosn't warn abot this implies that the ssue
    // probably doesn't manifest when used reasonably.
    // 2. There is no plausible reason the compiler might feel it needed to access the regsters backards (nor could I get
    // it to generate unsafe code)
    // 3. Even if it did, it would only be a problem in weirdo atypical use cases that violate the expectations of the
    // the core (such that the user at that point is already, at best, in "it is the user's responsibility to ensure"
    // zone - and possbly nasal-demon territory) and I stll have trouble contriving a situation where ti ould manifest
    // bad behavior even then... especially with, worst case, the low byte being read in the next system clock (and
    // like I said, I've never actually  seen code like that; concerns over that are I think entirely imaginary.)
    return ADCW;
  #endif
}

/* SUPER_PWM enables much more flexible, powerful PWM on the two parts with TOCC O=outputs.
 * This is far more powerful than the Timer1 Flexible pwm on the x7.... these are n
 * On the 16-bit timer(s), we put them into WGM 13 instead of the normal mode.
 *
 * setTimernTOP(value - 1-65535) - sets top of Timern
 * setTimernPS(value 1 - 5) - sets prescaler to 1, 8, 64, 256, or 1024. 0 stops clock, and 6/7 use external
 * 0 may be useful, but 6/7 almost certainly are not. Note that these timers are synchronous so
 * you must meet nyquist criterea for them to work here; they're not like the TCDs on the latest parts.
 * Not ready yet.
 */
/*
#if defined(SUPER_PWM)
  #if defined(DISABLE_MILLIS)
    inline void setTimer1PS(uint8_t psval) {
      TCCR1B = TCCR2B & 0xF8 | (psval & 0x07);
    }
  #endif
  inline void setTimer1TOP(uint16_t val) {
    ICR1 = val;
  }
  inline void setTimer1PS(uint8_t psval) {
    TCCR1B = TCCR2B & 0xF8 | (psval & 0x07);
  }
  #if defined(TCCR2A)
    inline void setTimer2TOP(uint16_t val) {
      ICR2 = val;
    }
    inline void setTimer2PS(uint8_t psval) {
      TCCR2B = TCCR2B & 0xF8 | (psval & 0x07);
    }
  #endif
  #if defined(TCCR2A)
    // on ATTiny841 A/Bness is fixed per pin
    #define PWM_TIMER0 0
    #define PWM_TIMER1 0x55
    #define PWM_TIMER2 0xAA
  #else
    // on ATtiny828 it's not, but there's only 4 channels
    #define PWM_TIMER0A 0
    #define PWM_TIMER0B 0x55
    #define PWM_TIMER1A 0xAA
    #define PWM_TIMER1B 0xFF
  #endif


  bool setPWMTimer(uint8_t pin, uint8_t newtimer, bool live) {
    uint8_t timer=digitalPinToTimer(pin)
    if (timer == NOT_ON_TIMER) return false;
    uint8_t tmsk = (timer & 0xCC)? 0xF0 : 0x0F;
    tmsk &= (timer & 0xAA) ? 0xCC : 0x33;
    newtimer &= tmsk; //suddenly those wacky values make sense!
    if (!live) {
      TOCPMCOE &= ~timer;
    }
    if (timer > 0x0F)
      TOCPMSA1 = (TOCPMSA1 & ~tmsk) | newtimer;
    else
      TOCPMSA0 = (TOCPMSA0 & ~tmsk) | newtimer;
    return true;
  }


  // SUPER implementation of analogWrite()
  void analogWrite(uint8_t pin, uint16_t val)
  {
    pinMode(pin, OUTPUT);
    //  (once we determine which timer it's on
    // we check for 255 if timer 0 - since if we stuff too large of a value into an 8-bit timer
    // what we want if a duty cycle > TOP is passed is to just stay on, not turn off
    // (val & 0xFF)/TOP of the way through the cycle
    if (val == 0) {
      digitalWrite(pin, LOW);
#else // Normal, non-super entrance to analogWrite();





    // 828 and x41 get their own implementation

    #if defined(SUPER_PWM)
      if (timer != 0) {
        uint8_t tmsk = timer & 0xCC?0xF0:0x0F;
        tmsk &= (timer & 0x55) ? 0x33 : 0xCC;
        if (timer > 0x0F)
          tmsk &= TOCPMSA1;
        else
          tmsk &= TOCPMSA0;
        #if defined(__AVR_ATtinyX41__)
          if (tmsk == 0) {
            if (val > 255) {digitalWrite(pin,HIGH);} //
            else {
              if (timer & 0x55) OCR0B = val; // odd TOCCs are channel B
              else OCR0A = val;
            }
          } else if (tmsk & 0xAA) {
            if (timer & 0x55) OCR2B = val; // odd TOCCs are channel B
            else OCR2A = val;
          } else {
            if (timer & 0x55) OCR1B = val; // odd TOCCs are channel B
            else OCR1A = val;
          }
        #else // 828
          if (tmsk & 0xAA) { //a high bit in the TOCMPSA is set, hence timer 1
            if (tmsk & 0x55) OCR1B = val; // low bit set, hence channel B
            else OCR1A = val;
          } else {
            if (tmsk & 0x55) OCR0B = val; // low bit set, hence channel B
            else OCR0A = val;
          }
        #endif
        TOCPMCOE |= timer;
    #else // non SUPER







*/


void analogWrite(uint8_t pin, int val) {
  if(__builtin_constant_p(pin)) {
    // No stupid exception here - nobody analogWrite()'s to pins that don't exist!
    if (pin >= NUM_DIGITAL_PINS) badArg("analogWrite to constant pin number that is not a valid pin");
  }
  // let's wait until the end to set pinMode - why output an unknown value for a few dozen clock cycles while we sort out the pwm channel?
  if (val <= 0) {
    digitalWrite(pin, LOW);
  } else if (val >= 255) {
    digitalWrite(pin, HIGH);
  } else {
    uint8_t timer = digitalPinToTimer(pin);
    #if defined(TOCPMCOE)
      if (timer != 0) {
        uint8_t oechan = (timer & 0x08)? 0xF0 & timer : timer >> 4;
        // oechan
        timer &= 0x07;
        switch (timer) {
          case TIMER0A:
            OCR0A = val;
            break;
          case TIMER1A:
            OCR1A = val;
            break;
          case TIMER1B:
            OCR1B = val;
            break;
          #if defined(TCCR2A)
            // only test for these cases on x41
            case TIMER2A:
              OCR2A = val;
              break;
            case TIMER2B:
              OCR2B = val;
              break;
            //end of x41-only
          #endif
          //case TIMER0B:
          default:
            // if it's not 0, and it's not one of the other timers, it's gotta be TIMER0B.
            OCR0B = val;
            break;
        }
        // In any event we can now switch OE for that pin.
        TOCPMCOE |= oechan;
      } else // has to end with this, from if (timer != 0) and it matches up with a block around testing if val > 128 to decide what to write the pin.
    #else //Non-TOCPMCOE implementation
      // Timer0 has a output compare channel A (most parts)
      #if defined(TCCR0A) && defined(COM0A1)
        if (timer == TIMER0A) {
          // connect pwm to pin on timer 0, channel A
          TCCR0A |= (1<<COM0A1);
          OCR0A = val; // set pwm duty
        } else
      #endif
      // Timer0 has a output compare channel B (most parts)
      #if defined(TCCR0A) && defined(COM0B1)
        if (timer == TIMER0B) {
          // connect pwm to pin on timer 0, channel B
          TCCR0A |= (1<<COM0B1);
          OCR0B = val; // set pwm duty
        } else
      #endif

      // TCCR1E is present only on tinyx61 and tinyx7 and there's no TCCR1A on Tiny85
      // Hence this line is approximately "If Timer1 has PWM, and isn't some wacky thing"
      #if defined(TCCR1A) && defined(COM1A1) && !defined(TCCR1D)
        //So this handles "normal" timers
        if (timer == TIMER1A) {
          // connect pwm to pin on timer 1, channel A
          TCCR1A |= (1<<COM1A1);
          OCR1A = val; // set pwm duty
        } else
      #endif
      #if defined(TCCR1A) && defined(COM1B1) && !defined(TCCR1D)
        if (timer == TIMER1B) {
          // connect pwm to pin on timer 1, channel B
          TCCR1A |= (1<<COM1B1);
          OCR1B = val; // set pwm duty
        } else
      #endif
      // Handle the Timer1 flexible PWM on the x7
      #if !defined(TCCR1E) && defined(TCCR1D)
        if (timer&0x08) {
          //Timer 1
          TCCR1A |= (1<<COM1B1)|(1<<COM1A1);
          if (timer&0x04) {
            OCR1B = val;
          } else {
            OCR1A = val;
          }
          TCCR1D |= (1<<(timer&0x07));
        } else
      #endif
      // ATtiny x61
      #if defined(TCCR1E) //Tiny861
        if (timer == TIMER1A) {
          // connect pwm to pin on timer 1, channel A
          TCCR1C |= (1<<COM1A0S);
          OCR1A = val; // set pwm duty
        } else if (timer == TIMER1B) {
          // connect pwm to pin on timer 1, channel B
          TCCR1C |= (1<<COM1B0S);
          OCR1B = val; // set pwm duty
        } else if (timer == TIMER1D) {
          // connect pwm to pin on timer 1, channel D
          TCCR1C |= (1<<COM1D0);
          OCR1D = val; // set pwm duty
        } else
      #endif
      // ATtiny x5
      #if defined(TCCR1) && defined(COM1A1)
        if (timer == TIMER1A) {
          // connect pwm to pin on timer 1, channel A
          TCCR1 |= (1<<COM1A1);
          OCR1A = val; // set pwm duty
        } else
      #endif
      // ATtiny x5
      #if defined(TCCR1) && defined(COM1B1)
        if (timer == TIMER1B) {
          // connect pwm to pin on timer 1, channel B
          GTCCR |= (1<<COM1B1);
          OCR1B = val; // set pwm duty
        } else
      #endif
    #endif // end non-TOCPMCOE implementation
    {
      if (val < 128) {
        digitalWrite(pin, LOW);
      } else {
        digitalWrite(pin, HIGH);
      }
    }
  }
  pinMode(pin,OUTPUT);
}
