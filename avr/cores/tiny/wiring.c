/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
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

  $Id: wiring.c 970 2010-05-25 20:16:15Z dmellis $

  Modified 28-08-2009 for attiny84 R.Wiersma
  Modified 14-10-2009 for attiny45 Saposoft
  Modified 20-11-2010 - B.Cook - Rewritten to use the various Veneers.
*/

#include "wiring_private.h"
#include <avr/boot.h>



#if F_CPU >= 3000000L

  #if defined(__AVR_ATtiny167__) || defined(__AVR_ATtiny87__)
    #if F_CPU < 8000000L // 4 and 6 MHz get PWM within the target range of 500-1kHz now on the one pin that is driven by timer0.
      #define timer0Prescaler (0b011)
      #define timer0_Prescale_Value  (32)
    #else
      #define timer0Prescaler (0b100)
      #define timer0_Prescale_Value  (64)
    #endif
  #else
    #define timer0Prescaler (0b011)
    #define timer0_Prescale_Value  (64)
  #endif
  #if (defined(TCCR1) || defined(TCCR1E)) // x5 and x61
    #if F_CPU < 8000000L // 4 and 6 MHz get PWM within the target range of 500-1kHz now on 2 pins of t85, and all PWM pins of the x61, since it's weirdo timer0 has "output" compare that just fires an ISR...
      #define timer1Prescaler (0b0110)
      #define timer1_Prescale_Value  (32)
    #else
      #define timer1Prescaler (0b0111)
      #define timer1_Prescale_Value  (64)
    #endif
  #else
    #define timer1Prescaler (0b011)
    #define timer1_Prescale_Value  (64)
  #endif
#else // 1 or 2 MHz system clock
  #define timer0Prescaler (0b010)
  #if (defined(TCCR1) || defined(TCCR1E))
    #define timer1Prescaler (0b0100)
  #else
    #define timer1Prescaler (0b010)
  #endif
  #define timer0_Prescale_Value    (8)
  #define timer1_Prescale_Value    (8)
#endif

#if (TIMER_TO_USE_FOR_MILLIS == 0)
  #define MillisTimer_Prescale_Value  (timer0_Prescale_Value)
  #define ToneTimer_Prescale_Value    (timer1_Prescale_Value)
  #define MillisTimer_Prescale_Index  (timer0Prescaler)
  #define ToneTimer_Prescale_Index    (timer1Prescaler)
#else
  #warning "WARNING: Use of Timer1 for millis has been configured - this option is untested and unsupported!"
  #define MillisTimer_Prescale_Value  (timer1_Prescale_Value)
  #define ToneTimer_Prescale_Value    (timer0_Prescale_Value)
  #define MillisTimer_Prescale_Index  (timer1Prescaler)
  #define ToneTimer_Prescale_Index    (timer0Prescaler)
#endif

#if F_CPU > 12000000L
  // above 12mhz, prescale by 128, the highest prescaler available
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU >= 6000000L
  // 12 MHz / 64 ~= 188 KHz
  // 8 MHz / 64 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B110
#elif F_CPU >= 3000000L
  // 4 MHz / 32 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B101
#elif F_CPU >= 1500000L
  // 2 MHz / 16 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B100
#elif F_CPU >= 750000L
  // 1 MHz / 8 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B011
#elif F_CPU < 400000L
  // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
  #define ADC_ARDUINO_PRESCALER   B000
#else //speed between 400khz and 750khz
  #define ADC_ARDUINO_PRESCALER   B010 //prescaler of 4
#endif

// the prescaler is set so that the millis timer ticks every MillisTimer_Prescale_Value (64) clock cycles, and the
// the overflow handler is called every 256 ticks.
#if (F_CPU==12800000)
//#define MICROSECONDS_PER_MILLIS_OVERFLOW (clockCyclesToMicroseconds(MillisTimer_Prescale_Value * 256))
//#define MICROSECONDS_PER_MILLIS_OVERFLOW ((64 * 256)/12.8) = 256*(64/12.8) = 256*5 = 1280
#define MICROSECONDS_PER_MILLIS_OVERFLOW (1280)
#elif (F_CPU==16500000)
#define MICROSECONDS_PER_MILLIS_OVERFLOW (992)
#else
#define MICROSECONDS_PER_MILLIS_OVERFLOW (clockCyclesToMicroseconds(MillisTimer_Prescale_Value * 256))
#endif
// the whole number of milliseconds per millis timer overflow
#define MILLIS_INC (MICROSECONDS_PER_MILLIS_OVERFLOW / 1000)

// the fractional number of milliseconds per millis timer overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_MILLIS_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

#if INITIALIZE_SECONDARY_TIMERS
static void initToneTimerInternal(void);
#endif

