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

#if F_CPU >= 3000000L

#if !defined(__AVR_ATtiny167__) && !defined(__AVR_ATtiny87__)
#define timer0Prescaler 0b011
#else
#define timer0Prescaler 0b100
#endif

//Timers with TCCR1 are slightly different.
#if defined(TCCR1) && (TIMER_TO_USE_FOR_MILLIS == 1)
  #define MillisTimer_Prescale_Index  (0b0111)
  #define ToneTimer_Prescale_Index    (timer0Prescaler)
#elif defined(TCCR1) && (TIMER_TO_USE_FOR_MILLIS == 0)
  #define MillisTimer_Prescale_Index  (timer0Prescaler)
  #define ToneTimer_Prescale_Index    (0b0111)
#elif defined(TCCR1E) && (TIMER_TO_USE_FOR_MILLIS == 1)
  #define MillisTimer_Prescale_Index  (0b0111)
  #define ToneTimer_Prescale_Index    (timer0Prescaler)
#elif defined(TCCR1E) && (TIMER_TO_USE_FOR_MILLIS == 0)
  #define MillisTimer_Prescale_Index  (timer0Prescaler)
  #define ToneTimer_Prescale_Index    (0b0111)
#elif (TIMER_TO_USE_FOR_MILLIS == 1)
  #define MillisTimer_Prescale_Index  (0b011)
  #define ToneTimer_Prescale_Index    (timer0Prescaler)
#else
  #define MillisTimer_Prescale_Index  (timer0Prescaler)
  #define ToneTimer_Prescale_Index    (0b011)
#endif

  #define MillisTimer_Prescale_Value  (64)
  #define ToneTimer_Prescale_Value    (64)
  
#else

#if defined(TCCR1) && (TIMER_TO_USE_FOR_MILLIS == 1)
  #define MillisTimer_Prescale_Index  (0b0100)
  #define ToneTimer_Prescale_Index    (0b010)
#elif defined(TCCR1) && (TIMER_TO_USE_FOR_MILLIS == 0)
  #define MillisTimer_Prescale_Index  (0b010)
  #define ToneTimer_Prescale_Index    (0b0100)
#elif defined(TCCR1E) && (TIMER_TO_USE_FOR_MILLIS == 1)
  #define MillisTimer_Prescale_Index  (0b0100)
  #define ToneTimer_Prescale_Index    (0b010)
#elif defined(TCCR1E) && (TIMER_TO_USE_FOR_MILLIS == 0)
  #define MillisTimer_Prescale_Index  (0b010)
  #define ToneTimer_Prescale_Index    (0b0100)
#else
  #define MillisTimer_Prescale_Index  (0b010)
  #define ToneTimer_Prescale_Index    (0b010)
#endif

  #define MillisTimer_Prescale_Value  (8)
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

volatile unsigned long millis_timer_overflow_count = 0;
volatile unsigned long millis_timer_millis = 0;
static unsigned char millis_timer_fract = 0;
#if (TIMER_TO_USE_FOR_MILLIS == 0)

#if defined(TIMER0_OVF_vect)
SIGNAL(TIMER0_OVF_vect)
#elif defined(TIM0_OVF_vect)
SIGNAL(TIM0_OVF_vect)
#else
#error cannot find Millis() timer overflow vector
#endif

#elif (TIMER_TO_USE_FOR_MILLIS == 1)

#if defined(TIMER1_OVF_vect)
SIGNAL(TIMER1_OVF_vect)
#elif defined(TIM1_OVF_vect)
SIGNAL(TIM1_OVF_vect)
#else
#error cannot find Millis() timer overflow vector
#endif

#else

#error Millis() timer not defined!

