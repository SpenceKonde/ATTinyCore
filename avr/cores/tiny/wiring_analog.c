/*
  wiring_analog.c - analog input and output
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.c 248 2007-02-03 15:36:30Z mellis $

  Modified  28-08-2009 for attiny84 R.Wiersma
  Modified  14-10-2009 for attiny45 Saposoft
  Corrected 17-05-2010 for ATtiny84 B.Cook
*/

#include "wiring_private.h"
#include "pins_arduino.h"

#ifndef DEFAULT
//For those with no ADC, need to define default.
#define DEFAULT (0)
#endif

uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode) {
  // can't actually set the register here because the default setting
  // will connect AVCC and the AREF pin, which would cause a short if
  // there's something connected to AREF.
  // fix? Validate the mode?
  // Huh, well, since the core never calls this. why *can't* we set the reference when the user's code calls this?
  // We can, goddamnit! And this should help deal with those first reads sucking!
  analog_reference = mode;

  #if defined(__AVR_ATtinyX61__)
    //this one is WACKY
    ADMUX = (((analog_reference & 0x03) << 6));
    ADCSRB = (ADCSRB & (0xA7)) | ((analog_reference & 0x04) << 2 );
    // They could have made that harder to deal with if they really tried, maybe...
  #elif defined(ADMUXB)
    #if defined(GSEL0)
      // x41
      ADMUXB = ((analog_reference & 0x07) << REFS0);
    #else
      // 828
      ADMUXB = (analog_reference ? (1 << REFS):0);
    #endif
  #elif defined(ADMUX)
    #if defined(REFS2)
      ADMUX = ((analog_reference & ADMUX_REFS_MASK) << REFS0) | (((analog_reference & 0x04) >> 2) << REFS2); //some have an extra reference bit in a weird position.
    #else
      ADMUX = ((analog_reference & ADMUX_REFS_MASK) << REFS0);  //select the reference
    #endif
  #endif
  #ifdef AMISCR
    //ATtiny87/167
    //external reference is selected by AREF bit in AMISCR. Internal reference is output if XREF bit is set.
    AMISCR = (AMISCR & 0x01) | (analog_reference & 0x0C) >> 1;
  #endif
}


#if defined(REFS1)
  #define ADMUX_REFS_MASK (0x03)
#else
  #define ADMUX_REFS_MASK (0x01)
#endif

#if defined(MUX5)
  #define ADMUX_MUX_MASK (0x3f)
#elif defined(MUX4)
  #define ADMUX_MUX_MASK (0x1f)
#elif defined(MUX3)
  #define ADMUX_MUX_MASK (0x0f)
#else
  #define ADMUX_MUX_MASK (0x07)
#endif

#if defined(__AVR_ATtinyX61__) || defined(__AVR_ATtinyX4__) || defined(__AVR_ATtinyX61__) || defined(__AVR_ATtinyX61__)
  inline __attribute__((always_inline)) void setADCBipolarMode(bool bipolar) {
    ADCSRB = (ADCSRB & (0x7F) ) | (bipolar ? 0x80 : 0);
  }
#elif defined(__AVR_ATtinyX41__)
  inline __attribute__((always_inline)) void setADCBipolarMode(bool bipolar) {
    if (!__builtin_constant_p(bipolar) || !bipolar) {
      badCall("This part only supports bipolar mode.");
    }
  }
#else
  inline __attribute__((always_inline)) void setADCBipolarMode(((unused))bool bipolar) {
    badCall("This part does not have a differential ADC, much less an option for bipolar vs unipolar mode");
  }