#ifndef DISABLEMILLIS
  volatile unsigned long millis_timer_overflow_count = 0;
  volatile unsigned long millis_timer_millis = 0;
  static unsigned char millis_timer_fract = 0;
  #if (TIMER_TO_USE_FOR_MILLIS == 0)
    #if defined(TIMER0_OVF_vect)
      ISR(TIMER0_OVF_vect)
    #elif defined(TIM0_OVF_vect)
      ISR(TIM0_OVF_vect)
    #else
      #error "cannot find Millis() timer overflow vector"
    #endif
  #elif (TIMER_TO_USE_FOR_MILLIS == 1)
    #if defined(TIMER1_OVF_vect)
      ISR(TIMER1_OVF_vect)
    #elif defined(TIM1_OVF_vect)
      ISR(TIM1_OVF_vect)
    #else
      #error "cannot find Millis() timer overflow vector"
    #endif
  #else
    #error "Millis() timer not defined!"
  #endif
  {
    // copy these to local variables so they can be stored in registers
    // (volatile variables must be read from memory on every access)
    unsigned long m = millis_timer_millis;
    unsigned char f = millis_timer_fract;

    f += FRACT_INC;

    if (f >= FRACT_MAX)
    {
      f -= FRACT_MAX;
    m += 1;
      m += MILLIS_INC;
    }
    else
    {
      m += MILLIS_INC;
    }

    millis_timer_fract = f;
    millis_timer_millis = m;
    millis_timer_overflow_count++;

  }

  unsigned long millis()
  {
    unsigned long m;
    uint8_t oldSREG = SREG;

    // disable interrupts while we read millis_timer_millis or we might get an
    // inconsistent value (e.g. in the middle of a write to millis_timer_millis)
    cli();
    m = millis_timer_millis;
    SREG = oldSREG;

    return m;
  }

  unsigned long micros()
  {
    unsigned long m;
    uint8_t oldSREG = SREG, t;

    cli();
    m = millis_timer_overflow_count;
  #if defined(TCNT0) && (TIMER_TO_USE_FOR_MILLIS == 0) && !defined(TCW0)
    t = TCNT0;
  #elif defined(TCNT0L) && (TIMER_TO_USE_FOR_MILLIS == 0)
    t = TCNT0L;
  #elif defined(TCNT1) && (TIMER_TO_USE_FOR_MILLIS == 1)
    t = TCNT1;
  #elif defined(TCNT1L) && (TIMER_TO_USE_FOR_MILLIS == 1)
    t = TCNT1L;
  #else
    #error "Millis()/Micros() timer not defined"
  #endif

  #if defined(TIFR0) && (TIMER_TO_USE_FOR_MILLIS == 0)
    if ((TIFR0 & _BV(TOV0)) && (t < 255))
      m++;
  #elif defined(TIFR) && (TIMER_TO_USE_FOR_MILLIS == 0)
    if ((TIFR & _BV(TOV0)) && (t < 255))
      m++;
  #elif defined(TIFR1) && (TIMER_TO_USE_FOR_MILLIS == 1)
    if ((TIFR1 & _BV(TOV1)) && (t < 255))
      m++;
  #elif defined(TIFR) && (TIMER_TO_USE_FOR_MILLIS == 1)
    if ((TIFR & _BV(TOV1)) && (t < 255))
      m++;
  #endif

    SREG = oldSREG;


  #if F_CPU < 1000000L
    return ((m << 8) + t) * MillisTimer_Prescale_Value * (1000000L/F_CPU);
  #else
    #if (MillisTimer_Prescale_Value % clockCyclesPerMicrosecond() == 0 && (F_CPU % 1000000 == 0 )) // Can we just do it the naive way? If so great!
      return ((m << 8) + t) * (MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
      // Otherwise we do clock-specific calculations
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU == 12800000L) //64/12.8=5, but the compiler wouldn't realize it because of integer math - this is a supported speed for Micronucleus.
      return ((m << 8) + t) * 5;
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU == 16500000L) //(16500000) - (16500000 >> 5) = approx 16000000
      m = (((m << 8) + t) << 2 ); // multiply by 4 - this gives us the value it would be if it were 16 MHz
      return (m - (m>>5));        // but it's not - we want 32/33nds of that. We can't divide an unsigned long by 33 in a time sewnsitive function. So we do 31/32nds, and that's goddamned close.
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU == 24000000L) // 2.6875 vs real value 2.67
      m = (m << 8) + t;
      return (m<<1) + (m >> 1) + (m >> 3) + (m >> 4); // multiply by 2.6875
    #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 20) // 3.187 vs real value 3.2
      m=(m << 8) + t;
      return m+(m<<1)+(m>>2)-(m>>4);
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU == 18432000L) // 3.5 vs real value 3.47
      m=(m << 8) + t;
      return m+(m<<1)+(m>>1);
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU==14745600L) //4.375  vs real value 4.34
      m=(m << 8) + t;
      return (m<<2)+(m>>1)-(m>>3);
    #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 14) //4.5 - actual 4.57 for 14.0mhz, 4.47 for the 14.3 crystals scrappable from everything
      m=(m << 8) + t;
      return (m<<2)+(m>>1);
    #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 12) // 5.3125 vs real value 5.333
      m=(m << 8) + t;
      return m+(m<<2)+(m>>2)+(m>>4);
    #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 11) // 5.75 vs real value 5.818 (11mhz) 5.78 (11.059)
      m=(m << 8) + t;
      return m+(m<<2)+(m>>1)+(m>>2);
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU==7372800L) // 8.625, vs real value 8.68
      m=(m << 8) + t;
      return (m<<3)+(m>>2)+(m>>3);
    #elif (MillisTimer_Prescale_Value == 64 && F_CPU==6000000L) // 10.625, vs real value 10.67
      m=(m << 8) + t;
      return (m<<3)+(m<<1)+(m>>2)+(m>>3);
    #elif (MillisTimer_Prescale_Value == 32 && F_CPU==7372800L) // 4.3125, vs real value 4.34 - x7 now runs timer0 twice as fast at speeds under 8 MHz
      m=(m << 8) + t;
      return (m<<2)+(m>>3)+(m>>4);
    #elif (MillisTimer_Prescale_Value == 32 && F_CPU==6000000L) // 5.3125, vs real value 5.33 - x7 now runs timer0 twice as fast at speeds under 8 MHz
      m=(m << 8) + t;
      return (m<<2)+(m)+(m>>3)+(m>>4);
    #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 9) // For 9mhz, this is a little off, but for 9.21, it's very close!
      return ((m << 8) + t) * (MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
    #else
      //return ((m << 8) + t) * (MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
      //return ((m << 8) + t) * MillisTimer_Prescale_Value / clockCyclesPerMicrosecond();
      //Integer division precludes the above technique.
      //so we have to get a bit more creative.
      //We can't just remove the parens, because then it will overflow (MillisTimer_Prescale_Value) times more often than unsigned longs should, so overflows would break everything.
      //So what we do here is:
      //the high part gets divided by cCPuS then multiplied by the prescaler. Then take the low 8 bits plus the high part modulo-cCPuS to correct for the division, then multiply that by the prescaler value first before dividing by cCPuS, and finally add the two together.
      //return ((m << 8 )/clockCyclesPerMicrosecond()* MillisTimer_Prescale_Value) + ((t+(((m<<8)%clockCyclesPerMicrosecond())) * MillisTimer_Prescale_Value / clockCyclesPerMicrosecond()));
      return ((m << 8 )/clockCyclesPerMicrosecond()* MillisTimer_Prescale_Value) + (t * MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
    #endif
  #endif
  }



  static void __empty() {
    // Empty
  }
  void yield(void) __attribute__ ((weak, alias("__empty")));

  void delay(unsigned long ms)
  {
    #if (F_CPU>=1000000L)
    uint16_t start = (uint16_t)micros();

    while (ms > 0) {
      yield();
      while (((uint16_t)micros() - start) >= 1000 && ms) {
        ms--;
        start += 1000;
      }
    }
    #else
    uint32_t start = millis();
    while((millis() - start) < ms)  /* NOP */yield();
    return;
    #endif
  }

#else //if DISABLEMILLIS is set, need no millis, micros, and different delay

  static void __empty() {
    // Empty
  }
  void yield(void) __attribute__ ((weak, alias("__empty")));

  void delay(unsigned long ms) //non-millis-timer-dependent delay()
  {
    while(ms--){
      yield();
      delayMicroseconds(1000);
    }
  }
#endif

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)

  // calling avrlib's delay_us() function with low values (e.g. 1 or
  // 2 microseconds) gives delays longer than desired.
  //delay_us(us);
  #if F_CPU >= 24000000L
    // for the 24 MHz clock for the adventurous ones, trying to overclock

    // zero delay fix
    if (!us) return; //  = 3 cycles, (4 when true)

    // the following loop takes a 1/6 of a microsecond (4 cycles)
    // per iteration, so execute it six times for each microsecond of
    // delay requested.
    us *= 6; // x6 us, = 7 cycles

    // account for the time taken in the preceding commands.
    // we just burned 22 (24) cycles above, remove 5, (5*4=20)
    // us is at least 6 so we can subtract 5
    us -= 5; //=2 cycles

  #elif F_CPU >= 20000000L
    // for the 20 MHz clock on rare Arduino boards

    // for a one-microsecond delay, simply return.  the overhead
    // of the function call takes 18 (20) cycles, which is 1us
    __asm__ __volatile__ (
      "nop" "\n\t"
      "nop" "\n\t"
      "nop" "\n\t"
      "nop"); //just waiting 4 cycles
    if (us <= 1) return; //  = 3 cycles, (4 when true)

    // the following loop takes a 1/5 of a microsecond (4 cycles)
    // per iteration, so execute it five times for each microsecond of
    // delay requested.
    us = (us << 2) + us; // x5 us, = 7 cycles

    // account for the time taken in the preceding commands.
    // we just burned 26 (28) cycles above, remove 7, (7*4=28)
    // us is at least 10 so we can subtract 7
    us -= 7; // 2 cycles

  #elif F_CPU >= 18432000L
    // for a one-microsecond delay, simply return.  the overhead
    // of the function call takes 17 (19) cycles, which is aprox. 1us
    __asm__ __volatile__ (
      "nop" "\n\t"
      "nop" "\n\t"
      "nop" "\n\t"
      "nop"); //just waiting 4 cycles

    if (us <= 1) return; //  = 3 cycles, (4 when true)

    // the following loop takes nearly 1/5 (0.217%) of a microsecond (4 cycles)
    // per iteration, so execute it five times for each microsecond of
    // delay requested.
    us = (us << 2) + us; // x5 us, = 7 cycles

    // user wants to wait longer than 9us - here we can use approximation with multiplication
    if (us > 36) { // 3 cycles
      // Since the loop is not accurately 1/5 of a microsecond we need
      // to multiply us by 0,9216 (18.432 / 20)
      us = (us >> 1) + (us >> 2) + (us >> 3) + (us >> 4); // x0.9375 us, = 20 cycles (TODO: the cycle count needs to be validated)

      // account for the time taken in the preceding commands.
      // we just burned 45 (47) cycles above, remove 12, (12*4=48) (TODO: calculate real number of cycles burned)
      // additionally, since we are not 100% precise (we are slower), subtract a bit more to fit for small values
      // us is at least 46, so we can subtract 18
      us -= 19; // 2 cycles
    } else {
      // account for the time taken in the preceding commands.
      // we just burned 30 (32) cycles above, remove 8, (8*4=32)
      // us is at least 10, so we can subtract 8
      us -= 8; // 2 cycles
    }
  #elif F_CPU >= 16000000L
    // for the 16 MHz clock on most Arduino boards

    // for a one-microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 1us
    if (us <= 1) return; //  = 3 cycles, (4 when true)

    // the following loop takes 1/4 of a microsecond (4 cycles)
    // per iteration, so execute it four times for each microsecond of
    // delay requested.
    us <<= 2; // x4 us, = 4 cycles

    // account for the time taken in the preceding commands.
    // we just burned 19 (21) cycles above, remove 5, (5*4=20)
    // us is at least 8 so we can subtract 5
    us -= 5; // = 2 cycles,

  #elif F_CPU >= 12000000L
    // for the 12 MHz clock if somebody is working with USB

    // for a 1 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 1.5us
    if (us <= 1) return; //  = 3 cycles, (4 when true)

    // the following loop takes 1/3 of a microsecond (4 cycles)
    // per iteration, so execute it three times for each microsecond of
    // delay requested.
    us = (us << 1) + us; // x3 us, = 5 cycles

    // account for the time taken in the preceding commands.
    // we just burned 20 (22) cycles above, remove 5, (5*4=20)
    // us is at least 6 so we can subtract 5
    us -= 5; //2 cycles

  #elif F_CPU >= 8000000L
    // for the 8 MHz internal clock

    // for a 1 and 2 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 2us
    if (us <= 2) return; //  = 3 cycles, (4 when true)

    // the following loop takes 1/2 of a microsecond (4 cycles)
    // per iteration, so execute it twice for each microsecond of
    // delay requested.
    us <<= 1; //x2 us, = 2 cycles

    // account for the time taken in the preceding commands.
    // we just burned 17 (19) cycles above, remove 4, (4*4=16)
    // us is at least 6 so we can subtract 4
    us -= 4; // = 2 cycles
  #elif F_CPU >= 6000000L
    // for that unusual 6mhz clock...

    // for a 1 and 2 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 2us
    if (us <= 2) return; //  = 3 cycles, (4 when true)

    // the following loop takes 2/3rd microsecond (4 cycles)
    // per iteration, so we want to add it to half of itself
    us +=us>>1;
    us -= 2; // = 2 cycles

  #elif F_CPU >= 4000000L
    // for that unusual 4mhz clock...

    // for a 1 and 2 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 2us
    if (us <= 2) return; //  = 3 cycles, (4 when true)

    // the following loop takes 1 microsecond (4 cycles)
    // per iteration, so nothing to do here! \o/

    us -= 2; // = 2 cycles


  #else
    // for the 1 MHz internal clock (default settings for common AVR microcontrollers)
    // the overhead of the function calls is 14 (16) cycles
    if (us <= 16) return; //= 3 cycles, (4 when true)
    if (us <= 25) return; //= 3 cycles, (4 when true), (must be at least 25 if we want to subtract 22)

    // compensate for the time taken by the preceding and next commands (about 22 cycles)
    us -= 22; // = 2 cycles
    // the following loop takes 4 microseconds (4 cycles)
    // per iteration, so execute it us/4 times
    // us is at least 4, divided by 4 gives us 1 (no zero delay bug)
    us >>= 2; // us div 4, = 4 cycles
  #endif

  // busy wait
  __asm__ __volatile__ (
    "1: sbiw %0,1" "\n\t" // 2 cycles
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
  // return = 4 cycles
}