#endif
{
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
  unsigned long m = millis_timer_millis;
  unsigned char f = millis_timer_fract;

/* rmv: The code below generates considerably less code (emtpy Sketch is 326 versus 304)...

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
  
  
//MICROSECONDS_PER_MILLIS_OVERFLOW=2048
//MILLIS_INC=2
//FRACT_INC=6
//FRACT_MAX=125
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
  
  return ((m << 8) + t) * (MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
}

void delay(unsigned long ms)
{
  uint16_t start = (uint16_t)micros();
  while (ms > 0) {
    if (((uint16_t)micros() - start) >= 1000) {
      ms--;
      start += 1000;
    }
  }
}

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)
  
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);
#if F_CPU >= 24000000L
	// for the 24 MHz clock for the aventurous ones, trying to overclock

	// zero delay fix
  if (!us) return; //  = 3 cycles, (4 when true)

	// the following loop takes a 1/6 of a microsecond (4 cycles)
	// per iteration, so execute it six times for each microsecond of
	// delay requested.
	us *= 6; // x6 us, = 7 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 22 (24) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
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

	// account for the time taken in the preceeding commands.
	// we just burned 26 (28) cycles above, remove 7, (7*4=28)
  // us is at least 10 so we can substract 7
	us -= 7; // 2 cycles

#elif F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call takes 14 (16) cycles, which is 1us
	if (us <= 1) return; //  = 3 cycles, (4 when true)

	// the following loop takes 1/4 of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2; // x4 us, = 4 cycles

	// account for the time taken in the preceeding commands.
	// we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // us is at least 8 so we can substract 5
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

	// account for the time taken in the preceeding commands.
	// we just burned 20 (22) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
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

	// account for the time taken in the preceeding commands.
	// we just burned 17 (19) cycles above, remove 4, (4*4=16)
  // us is at least 6 so we can substract 4
	us -= 4; // = 2 cycles

#else
	// for the 1 MHz internal clock (default settings for common AVR microcontrollers)

	// the overhead of the function calls is 14 (16) cycles
	if (us <= 16) return; //= 3 cycles, (4 when true)
	if (us <= 25) return; //= 3 cycles, (4 when true), (must be at least 25 if we want to substract 22)

	// compensate for the time taken by the preceeding and next commands (about 22 cycles)
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

#if INITIALIZE_SECONDARY_TIMERS
static void initToneTimerInternal(void)
{
  // Timer is processor clock divided by ToneTimer_Prescale_Index
  #if (TIMER_TO_USE_FOR_TONE == 0)
  TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00)); //stop the clock to configure
  // Use the Tone Timer for phase correct PWM
  sbi(TCCR0A, WGM00);
  cbi(TCCR0A, WGM01);
  cbi(TCCR0B, WGM02);
  TCCR0B |= (ToneTimer_Prescale_Index << CS00);
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
  TCCR1 &= ~((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10)); //stop the clock to configure
  // Use the Tone Timer for fast PWM as phase correct not supported by this timer
  sbi(TCCR1, CTC1);
  #if !defined(__AVR_ATtiny85__)
  sbi(TCCR1, PWM1A); //for the tiny 85, Timer0 is used instead.
  #endif
  sbi(GTCCR, PWM1B);
  OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
  TCCR1 |= (ToneTimer_Prescale_Index << CS10);
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E)
  TCCR1B &= ~((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10)); //stop the clock to configure
  // Use the Tone Timer for phase correct PWM
  sbi(TCCR1A, PWM1A);
  sbi(TCCR1A, PWM1B);
  sbi(TCCR1C, PWM1D);
  cbi(TCCR1D, WGM11);
  sbi(TCCR1D, WGM10);
  TCCR1B |= (ToneTimer_Prescale_Index << CS10);
  #elif (TIMER_TO_USE_FOR_TONE == 1)
  TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10)); //stop the clock to configure
  // Use the Tone Timer for phase correct PWM
  sbi(TCCR1A, WGM10);
  cbi(TCCR1A, WGM11);
  cbi(TCCR1B, WGM12);
  cbi(TCCR1B, WGM13);
  TCCR1B |= (ToneTimer_Prescale_Index << CS10); //set the clock
  #endif
}
 #endif

void initToneTimer(void)
{
  // Ensure the timer is in the same state as power-up
  #if (TIMER_TO_USE_FOR_TONE == 0)
  TCCR0B = (0<<FOC0A) | (0<<FOC0B) | (0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
  TCCR0A = (0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
  // Reset the count to zero
  TCNT0 = 0;
  // Set the output compare registers to zero
  OCR0A = 0;
  OCR0B = 0;
  #if defined(TIMSK)
  // Disable all Timer0 interrupts
  TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
  #elif defined(TIMSK1)
  // Disable all Timer0 interrupts
  TIMSK0 &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR0 |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
  #endif
  
  
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1)
  // Turn off Clear on Compare Match, turn off PWM A, disconnect the timer from the output pin, stop the clock
  TCCR1 = (0<<CTC1) | (0<<PWM1A) | (0<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (0<<CS10);
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
  TIFR |= ((1<<OCF1A) | (1<<OCF1B) | (1<<TOV1));
  
  
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
  TIFR |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<OCF1D));
  
  
  #elif (TIMER_TO_USE_FOR_TONE == 1)
  // Turn off Input Capture Noise Canceler, Input Capture Edge Select on Falling, stop the clock
  TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
  // Disconnect the timer from the output pins, Set Waveform Generation Mode to Normal
  TCCR1A = (0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
  // Reset the count to zero
  TCNT1 = 0;
  // Set the output compare registers to zero
  OCR1A = 0;
  OCR1B = 0;
  // Disable all Timer1 interrupts
  #if defined(TIMSK)
  TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
  // Clear the Timer1 interrupt flags
  TIFR |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
  #elif defined(TIMSK1)
  // Disable all Timer1 interrupts
  TIMSK1 &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
  // Clear the Timer1 interrupt flags
  TIFR1 |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
  #endif
  
  #endif

  #if INITIALIZE_SECONDARY_TIMERS
  // Prepare the timer for PWM
    initToneTimerInternal();
  #endif
}
#if F_CPU == 20000000
  // 20 MHz / 128 ~= 125 KHz
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU == 18432000
  // 18.432 MHz / 128 ~= 125 KHz
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU == 16000000
  // 16 MHz / 128 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B111
#elif F_CPU == 12000000
  // 12 MHz / 64 ~= 125 KHz
  #define ADC_ARDUINO_PRESCALER   B110
#elif F_CPU == 8000000
  // 8 MHz / 64 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B110
#elif F_CPU == 1000000
  // 1 MHz / 8 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   B011
#elif F_CPU == 128000
  // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
  #define ADC_ARDUINO_PRESCALER   B000
#else
  #error Add an entry for the selected processor speed.
#endif

void init(void)
{

  // In case the bootloader left our millis timer in a bad way
  #if defined( HAVE_BOOTLOADER ) && HAVE_BOOTLOADER
  // Ensure the timer is in the same state as power-up
  #if (TIMER_TO_USE_FOR_MILLIS == 0) && defined(WGM01)
  TCCR0B = 0;
  TCCR0A = 0;
  // Reset the count to zero
  TCNT0 = 0;
  // Set the output compare registers to zero
  OCR0A = 0;
  #ifdef OCR0B
  OCR0B = 0;
  #endif
  #if defined(TIMSK)
  // Disable all Timer0 interrupts
  TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
  #elif defined(TIMSK1)
  #ifdef OCIE0B
  // Disable all Timer0 interrupts
  TIMSK0 &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR0 |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
  #else
  // Disable all Timer0 interrupts
  TIMSK0 &= ~((1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR0 |= ((1<<OCF0A) | (1<<TOV0));
  #endif
  #endif
  
  #elif (TIMER_TO_USE_FOR_MILLIS == 0) && defined(TCW0)
  TCCR0A = 0;
  TCCR0B = 0;
  // Reset the count to zero
  TCNT0 = 0;  
  #if defined(TIMSK)
  // Disable all Timer0 interrupts
  TIMSK &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
  #if defined(TICIE0)
  cbi(TIMSK,TICIE0);
  sbi(TIFR0,ICF0);
  #endif
  #elif defined(TIMSK1)
  // Disable all Timer0 interrupts
  TIMSK0 &= ~((1<<OCIE0B) | (1<<OCIE0A) | (1<<TOIE0));
  // Clear the Timer0 interrupt flags
  TIFR0 |= ((1<<OCF0B) | (1<<OCF0A) | (1<<TOV0));
  #if defined(TICIE0)
  cbi(TIMSK0,TICIE0);
  sbi(TIFR0,ICF0);
  #endif
  #endif
  
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1)
  // Turn off Clear on Compare Match, turn off PWM A, disconnect the timer from the output pin, stop the clock
  TCCR1 = (0<<CTC1) | (0<<PWM1A) | (0<<COM1A1) | (0<<COM1A0) | (0<<CS13) | (0<<CS12) | (0<<CS11) | (0<<CS10);
  // Turn off PWM A, disconnect the timer from the output pin, no Force Output Compare Match, no Prescaler Reset
  GTCCR &= ~((1<<PWM1B) | (1<<COM1B1) | (1<<COM1B0) | (1<<FOC1B) | (1<<FOC1A) | (1<<PSR1));
  // Reset the count to zero
  TCNT1 = 0;
  // Set the output compare registers to zero
  OCR1A = 0;
  OCR1B = 0;
  OCR1C = 0;
  // Disable all Timer1 interrupts
  TIMSK = 0;
  // Clear the Timer1 interrupt flags
  TIFR |= ((1<<OCF1A) | (1<<OCF1B) | (1<<TOV1));
  
  
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1E)
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
  // Disable all Timer1 interrupts
  TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<OCIE1D));
  // Clear the Timer1 interrupt flags
  TIFR |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<OCF1D));
  
  
  #elif (TIMER_TO_USE_FOR_MILLIS == 1)
  // Turn off Input Capture Noise Canceler, Input Capture Edge Select on Falling, stop the clock
  TCCR1B = (0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
  // Disconnect the timer from the output pins, Set Waveform Generation Mode to Normal
  TCCR1A = (0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
  // Reset the count to zero
  TCNT1 = 0;
  // Set the output compare registers to zero
  OCR1A = 0;
  OCR1B = 0;
  // Disable all Timer1 interrupts
  #if defined(TIMSK)
  TIMSK &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
  // Clear the Timer1 interrupt flags
  TIFR |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
  #elif defined(TIMSK1)
  // Disable all Timer1 interrupts
  TIMSK1 &= ~((1<<TOIE1) | (1<<OCIE1A) | (1<<OCIE1B) | (1<<ICIE1));
  // Clear the Timer1 interrupt flags
  TIFR1 |= ((1<<TOV1) | (1<<OCF1A) | (1<<OCF1B) | (1<<ICF1));
  #endif
  
  #endif
  #endif

  // Use the Millis Timer for fast PWM (unless it doesn't have an output).
  #if (TIMER_TO_USE_FOR_MILLIS == 0) && defined(WGM01)
  sbi(TCCR0A, WGM01);
  sbi(TCCR0A, WGM00);
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1)
  sbi(TCCR1, CTC1);
  //#if !defined(__AVR_ATtiny85__)
 // sbi(TCCR1, PWM1A); //for the tiny 85, Timer0 is used instead.
  //#endif
  //sbi(GTCCR, PWM1B);
  OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1E)
  sbi(TCCR1C, PWM1D);
  sbi(TCCR1A, PWM1A);
  sbi(TCCR1A, PWM1B);
  cbi(TCCR1E, WGM10); //fast pwm mode
  cbi(TCCR1E, WGM11);
  OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
  #elif (TIMER_TO_USE_FOR_MILLIS == 1)
  sbi(TCCR1A, WGM10);
  sbi(TCCR1B, WGM12);
  #endif
  
  // Millis timer is always processor clock divided by MillisTimer_Prescale_Value (64)
  #if (TIMER_TO_USE_FOR_MILLIS == 0)
  TCCR0B = (TCCR0B & ~((1<<CS02)|(1<<CS01)|(1<<CS00))) | (MillisTimer_Prescale_Index << CS00);
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1)
  TCCR1 = (TCCR1 & ~((1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10))) | (MillisTimer_Prescale_Index << CS10);
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1E)
  TCCR1B = (TCCR1B & ~((1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10))) | (MillisTimer_Prescale_Index << CS10);
  #elif (TIMER_TO_USE_FOR_MILLIS == 1)
  TCCR1B = (TCCR1B & ~((1<<CS12)|(1<<CS11)|(1<<CS10))) | (MillisTimer_Prescale_Index << CS10);
  #endif
  // this needs to be called before setup() or some functions won't work there
  sei();
  
  // Enable the overlow interrupt (this is the basic system tic-toc for millis)
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
  
  // Initialize the timer used for Tone
  #if INITIALIZE_SECONDARY_TIMERS
    initToneTimerInternal();
  #endif

  // Initialize the ADC
  #if defined( INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER ) && INITIALIZE_ANALOG_TO_DIGITAL_CONVERTER
  #if defined(ADCSRA)
    // set a2d prescale factor
	ADCSRA = (ADCSRA & ~((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))) | (ADC_ARDUINO_PRESCALER << ADPS0) | (1<<ADEN);
    // enable a2d conversions
    sbi(ADCSRA, ADEN);
  #endif
  #endif
}