#endif
int analogRead(uint8_t pin) {
  _analogRead(pin, false)
}
/* in wiring_analog_noise.c so the ISR is only defined if using this
int analogReadNR(uint8_t pin) {
  _analogRead(pin, true)
}
*/
int _analogRead(uint8_t pin, bool use_noise_reduction)
{
  if (!(pin & 0x80)) {
    digitalPinToAnalogInput(pin);
  }
  if (__builtin_constant_p(pin)) {
    #if defined(__AVR_ATtinyX61__)
      if ((pin & 0x3F) < 32 && (pin & 0x40))
        badArg("Compiletime known ADC channel with gain selection set does not support gain selection");
    #elif defined(__AVR_ATtinyX5__)
      if ((pin & 0x5F > ADMUX_MUX_MASK))
        badArg("Compiletime known ADC channel does not exist on this part");
      if ((pin < 4 || pin > 11 ) && (pin & 0x20))
        badArg("Compiletime known ADC channel not valid: IPR bit only valid for differential channels");
    #else
      if (pin & 0x7F > ADMUX_MUX_MASK)
        badArg("Compiletime known ADC channel does not exist on this part");
    #endif
  }
  #ifndef ADCSRA
    badCall("analogRead() cannot be used on a part without an ADC");
    /* if a device does not have an ADC, instead of giving a number we know is wrong AND that isn't unique to error conditions,
     * let's give a number that will be very obviously an error, and could not be generated if the pin were capable of analogRead()
     * if they do any sort of error checking, they would hopefully verify that analogRead() didn't give them back an obviously erroneous value .
     */
    return -32768;
  #else
    #if defined(__AVR_ATtinyX61__)
      if ((pin & 0x3F) < 32 && (pin & 0x40))  return ADC_NOT_A_CHANNEL;

    #elif defined(__AVR_ATtinyX5__)
      if ((pin & 0x5F > ADMUX_MUX_MASK))      return ADC_NOT_A_CHANNEL;
      if ((pin & 0x20) && ((pin & 0x0F) < 4 \
        || (pin & 0x0F) > 11 ))               return ADC_IPR_USED_WRONG;

    #else
      if (pin & 0x7F > ADMUX_MUX_MASK)        return ADC_NOT_A_CHANNEL

    #endif
  }
    /* I don't think we need to check for this? Can we say it's the responsibility of the user to avoid calling analogRead() if they have turned off the ADC or chose the "don't initialize the ADC"
     * whern compiling? . We can't switch to this on the basis of the compiletime options, because it is very plausible that someone with a highly atypical ADC configuration might want to disable
     * the builtin initialization and do it themselves.
     * On second thought, lets throw it in by default, but categorize ADC_NO_CHECK_STATUS as a "flash-saver" option, and probably enable by default on small chips.
     */
    #if !defined(ADC_NO_CHECK_STATUS)
      uint8_t t = (ADCSRA & ((1 << ADEN) | (1 << ADSC)
      if (!(t == (1 << ADEN))) return (t == (1 << ADSC)) ? -32000 : -30000;
    #endif
    #if defined(__AVR_ATtinyX61__)
      //this one is WACKY
      ADMUX = (((analog_reference & 0x03) << 6) | (pin & 0x1F));
      // 1 ref bit, 1 mux bit, and the gain sel bit are scattered in ADCSRB, and we can't blow away it's contents because thats where we store unipolar vs bipolar mode setting too...
      // ADCSRB     BIN(put)| GSEL | - | REFS2 | MUX5 | ADTS2 | ADTS1 | ADTS0
      // BIN stands for Bipolar INput, not BINary.
      ADCSRB = (ADCSRB & (0xA7)) | ((analog_reference & 0x04) << 2 ) | (pin & 0x40) | ((pin & 0x20) >> 2);
      // They could have made that harder to deal with if they really tried, maybe...
    #elif defined(ADMUXB)
      #if defined(GSEL0)
        // x41
        ADMUXA = pin&&0x3f;
        ADMUXB = ((analog_reference & 0x07) << REFS0);
      #else
        // 828
        ADMUXA = pin & 0x1f;
        ADMUXB = (analog_reference ? (1 << REFS):0);
      #endif
    #elif defined(ADMUX)
      #if defined(REFS2)
        ADMUX = ((analog_reference & ADMUX_REFS_MASK) << REFS0) | ((pin & ADMUX_MUX_MASK) << MUX0) | (((analog_reference & 0x04) >> 2) << REFS2); //some have an extra reference bit in a weird position.
      #else
        ADMUX = ((analog_reference & ADMUX_REFS_MASK) << REFS0) | ((pin & ADMUX_MUX_MASK) << MUX0); //select the channel and reference
        #ifdef AMISCR
          //ATtiny87/167
          //external reference is selected by AREF bit in AMISCR. Internal reference is output if XREF bit is set.
          AMISCR = (AMISCR & 0x01) | (analog_reference & 0x0C) >> 1;
        #endif
      #endif
      #if defined(__AVR_ATtinyX5__)
        ADCSRB = (ADCSRB & (~(1 << IPR))) | (pin & (1 << IPR));
      #endif
    #endif
    // if (use_noise_reduction == 1)
    //
    ADCSRA |= (1<<ADSC);        //Start conversion
    while(ADCSRA & (1<<ADSC));  //Wait for conversion to complete.
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    return (high << 8) | low;
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

inline __attribute__((always_inline)) void check_valid_digital_pin(uint8_t pin) {

}
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
  /* #endif */  // SUPER PWM
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