// This clears up the timer settings, and then calls the tone timer initialization function (unless it's been disabled - but in this case, whatever called this isn't working anyway!
void initToneTimer(void)
{
  // Ensure the timer is in the same state as power-up
  #if defined(__AVR_ATtiny43__)
    TIMSK1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 0;
    OCR1B = 0;
    TIFR1 = 0x07;
  #elif (TIMER_TO_USE_FOR_TONE == 0)
    // Just zero the registers out, instead of trying to name all the bits, as there are combinations of hardware and settings where that doesn't work
    TCCR0B = 0; //  (0<<FOC0A) | (0<<FOC0B) | (0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
    TCCR0A = 0; // (0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
    // Reset the count to zero
    TCNT0 = 0;
    // Set the output compare registers to zero
    OCR0A = 0;
    OCR0B = 0;
    #if defined(TIMSK)
      // Disable all Timer0 interrupts
      // Clear the Timer0 interrupt flags
      #if defined(TICIE0) // x61-series has an additional input capture interrupt vector...
        TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0)|(1<<TICIE0));
        TIFR = ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0)|(1<<ICF0));
      #else
        TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
        TIFR = ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
      #endif
    #elif defined(TIMSK0)
      // Disable all Timer0 interrupts
      TIMSK0 = 0; //can do this because all of TIMSK0 is timer 0 interrupt masks
      // Clear the Timer0 interrupt flags
      TIFR0 = ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0)); //no ICF0 interrupt on any supported part with TIMSK0
    #endif
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
    // Turn off Clear on Compare Match, turn off PWM A, disconnect the timer from the output pin, stop the clock
    TCCR1 = (0<<CTC1) | (0<<PWM1A) | (0<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (0<<CS10);
    // 0 out TCCR1
    // Turn off PWM A, disconnect the timer from the output pin, no Force Output Compare Match, no Prescaler Reset
    GTCCR &= ~((1<<PWM1B) | (1<<COM1B1) | (1<<COM1B0) | (1<<FOC1B) | (1<<FOC1A) | (1<<PSR1));
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    OCR1C = 0;
    // Disable all Timer1 interrupts
    TIMSK &= ~((1<<OCIE1A) | (1<<OCIE1B) | (1<<TOIE1));
    // Clear the Timer1 interrupt flags
    TIFR = ((1<<OCF1A) | (1<<OCF1B) | (1<<TOV1));
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TCCR1D = 0;
    TCCR1E = 0;
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    OCR1C = 0;
    OCR1D = 0;
    // Disable all Timer1 interrupts
    TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<OCIE1D));
    // Clear the Timer1 interrupt flags
    TIFR = ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<OCF1D));
  #elif (TIMER_TO_USE_FOR_TONE == 1)
    // Normal, well-behaved 16-bit Timer 1.
    // Turn off Input Capture Noise Canceler, Input Capture Edge Select on Falling, stop the clock
    TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
    // TCCR1B=0; But above is compile time known, so optimized out, and will fail if
    // Disconnect the timer from the output pins, Set Waveform Generation Mode to Normal
    TCCR1A = (0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    // TCCR1A = 0, same logic as above
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    // Disable all Timer1 interrupts
    #if defined(TIMSK)
    TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
    // Clear the Timer1 interrupt flags
    TIFR = ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
    #elif defined(TIMSK1)
    // Disable all Timer1 interrupts
    TIMSK1 = 0; //~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
    // Clear the Timer1 interrupt flags
    TIFR1 = ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
    #endif
  #endif

  #if INITIALIZE_SECONDARY_TIMERS
  // Prepare the timer for PWM
    initToneTimerInternal();
  #endif
}

