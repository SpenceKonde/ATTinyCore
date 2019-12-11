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
  Modified 2015 for Attiny841/1634/828 and for uart clock support S. Konde
*/

#include "core_build_options.h"
#include "core_adc.h"
#include "core_timers.h"
#include "wiring_private.h"
#include "ToneTimer.h"

#define millistimer_(t)                           TIMER_PASTE_A( timer, TIMER_TO_USE_FOR_MILLIS, t )
#define MillisTimer_(f)                           TIMER_PASTE_A( Timer, TIMER_TO_USE_FOR_MILLIS, f )
#define MILLISTIMER_(c)                           TIMER_PASTE_A( TIMER, TIMER_TO_USE_FOR_MILLIS, c )

#define MillisTimer_SetToPowerup                  MillisTimer_(SetToPowerup)
#define MillisTimer_SetWaveformGenerationMode     MillisTimer_(SetWaveformGenerationMode)
#define MillisTimer_GetCount                      MillisTimer_(GetCount)
#define MillisTimer_IsOverflowSet                 MillisTimer_(IsOverflowSet)
#define MillisTimer_ClockSelect                   MillisTimer_(ClockSelect)
#define MillisTimer_EnableOverflowInterrupt       MillisTimer_(EnableOverflowInterrupt)
#define MILLISTIMER_OVF_vect                      MILLISTIMER_(OVF_vect)


#if F_CPU >= 3000000L
  #define MillisTimer_Prescale_Index  MillisTimer_(Prescale_Value_64)
  #define MillisTimer_Prescale_Value  (64)
  #define ToneTimer_Prescale_Index    ToneTimer_(Prescale_Value_64)
  #define ToneTimer_Prescale_Value    (64)
#else
  #define MillisTimer_Prescale_Index  MillisTimer_(Prescale_Value_8)
  #define MillisTimer_Prescale_Value  (8)
  #define ToneTimer_Prescale_Index    ToneTimer_(Prescale_Value_8)
  #define ToneTimer_Prescale_Value    (8)
#endif

// the prescaler is set so that the millis timer ticks every MillisTimer_Prescale_Value (64) clock cycles, and the
// the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_MILLIS_OVERFLOW (clockCyclesToMicroseconds(MillisTimer_Prescale_Value * 256))

// the whole number of milliseconds per millis timer overflow
#define MILLIS_INC (MICROSECONDS_PER_MILLIS_OVERFLOW / 1000)

// the fractional number of milliseconds per millis timer overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_MILLIS_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

#ifndef DISABLEMILLIS

volatile unsigned long millis_timer_overflow_count = 0;
volatile unsigned long millis_timer_millis = 0;
static unsigned char millis_timer_fract = 0;

ISR(MILLISTIMER_OVF_vect)
{
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
  unsigned long m = millis_timer_millis;
  unsigned char f = millis_timer_fract;

/* rmv: The code below generates considerably less code (empty Sketch is 326 versus 304)...

  m += MILLIS_INC;
  f += FRACT_INC;
  if (f >= FRACT_MAX) {
    f -= FRACT_MAX;
    m += 1;
  }
...rmv */

  f += FRACT_INC;

  if (f >= FRACT_MAX)
  {
    f -= FRACT_MAX;
    m = m + MILLIS_INC + 1;
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
  #error Millis()/Micros() timer not defined
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
#if (MillisTimer_Prescale_Value % clockCyclesPerMicrosecond() == 0 ) // Can we just do it the naive way? If so great!
  return ((m << 8) + t) * (MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
  // Otherwise we do clock-specific calculations
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

#else

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

/* Delay for the given number of microseconds.  Assumes a 1,8,12,16,20 or 24 MHz clock. */
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
  // of the function call takes 17 (19) cycles, which is approx. 1us
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


static void initToneTimerInternal(void)
{
  // Stop the clock while we make changes
  ToneTimer_ClockSelect( ToneTimer_(Stopped) );

  // Set the timer to phase-correct PWM
  #if defined( TONETIMER_SUPPORTS_PHASE_CORRECT_PWM ) && TONETIMER_SUPPORTS_PHASE_CORRECT_PWM
    ToneTimer_SetWaveformGenerationMode( ToneTimer_(Phase_Correct_PWM_FF) );
  #else
    ToneTimer_SetWaveformGenerationMode( ToneTimer_(Fast_PWM_FF) );
  #endif

  // Timer is processor clock divided by ToneTimer_Prescale_Index (64)
  ToneTimer_ClockSelect( ToneTimer_Prescale_Index );
}

#if defined (__AVR_ATtinyX41__)
static void initTimer841(void)
{
  #if(TIMER_TO_USE_FOR_TONE==1)
  Timer2_ClockSelect(0);
  Timer2_SetWaveformGenerationMode(1);
  Timer2_ClockSelect(3);
  #else
  Timer1_ClockSelect(0);
  Timer1_SetWaveformGenerationMode(1);
  Timer1_ClockSelect(3);

  #endif

  TOCPMSA0=0b00010000;
  TOCPMSA1=0b10100100;
  TOCPMCOE=0b11111100;

}
#endif

void initToneTimer(void)
{
  // Ensure the timer is in the same state as power-up
  ToneTimer_SetToPowerup();

  #if defined( INITIALIZE_SECONDARY_TIMERS ) && INITIALIZE_SECONDARY_TIMERS
    // Prepare the timer for PWM
    initToneTimerInternal();
  #endif
}

void init(void)
{
  // this needs to be called before setup() or some functions won't work there
  #if (F_CPU==4000000L && CLOCK_SOURCE==0)
  cli();
  #ifdef CCP
  CCP=0xD8; //enable change of protected register
  #else
  CLKPR=1<<CLKPCE; //enable change of protected register
  #endif
  CLKPR=1; //prescale by 2 for 4MHz
  #endif
  sei();
  // In case the bootloader left our millis timer in a bad way
  #if defined( HAVE_BOOTLOADER ) && HAVE_BOOTLOADER
    MillisTimer_SetToPowerup();
  #endif

  // Use the Millis Timer for fast PWM
  MillisTimer_SetWaveformGenerationMode( MillisTimer_(Fast_PWM_FF) );

  // Millis timer is always processor clock divided by MillisTimer_Prescale_Value (64)
  MillisTimer_ClockSelect( MillisTimer_Prescale_Index );

  // Enable the overflow interrupt (this is the basic system tic-toc for millis)
  #ifndef DISABLEMILLIS
  MillisTimer_EnableOverflowInterrupt();
  #endif

  // Initialize the timer used for Tone
  #if defined( INITIALIZE_SECONDARY_TIMERS ) && INITIALIZE_SECONDARY_TIMERS
    initToneTimerInternal();
    #if defined(__AVR_ATtinyX41__)
      initTimer841();
    #endif
    #if defined(__AVR_ATtiny828__)
      TOCPMSA0=0b00010000;
      TOCPMSA1=0b10100100;
      TOCPMCOE=0b11111100;
    #endif
  #endif



  // Initialize the ADC
  #if defined( INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER ) && INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
    ADC_PrescalerSelect( ADC_ARDUINO_PRESCALER );
    ADC_Enable();
  #endif
}
