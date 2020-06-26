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
#include <avr/boot.h>

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
#elif (MillisTimer_Prescale_Value == 64 && F_CPU == 12800000L)  //64/12.8=5, but the compiler wouldn't realize it because of integer math - this is a supported speed for Micronucleus.
  return ((m << 8) + t) * 5;
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
#elif (MillisTimer_Prescale_Value == 64 && F_CPU == 14745600L) //4.375  vs real value 4.34
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
#if ((F_CPU==16000000 || defined(LOWERCAL)) && CLOCK_SOURCE==0 )

  static uint8_t origOSC=0;

  uint8_t read_factory_calibration(void)
  {
    uint8_t SIGRD = 5; //Yes, this variable is needed. boot.h is looking for SIGRD but the io.h calls it RSIG... (unlike where this is needed in the other half of this core, at least the io.h file mentions it... ). Since it's actually a macro, not a function call, this works...
    uint8_t value = boot_signature_byte_get(1);
    return value;
  }
  #ifdef INSTANT_BOOST
  //Not supposed to be "safe" - but in my testing, it seemed to work fine...
  void oscSlow(uint8_t newcal) {
    OSCCAL0=newcal;
  }
  #else
  //The safe way, per datasheet recommendations.
  void oscSlow(uint8_t newcal) {
    uint8_t i=OSCCAL0;
    // We do it this way so we can avoid constantly having to re-read OSCCAL0; might as well get this done as fast as possible (this whole bit may not even be necessary, but it's what the datasheet says)
    while (i>newcal) {
      OSCCAL0=--i;
    }
  }
  #endif

#endif


#if ((defined(__AVR_ATtinyX41__) && F_CPU==16000000) && CLOCK_SOURCE==0 )
  //functions related to the 16 MHz internal option on ATtiny841/441.
  // 174 CALBOOST was empirically determined from a few parts I had lying around.
  #define CALBOOST 174
  #define MAXINITCAL (255-CALBOOST)
  static uint8_t saveTCNT=0;
  #ifdef INSTANT_BOOST
  //Not supposed to be "safe" - but in my testing, it seemed to work fine...
  void oscBoost() {
    OSCCAL0=(origOSC>MAXINITCAL?255:(origOSC+CALBOOST));
  }
  #else
  void oscBoost() {
    uint8_t i=OSCCAL0;
    // We do it this way so we can avoid constantly having to re-read OSCCAL0; might as well get this done as fast as possible (this whole bit may not even be necessary, but it's what the datasheet says)
    while (i<255 &&i<(origOSC+CALBOOST)) {
      OSCCAL0=++i;
    }
  }
  #endif

  void oscSafeNVM() { //called immediately prior to writing to EEPROM.
    TIMSK0&=~(_BV(TOIE0)); //turn off millis interrupt - let PWM keep running (Though at half frequency, of course!)
    saveTCNT=TCNT0;
    if (TIFR0&_BV(TOV0)) { // might have just missed interrupt - recording as 255 is good enough (this may or may not have been what we recorded, but if it was still set, interrupt didn't fire)
      saveTCNT=255;
    }
    oscSlow(origOSC);
  }
  void oscDoneNVM(uint8_t bytes_written) { // number of bytes of eeprom written.
    // EEPROM does it one at a time, but user code could call these two methods when doing block writes (up to 64 bytes). Just be sure to do the eeprom_busy_wait(); at the end, as in EEPROM.h.
    // 3.3ms is good approximation of the duration of writing a byte - it'll be about 3~4% faaster since we're running around 5V at default call - hence, we're picking 3.3ms - the oscillator
    // adjustment loops and these calculations should be fast enough that the time they dont take long enough to worry about...
    // Srelies on assumptions from implementation above of millis on this part at 16MHz!
    //millis interrupt was disabled when oscSaveNVM() was called - so we don't need to do anything fancy to access the volatile variables related to it.
    //1 millis interrupt fires every 1.024ms, so we want 3.3/1.024= 3.223 overflows; there are 256 timer ticksin an overflow, so 57 timer ticks...
    oscBoost();
    uint8_t m = 3*bytes_written; //the 3 whole overflows
    uint16_t tickcount=57*bytes_written+saveTCNT;
    m+=(tickcount>>8); //overflows from theose extra /0.223's
    millis_timer_overflow_count+=m; //done with actual overflows, so record that.
    uint16_t f = FRACT_INC*m+millis_timer_fract; //(m could be up to 207)
    while(f>FRACT_MAX){ //at most 621+124=745
      f-=FRACT_MAX;
      m++; // now we're adding up the contributions to millis from the 0.024 part...
    }
    // save the results
    millis_timer_fract=f;
    millis_timer_millis+=m;
    TCNT0=0;
    TIFR0|=_BV(TOV0); //clear overflow flag
    TIMSK0|=_BV(TOIE0); //enable overflow interrupt
    TCNT0=tickcount; //restore new tick count
    // wonder if it was worth all that just to write to the EEPROM while running at 16MHz off internal oscillator without screwing up millis and micros...
  }
#endif



void init(void)
{
  // We should take some precaution against accidentally applying the same changes if the sketch restarts without a clean reset - so we base what we change to on the original value, not whatever it happens to be set to.
  #if (defined(__AVR_ATtinyX41__) && CLOCK_SOURCE==0 && F_CPU==16000000L)
    // jumping up about 174 from the factory cal gives us ~16 MHz at 4.5~5.25V - not always perfect, but should generally be close enough.
    origOSC=read_factory_calibration();
    oscBoost();
  #elif (CLOCK_SOURCE==0 && defined(LOWERCAL))
    origOSC=read_factory_calibration();
    oscSlow(origOSC-LOWERCAL);
  #endif
  // this needs to be called before setup() or some functions won't work there
  #if (F_CPU==4000000L && CLOCK_SOURCE==0)
  //Ordinarily, this absolutely must be within cli() / sei() - however at this point in the code, we KNOW it isn't, because we have not enabled any
  //cli();
  #ifdef CCP
  CCP=0xD8; //enable change of protected register
  #else
  CLKPR=1<<CLKPCE; //enable change of protected register
  #endif
  CLKPR=1; //prescale by 2 for 4MHz
  #endif
  //sei();


  /*
  // In case the bootloader left our millis timer in a bad way
  // but none of the supported bootloaders do this - so this can be commented out entirely
  #if defined( HAVE_BOOTLOADER ) && HAVE_BOOTLOADER
    MillisTimer_SetToPowerup();
  #endif
  */
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