// initToneTimerInternal() - initialize the timer used for tone for PWM

#if INITIALIZE_SECONDARY_TIMERS
  static void initToneTimerInternal(void)
  {
    // Timer is processor clock divided by ToneTimer_Prescale_Index
    #if (TIMER_TO_USE_FOR_TONE == 0)
      // Use the Tone Timer for phase correct PWM
      TCCR0A = (1<<WGM00) | (0<<WGM01);
      TCCR0B = (ToneTimer_Prescale_Index << CS00) | (0<<WGM02);
    #elif defined(__AVR_ATtiny43__)
      TCCR1A = 3; //WGM 10=1, WGM11=1
      TCCR1B = 3; //prescaler of 64
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1) // ATtiny x5
      // Use the Tone Timer for fast PWM as phase correct not supported by this timer
      GTCCR = (1<<PWM1B);
      OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
      TCCR1 = (1<<CTC1) | (1<<PWM1A) | (ToneTimer_Prescale_Index << CS10);
    #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E) // ATtiny x61
      // Use the Tone Timer for phase correct PWM
      TCCR1A = (1<<PWM1A) | (1<<PWM1B);
      TCCR1C = (1<<PWM1D);
      TCCR1D = (1<<WGM10) | (0<<WGM11);
      TCCR1B = (ToneTimer_Prescale_Index << CS10);
    #elif (TIMER_TO_USE_FOR_TONE == 1 ) && defined(__AVR_ATtinyX7__)
      TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM10);
      TCCR1B = (ToneTimer_Prescale_Index << CS10);
    #elif (TIMER_TO_USE_FOR_TONE == 1) // x4, x8, x313,
      // Use the Tone Timer for phase correct PWM
      TCCR1A = (1<<WGM10);
      TCCR1B = (0<<WGM12) | (0<<WGM13) | (ToneTimer_Prescale_Index << CS10); //set the clock
    #endif
  }
#endif


uint8_t read_factory_calibration(void)
{
  uint8_t value = boot_signature_byte_get(1);
  return value;
}

void init(void)
{
  /*
  If clocked from the PLL (CLOCK_SOURCE==6) then there are three special cases all involving
  the 16.5 MHz clock option used to support VUSB on PLL-clocked parts.
  If F_CPU is set to 16.5, and the Micronucleus bootloader is in use (indicated by BOOTTUNED165
  being defined), the bootloader has already set OSCCAL to run at 16.5; if that is set while
  F_CPU is set to 16, we reload that from factory.
  If it is set to 16.5 but BOOTTUNED165 is not set, we're not using the micronucleus bootloader
  and we need to increase OSCCAL; this is a guess, but works better than nothing.
  */

  #if (CLOCK_SOURCE==6) //handle weird frequencies when using the PLL clock
    #if (defined(BOOTTUNED165)) // If it's a micronucleus board, it will either run at 16.5 after
      // adjusting the internal oscillator for that speed (in which case we are done) or we want
      // it to be set to run at 16, in which case we need to reload the factory cal.
      #if (F_CPU!=16500000L) //if not 16.5, it's 16, or that divided by power of two
        OSCCAL = read_factory_calibration(); //we do this if it was tuned by micronucleus, but we don't want USB.
        #if (F_CPU!=16000000) // 16MHz is speed of unprescaled PLL clock - if we don't want that...
          #ifdef CCP
            CCP=0xD8; //enable change of protected register
          #else
            CLKPR=1<<CLKPCE; //enable change of protected register
          #endif
          #if (F_CPU ==8000000L)
            CLKPR=1; //prescale by 2 for 8MHz
          #elif (F_CPU==4000000L)
            CLKPR=2; //prescale by 4 for 4MHz
          #elif (F_CPU==2000000L)
            CLKPR=3; //prescale by 8 for 2MHz
          #elif (F_CPU==1000000L)
            CLKPR=4; //prescale by 16 for 1MHz
          #elif (F_CPU ==500000L)
            CLKPR=5; //prescale by 32 for 0.5MHz
          #elif (F_CPU ==250000L) // these extremely slow speeds are of questionable value.
            CLKPR=6; //prescale by 64 for 0.25MHz
          #elif (F_CPU ==125000L) //
            CLKPR=7; //prescale by 128 for 125kHz
          #elif (F_CPU ==62500L)
            CLKPR=8; //prescale by 256 for 62.50kHz
          #else
            #error "Frequency requested from PLL that cannot be generated by prescaling"
            #error "Custom tuning is not supported in the current version of ATTinyCore"
          #endif // end check for each prescale freq
        #endif //end if not 16
      #endif // end if not 16.5
    #elif (F_CPU == 16500000L) // not pretuned to 16.5 (no micronucleus), but 16.5 requested, presumably for VUSB.
      if (OSCCAL == read_factory_calibration()) {
        // adjust the calibration up from 16.0mhz to 16.5mhz
        if (OSCCAL >= 128) {
          // maybe 8 is better? oh well - only about 0.3% out anyway
          OSCCAL += 7;
        } else {
          OSCCAL += 5;
        }
      }
    #else // We're using PLL, and it's not a VUSB special case.
      #if (F_CPU!=16000000) // 16MHz is speed of unprescaled PLL clock.
        #ifdef CCP
          CCP=0xD8; //enable change of protected register
        #else
          CLKPR=1<<CLKPCE; //enable change of protected register
        #endif
        // One really wonders why someone would use the PLL as clock source if they weren't using VUSB or running at 16MHz, it's got to burn more power...
        #if (F_CPU ==8000000L)
          CLKPR=1; //prescale by 2 for 8MHz
        #elif (F_CPU==4000000L)
          CLKPR=2; //prescale by 4 for 4MHz
        #elif (F_CPU==2000000L)
          CLKPR=3; //prescale by 8 for 2MHz
        #elif (F_CPU==1000000L)
          CLKPR=4; //prescale by 16 for 1MHz
        #elif (F_CPU ==500000L)
          CLKPR=5; //prescale by 32 for 0.5MHz
        #elif (F_CPU ==250000L) // these extremely slow speeds are of questionable value.
          CLKPR=6; //prescale by 64 for 0.25MHz
        #elif (F_CPU ==125000L) //
          CLKPR=7; //prescale by 128 for 125kHz
        #elif (F_CPU ==62500L)
          CLKPR=8; //prescale by 256 for 62.50kHz
        #else
          #error "Frequency requested from PLL that cannot be generated by prescaling"
          #error "Custom tuning is not supported in the current version of ATTinyCore"
        #endif //end handling of individual frequencies
      #endif //end if not 16 MHz (default speed)
    #endif // end check for VUSB-related special cases
  #elif (CLOCK_SOURCE==0 && F_CPU!=8000000L && F_CPU!=1000000L)
    // normal oscillator, we want a setting that fuses won't give us,
    // so need to set prescale.
    #ifdef CCP
      CCP=0xD8; //enable change of protected register
    #else
      CLKPR=1<<CLKPCE; //enable change of protected register
    #endif
    #if (F_CPU ==4000000L)
      CLKPR=1; //prescale by 2 for 4MHz
    #elif (F_CPU ==2000000L)
      CLKPR=2; //prescale by 4 for 2MHz
      // 1 MHz handled by fuse
    #elif (F_CPU ==500000L) // these extremely slow speeds are of questionable value.
      CLKPR=4; //prescale by 16 for 0.5MHz
    #elif (F_CPU ==250000L)
      CLKPR=5; //prescale by 32 for 0.25MHz
    #elif (F_CPU ==125000L) //
      CLKPR=6; //prescale by 64 for 125kHz
    #elif (F_CPU ==62500L)
      CLKPR=7; //prescale by 128 for 62.50kHz
    #elif (F_CPU ==31250L)
      CLKPR=8; //prescale by 256 for 31.25kHz
    #else
      #error "Frequency requested from internal oscillator that cannot be generated by prescaling"
      #error "Custom tuning is not supported in the current version of ATTinyCore"
    #endif
  #elif(CLOCK_SOURCE==17 || CLOCK_SOURCE==18) // external 16MHz CLOCK or Crystal, but maybe they want to go slower to save power...
    #if (F_CPU!=16000000) // 16MHz is speed of external clock on these
      #ifdef CCP
        CCP=0xD8; //enable change of protected register
      #else
        CLKPR=1<<CLKPCE; //enable change of protected register
      #endif
      #if (F_CPU ==8000000L)
        CLKPR=1; //prescale by 2 for 8MHz
      #elif (F_CPU==4000000L)
        CLKPR=2; //prescale by 4 for 4MHz
      #elif (F_CPU==2000000L)
        CLKPR=3; //prescale by 8 for 2MHz
      #elif (F_CPU==1000000L)
        CLKPR=4; //prescale by 16 for 1MHz
      #elif (F_CPU ==500000L)
        CLKPR=5; //prescale by 32 for 0.5MHz
      #elif (F_CPU ==250000L) // these extremely slow speeds are of questionable value.
        CLKPR=6; //prescale by 64 for 0.25MHz
      #elif (F_CPU ==125000L) //
        CLKPR=7; //prescale by 128 for 125kHz
      #elif (F_CPU ==62500L)
        CLKPR=8; //prescale by 256 for 62.50kHz
      #else
        #error "Frequency requested from 16MHz external clock that cannot be generated by prescaling"
      #endif //end handling of individual frequencies
    #endif //end if not 16 MHz (default speed)
  #endif //end handling for the two types of internal oscillator derived clock source and 16MHz ext clock of MH-ET tiny88

/*

  // In case the bootloader left our millis timer in a bad way
  there had been some very dubious code here that reinitialized registers.
  ATTinyCore does not include any bootloaders that mess with timers without ensuring that they are reset.
  so this code has been removed.
*/

  // Use the Millis Timer for fast PWM (unless it doesn't have an output).
  #if (TIMER_TO_USE_FOR_MILLIS == 0)
    #if defined(WGM01) // if Timer0 has PWM
      TCCR0A = (1<<WGM01) | (1<<WGM00);
    #endif
    #if defined(TCCR0B) //The x61 has a wacky Timer0!
      TCCR0B = (MillisTimer_Prescale_Index << CS00);
    #else  //I think this means t's an x8
      TCCR0A = (MillisTimer_Prescale_Index << CS00);
    #endif
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1) //ATtiny x5
    TCCR1 = (1<<CTC1) | (1<<PWM1A) | (MillisTimer_Prescale_Index << CS10);
    GTCCR = (1<<PWM1B);
    OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1E) //ATtiny x61
    TCCR1C = 1<<PWM1D;
    TCCR1B = (MillisTimer_Prescale_Index << CS10);
    TCCR1A = (1<<PWM1A) | (1<<PWM1B);
    //cbi(TCCR1E, WGM10); //fast pwm mode
    //cbi(TCCR1E, WGM11);
    OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
  #elif (TIMER_TO_USE_FOR_MILLIS == 1)
    TCCR1A = 1<<WGM10;
    TCCR1B = (1<<WGM12) | (MillisTimer_Prescale_Index << CS10);
  #endif

  // this needs to be called before setup() or some functions won't work there
  sei();

  #ifndef DISABLEMILLIS
    // Enable the overflow interrupt (this is the basic system tic-toc for millis)
    #if defined(TIMSK) && defined(TOIE0) && (TIMER_TO_USE_FOR_MILLIS == 0)
      sbi(TIMSK, TOIE0);
    #elif defined(TIMSK0) && defined(TOIE0) && (TIMER_TO_USE_FOR_MILLIS == 0)
      sbi(TIMSK0, TOIE0);
    #elif defined(TIMSK) && defined(TOIE1) && (TIMER_TO_USE_FOR_MILLIS == 1)
      sbi(TIMSK, TOIE1);
    #elif defined(TIMSK1) && defined(TOIE1) && (TIMER_TO_USE_FOR_MILLIS == 1)
      sbi(TIMSK1, TOIE1);
    #else
      #error Millis() Timer overflow interrupt not set correctly
    #endif
  #endif
  #if defined(PLLTIMER1) && (!defined(PLLCSR))
    #error "Chip does not have PLL (only x5, x61 series do), yet you somehow selected PLL as timer1 source. If you have not modified the core, please report this to core maintainer."
  #endif
  #ifdef PLLTIMER1 // option on x5 and x61
    if (!PLLCSR) {
      PLLCSR = (1<<PLLE);
        while (!(PLLCSR&1)) {
          ; //wait for lock
        }
      PLLCSR = (1<<PCKE)|(1<<PLLE);
    }
  #endif
  #if defined(LOWPLLTIMER1) && ((CLOCK_SOURCE==6) || (!defined(PLLCSR)))
    #error "LOW SPEED PLL Timer1 clock source is NOT SUPPORTED when PLL is used as system clock source; the bit to enable it cannot be set, per datasheet (section Timer/Counter1->Register Descriotion->PLLCSR, x5 / x61 only) or chip does not have PLL"
  #endif
  #ifdef LOWPLLTIMER1 // option on x5 and x61
    if (!PLLCSR) {
      PLLCSR = (1<<LSM) | (1<<PLLE);
      while (!(PLLCSR&1)) {
        ; //wait for lock
      }
      // faster than |= since we know the value we want (OUT vs )
      PLLCSR = (1<<PCKE)|(1<<LSM)|(1<<PLLE);
    }
  #endif
  // Initialize the timer used for Tone
  #if INITIALIZE_SECONDARY_TIMERS
    initToneTimerInternal();
  #endif

  // Initialize the ADC
  #if defined( INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER ) && INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
    #if defined(ADCSRA)
      // set a2d prescale factor
      // ADCSRA = (ADCSRA & ~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))) | (ADC_ARDUINO_PRESCALER << ADPS0) | (1<<ADEN);
      // dude, this is being called on startup. We know that ADCSRA is 0! Why add a RMW cycle?!
      ADCSRA = (ADC_ARDUINO_PRESCALER << ADPS0) | (1<<ADEN);
      // enable a2d conversions
      // sbi(ADCSRA, ADEN); //we already set this!!!
    #endif
  #endif
}
