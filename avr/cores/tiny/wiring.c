/* wiring.c - Timing and initialization
 * Part of ATTinyCore - github.com/SpenceKonde/ATTinyCore
 *   (c) 2005-2006 David A. Mellis as part of Arduino
 *   Modified  28-08-2009 for attiny84 R.Wiersma
 *   Modified  14-10-2009 for attiny45 Saposoft
 *   Modified 17-05-2010 - B.Cook Rewritten to use the various Veneers.
 *   Modified extensively 2016-2021 Spence Konde for ATTinyCore
 *   Free Software - LGPL 2.1, please see LICENCE.md for details
 *---------------------------------------------------------------------------*/


#include "wiring_private.h"
#ifndef __AVR_ATtiny26__
#include <avr/boot.h>
#endif
#if USING_BOOTLOADER
  #include <avr/pgmspace.h>
#else
  #include <avr/eeprom.h>
#endif

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
    #if F_CPU < 8000000L // 4 and 6 MHz get PWM within the target range of 500-1kHz now on 2 pins of t85, and all PWM pins of the x61/
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
#if 0 // generally valid scaling formula follows below in the #else branch
#if (F_CPU==12800000)
//#define MICROSECONDS_PER_MILLIS_OVERFLOW (clockCyclesToMicroseconds(MillisTimer_Prescale_Value * 256))
//#define MICROSECONDS_PER_MILLIS_OVERFLOW ((64 * 256)/12.8) = 256*(64/12.8) = 256*5 = 1280
#define MICROSECONDS_PER_MILLIS_OVERFLOW (1280)
#elif (F_CPU==16500000)
#define MICROSECONDS_PER_MILLIS_OVERFLOW (992)
#else
#define MICROSECONDS_PER_MILLIS_OVERFLOW (clockCyclesToMicroseconds(MillisTimer_Prescale_Value * 256))
#endif
#else
/* The key is never to compute (F_CPU / 1000000L), which may lose precision.
   The formula below is correct for all F_CPU times that evenly divide by 10,
   at least for prescaler values up and including 64 as used in this file. */
#if MillisTimer_Prescale_Value <= 64
#define MICROSECONDS_PER_MILLIS_OVERFLOW \
  (MillisTimer_Prescale_Value * 256UL * 1000UL * 100UL / ((F_CPU + 5UL) / 10UL))
#else
/* It may be sufficient to swap the 100L and 10L in the above formula, but
   please double-check EXACT_NUMERATOR and EXACT_DENOMINATOR below as well
   and make sure it does not roll over. */
#define MICROSECONDS_PER_MILLIS_OVERFLOW 0
#error "Please adjust MICROSECONDS_PER_MILLIS_OVERFLOW formula"
#endif
#endif

/* Correct millis to zero long term drift
   --------------------------------------

   When MICROSECONDS_PER_MILLIS_OVERFLOW >> 3 is exact, we do nothing.
   In this case, millis() has zero long-term drift, that is,
   it precisely follows the oscillator used for timing.

   When it has a fractional part that leads to an error when ignored,
   we apply a correction.  This correction yields a drift of 30 ppm or less:
   1e6 / (512 * (minimum_MICROSECONDS_PER_MILLIS_OVERFLOW >> 3)) <= 30.

   The mathematics of the correction are coded in the preprocessor and
   produce compile-time constants that do not affect size or run time.
 */

/* We cancel a factor of 10 in the ratio MICROSECONDS_PER_MILLIS_OVERFLOW
   and divide the numerator by 8.  The calculation fits into a long int
   and produces the same right shift by 3 as the original code.
 */
#define EXACT_NUMERATOR (MillisTimer_Prescale_Value * 256UL * 12500UL)
#define EXACT_DENOMINATOR ((F_CPU + 5UL) / 10UL)

/* The remainder is an integer in the range [0, EXACT_DENOMINATOR). */
#define EXACT_REMAINDER \
  (EXACT_NUMERATOR - (EXACT_NUMERATOR / EXACT_DENOMINATOR) * EXACT_DENOMINATOR)

/* If the remainder is zero, MICROSECONDS_PER_MILLIS_OVERFLOW is exact.

   Otherwise we compute the fractional part and approximate it by the closest
   rational number n / 256.  Effectively, we increase millis accuracy by 512x.

   We compute n by scaling down the remainder to the range [0, 256].
   The two extreme cases 0 and 256 require only trivial correction.
   All others are handled by an unsigned char counter in millis().
 */
#define CORRECT_FRACT_PLUSONE // possibly needed for high/cheap corner case
#if EXACT_REMAINDER > 0
#define CORRECT_EXACT_MILLIS // enable zero drift correction in millis()
#define CORRECT_EXACT_MICROS // enable zero drift correction in micros()
#define CORRECT_EXACT_MANY \
  ((2U * 256U * EXACT_REMAINDER + EXACT_DENOMINATOR) / (2U * EXACT_DENOMINATOR))
#if CORRECT_EXACT_MANY < 0 || CORRECT_EXACT_MANY > 256
#error "Miscalculation in millis() exactness correction"
#endif
#if CORRECT_EXACT_MANY == 0 // low/cheap corner case
#undef CORRECT_EXACT_MILLIS // go back to nothing for millis only
#elif CORRECT_EXACT_MANY == 256 // high/cheap corner case
#undef CORRECT_EXACT_MILLIS // go back to nothing for millis only
#undef CORRECT_FRACT_PLUSONE // but use this macro...
#define CORRECT_FRACT_PLUSONE + 1 // ...to add 1 more to fract every time
#endif // cheap corner cases
#endif // EXACT_REMAINDER > 0
/* End of preparations for exact millis() with oddball frequencies */

/* More preparations to optimize calculation of exact micros().
   The idea is to reduce microseconds per overflow to unsigned char.
   Then we find the leading one-bits to add, avoiding multiplication.

   This way of calculating micros is currently enabled whenever
   *both* the millis() exactness correction is enabled
   *and* MICROSECONDS_PER_MILLIS_OVERFLOW < 65536.
   Otherwise we fall back to the existing micros().
 */
#ifdef CORRECT_EXACT_MICROS
#if MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 16)
#undef CORRECT_EXACT_MICROS // disable correction for such long intervals
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 15)
#define CORRECT_BITS 8
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 14)
#define CORRECT_BITS 7
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 13)
#define CORRECT_BITS 6
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 12)
#define CORRECT_BITS 5
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 11)
#define CORRECT_BITS 4
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 10)
#define CORRECT_BITS 3
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 9)
#define CORRECT_BITS 2
#elif MICROSECONDS_PER_MILLIS_OVERFLOW >= (1 << 8)
#define CORRECT_BITS 1
#else
#define CORRECT_BITS 0
#endif
#ifdef CORRECT_BITS // microsecs per overflow in the expected range of values
#define CORRECT_BIT7S (0)
#define CORRECT_BIT6
#define CORRECT_BIT5
#define CORRECT_BIT4
#define CORRECT_BIT3
#define CORRECT_BIT2
#define CORRECT_BIT1
#define CORRECT_BIT0
#define CORRECT_UINT ((unsigned int) t)
#define CORRECT_BYTE (MICROSECONDS_PER_MILLIS_OVERFLOW >> CORRECT_BITS)
#if CORRECT_BYTE >= (1 << 8)
#error "Miscalculation in micros() exactness correction"
#endif
#if (CORRECT_BYTE & (1 << 7))
#undef  CORRECT_BIT7S
#define CORRECT_BIT7S (CORRECT_UINT << 1)
#endif
#if (CORRECT_BYTE & (1 << 6))
#undef  CORRECT_BIT6
#define CORRECT_BIT6 + CORRECT_UINT
#endif
#if (CORRECT_BYTE & (1 << 5))
#undef  CORRECT_BIT5
#define CORRECT_BIT5 + CORRECT_UINT
#endif
#if (CORRECT_BYTE & (1 << 4))
#undef  CORRECT_BIT4
#define CORRECT_BIT4 + CORRECT_UINT
#endif
#if (CORRECT_BYTE & (1 << 3))
#undef  CORRECT_BIT3
#define CORRECT_BIT3 + CORRECT_UINT
#endif
#if (CORRECT_BYTE & (1 << 2))
#undef  CORRECT_BIT2
#define CORRECT_BIT2 + CORRECT_UINT
#endif
#if (CORRECT_BYTE & (1 << 1))
#undef  CORRECT_BIT1
#define CORRECT_BIT1 + CORRECT_UINT
#endif
#if (CORRECT_BYTE & (1 << 0))
#undef  CORRECT_BIT0
#define CORRECT_BIT0 + CORRECT_UINT
#endif
#endif // CORRECT_BITS
#endif // CORRECT_EXACT_MICROS

// the whole number of milliseconds per millis timer overflow
#define MILLIS_INC (MICROSECONDS_PER_MILLIS_OVERFLOW / 1000U)

// the fractional number of milliseconds per millis timer overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC (((MICROSECONDS_PER_MILLIS_OVERFLOW % 1000U) >> 3) \
                   CORRECT_FRACT_PLUSONE)
#define FRACT_MAX (1000U >> 3)

#if INITIALIZE_SECONDARY_TIMERS
static void initToneTimerInternal(void);
#endif

#ifndef DISABLEMILLIS
#ifndef CORRECT_EXACT_MICROS
  volatile unsigned long millis_timer_overflow_count = 0;
#endif
  volatile unsigned long millis_timer_millis = 0;
  volatile unsigned char millis_timer_fract = 0;
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
#ifdef CORRECT_EXACT_MILLIS
    static unsigned char correct_exact = 0;     // rollover intended
    if (++correct_exact < CORRECT_EXACT_MANY) {
      ++f;
    }
#endif
    f += FRACT_INC;

    if (f >= FRACT_MAX)
    {
      f -= FRACT_MAX;
      m += MILLIS_INC + 1;
    }
    else
    {
      m += MILLIS_INC;
    }

    millis_timer_fract = f;
    millis_timer_millis = m;
#ifndef CORRECT_EXACT_MICROS
    millis_timer_overflow_count++;
#endif
  }

  uint32_t millis() {
    uint32_t m;
    uint8_t oldSREG = SREG;

    // disable interrupts while we read millis_timer_millis or we might get an
    // inconsistent value (e.g. in the middle of a write to millis_timer_millis)
    cli();
    m = millis_timer_millis;
    SREG = oldSREG;

    return m;
  }

  uint32_t micros() {
#ifdef CORRECT_EXACT_MICROS

    #if (F_CPU == 24000000L || F_CPU == 12000000L || F_CPU == 6000000L || F_CPU == 20000000L || F_CPU == 10000000L || F_CPU == 18000000L)
    uint16_t r; // needed for some frequencies, optimized away otherwise
    // No, you may not lean on the optimizer to do what your #ifdefs should do, it produces an unused variable warning.
    #endif
    uint8_t f;     // temporary storage for millis fraction counter
    uint8_t q = 0; // record whether an overflow is flagged
#endif
    unsigned long m;
    uint8_t t, oldSREG = SREG;

    cli();
#ifdef CORRECT_EXACT_MICROS
    m = millis_timer_millis;
    f = millis_timer_fract;
#else
    m = millis_timer_overflow_count;
#endif
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
    #ifndef CORRECT_EXACT_MICROS
      m++;
    #else
      q = 1;
    #endif
  #elif defined(TIFR) && (TIMER_TO_USE_FOR_MILLIS == 0)
    if ((TIFR & _BV(TOV0)) && (t < 255))
    #ifndef CORRECT_EXACT_MICROS
      m++;
    #else
      q = 1;
    #endif
  #elif defined(TIFR1) && (TIMER_TO_USE_FOR_MILLIS == 1)
    if ((TIFR1 & _BV(TOV1)) && (t < 255))
    #ifndef CORRECT_EXACT_MICROS
      m++;
    #else
      q = 1;
    #endif
  #elif defined(TIFR) && (TIMER_TO_USE_FOR_MILLIS == 1)
    if ((TIFR & _BV(TOV1)) && (t < 255))
    #ifndef CORRECT_EXACT_MICROS
      m++;
    #else
      q = 1;
    #endif
  #endif

    SREG = oldSREG;

  #ifdef CORRECT_EXACT_MICROS
    /* We convert milliseconds, fractional part and timer value
       into a microsecond value.  Relies on CORRECT_EXACT_MILLIS.
       Basically we multiply by 1000 and add the scaled timer.

       The leading part by m and f is long-term accurate.
       For the timer we just need to be close from below.
       Must never be too high, or micros jumps backwards. */
    m = (((m << 7) - (m << 1) - m + f) << 3) + ((
    #if   F_CPU == 24000000L || F_CPU == 12000000L || F_CPU == 6000000L // 1360, 680
        (r = ((unsigned int) t << 7) + ((unsigned int) t << 5), r + (r >> 4))
    #elif F_CPU == 22118400L || F_CPU == 11059200L // 1472, 736
        ((unsigned int) t << 8) - ((unsigned int) t << 6) - ((unsigned int) t << 3)
    #elif F_CPU == 20000000L || F_CPU == 10000000L // 816, 408
        (r = ((unsigned int) t << 8) - ((unsigned int) t << 6), r + (r >> 4))
    #elif F_CPU == 18432000L || F_CPU == 9216000L // 888, 444, etc.
        ((unsigned int) t << 8) - ((unsigned int) t << 5) - ((unsigned int) t << 1)
    #elif F_CPU == 18000000L // hand-tuned correction: 910
        (r = ((unsigned int) t << 8) - ((unsigned int) t << 5), r + (r >> 6))
    #elif F_CPU == 16500000L // hand-tuned correction: 992
        ((unsigned int) t << 8) - ((unsigned int) t << 3)
    #elif F_CPU == 14745600L || F_CPU == 7372800L || F_CPU == 3686400L // 1104, 552
        ((unsigned int) t << 7) + ((unsigned int) t << 3) + ((unsigned int) t << 1)
    #else // general catch-all
        (((((((((((((CORRECT_BIT7S
                     CORRECT_BIT6) << 1)
                     CORRECT_BIT5) << 1)
                     CORRECT_BIT4) << 1)
                     CORRECT_BIT3) << 1)
                     CORRECT_BIT2) << 1)
                     CORRECT_BIT1) << 1)
                     CORRECT_BIT0)
    #endif
      ) >> (8 - CORRECT_BITS));
    return q ? m + MICROSECONDS_PER_MILLIS_OVERFLOW : m;
  #else
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
  #endif // !CORRECT_EXACT_MICROS
  }

  static void __empty() {
    // Empty
  }
  void yield(void) __attribute__ ((weak, alias("__empty")));

  void delay(uint32_t ms) {
    #if (F_CPU >= 1000000L)
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

  void delay(uint32_t ms) { // non-millis-timer-dependent delay()

    while(ms--){
      yield();
      delayMicroseconds(1000);
    }
  }
#endif

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */

void delayMicroseconds(uint16_t us)
{
  #define _MORENOP_ "" // redefine to include NOPs depending on frequency

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
    // of the function call takes 18 (20) cycles, which is approx. 1us
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

    // user wants to wait 7us or more -- here we can use approximation
    if (us > 34) { // 3 cycles
      // Since the loop is not accurately 1/5 of a microsecond we need
      // to multiply us by (18.432 / 20), very close to 60398 / 2.**16.

      // Approximate (60398UL * us) >> 16 by using 60384 instead.
      // This leaves a relative error of 232ppm, or 1 in 4321.
      unsigned int r = us - (us >> 5);  // 30 cycles
      us = r + (r >> 6) - (us >> 4);    // 55 cycles
      // careful: us is generally less than before, so don't underrun below

      // account for the time taken in the preceding and following commands.
      // we are burning 114 (116) cycles, remove 29 iterations: 29*4=116.

         TODO: is this calculation correct.  Right now, we do
                function call           6 (+ 2) cycles
                wait at top             4
                comparison false        3
                multiply by 5           7
                comparison false        3
                compute r               30
                update us               55
                subtraction             2
                return                  4
                total                   --> 114 (116) cycles


      // us dropped to no less than 32, so we can subtract 29
      us -= 29; // 2 cycles
    } else {
      // account for the time taken in the preceding commands.
      // we just burned 30 (32) cycles above, remove 8, (8*4=32)
      // us is at least 10, so we can subtract 8
      us -= 8; // 2 cycles
    }

  #elif F_CPU >= 18000000L
    // for the 18 MHz clock, if somebody is working with USB
    // or otherwise relating to 12 or 24 MHz clocks

    // for a 1 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is .8 us
    if (us <= 1) return; // = 3 cycles, (4 when true)

    // make the loop below last 6 cycles
  #undef  _MORENOP_
  #define _MORENOP_ " nop \n\t  nop \n\t"

    // the following loop takes 1/3 of a microsecond (6 cycles) per iteration,
    // so execute it three times for each microsecond of delay requested.
    us = (us << 1) + us; // x3 us, = 5 cycles

    // account for the time taken in the preceding commands.
    // we burned 20 (22) cycles above, plus 2 more below, remove 4 (4*6=24),
    // us is at least 6 so we may subtract 4
    us -= 4; // = 2 cycles

  #elif F_CPU >= 16500000L
    // for the special 16.5 MHz clock

    // for a one-microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is about 1us
    if (us <= 1) return; //  = 3 cycles, (4 when true)

    // the following loop takes 1/4 of a microsecond (4 cycles) times 32./33.
    // per iteration, thus rescale us by 4. * 33. / 32. = 4.125 to compensate
    us = (us << 2) + (us >> 3); // x4.125 with 23 cycles

    // account for the time taken in the preceding commands.
    // we burned 38 (40) cycles above, plus 2 below, remove 10 (4*10=40)
    // us is at least 8, so we subtract only 7 to keep it positive
    // the error is below one microsecond and not worth extra code
    us -= 7; // = 2 cycles

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

  #elif F_CPU >= 14745600L
    // for a one-microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is approx. 1us

    if (us <= 1) return; //  = 3 cycles, (4 when true)

    // the following loop takes nearly 1/4 (0.271%) of a microsecond (4 cycles)
    // per iteration, so execute it four times for each microsecond of
    // delay requested.
    us <<= 2; // x4 us, = 4 cycles

    // user wants to wait 8us or more -- here we can use approximation
    if (us > 31) { // 3 cycles
      // Since the loop is not accurately 1/4 of a microsecond we need
      // to multiply us by (14.7456 / 16), very close to 60398 / 2.**16.

      // Approximate (60398UL * us) >> 16 by using 60384 instead.
      // This leaves a relative error of 232ppm, or 1 in 4321.
      unsigned int r = us - (us >> 5);  // 30 cycles
      us = r + (r >> 6) - (us >> 4);    // 55 cycles
      // careful: us is generally less than before, so don't underrun below

      // account for the time taken in the preceding and following commands.
      // we are burning 107 (109) cycles, remove 27 iterations: 27*4=108.

      // us dropped to no less than 29, so we can subtract 27
      us -= 27; // 2 cycles
    } else {
      // account for the time taken in the preceding commands.
      // we just burned 23 (25) cycles above, remove 6, (6*4=24)
      // us is at least 8, so we can subtract 6
      us -= 6; // 2 cycles
    }

  #elif F_CPU >= 12000000L
    // for the 12 MHz clock if somebody is working with USB

    // for a 1 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 1.3us
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

    // for a 1 to 3 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 2.5us
    if (us <= 3) return; //  = 3 cycles, (4 when true)

    // make the loop below last 6 cycles
  #undef  _MORENOP_
  #define _MORENOP_ " nop \n\t  nop \n\t"

    // the following loop takes 1 microsecond (6 cycles) per iteration
    // we burned 15 (17) cycles above, plus 2 below, remove 3 (3 * 6 = 18)
    // us is at least 4 so we can subtract 3
    us -= 3; // = 2 cycles

  #elif F_CPU >= 4000000L
    // for that unusual 4mhz clock...

    // for a 1 to 4 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 4us
    if (us <= 4) return; //  = 3 cycles, (4 when true)

    // the following loop takes 1 microsecond (4 cycles)
    // per iteration, so nothing to do here! \o/
    // ... in terms of rescaling.  We burned 15 (17) above plus 2 below,
    // so remove 5 (5 * 4 = 20), but we may at most remove 4 to keep us > 0.
    us -= 4; // = 2 cycles

  #elif F_CPU >= 2000000L
    // for that unusual 2mhz clock...

    // for a 1 to 9 microsecond delay, simply return.  the overhead
    // of the function call takes 14 (16) cycles, which is 8us
    if (us <= 9) return; //  = 3 cycles, (4 when true)
    // must be at least 10 if we want to do /= 2 -= 4

    // divide by 2 to account for 2us runtime per loop iteration
    us >>= 1; // = 2 cycles;

    // the following loop takes 2 microseconds (4 cycles) per iteration
    // we burned 17 (19) above plus 2 below,
    // so remove 5 (5 * 4 = 20), but we may at most remove 4 to keep us > 0.
    us -= 4; // = 2 cycles

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
        _MORENOP_         // more cycles according to definition
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
  // return = 4 cycles
}

// This clears up the timer settings, and then calls the tone timer initialization function (unless it's been disabled - but in this case, whatever called this isn't working anyway!
void initToneTimer(void) {
  // Ensure the timer is in the same state as power-up
  #if defined(__AVR_ATtiny43__)
    TIMSK1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = 0;
    OCR1B  = 0;
    TIFR1  = 0x07;
  #elif defined(__AVR_ATtiny26__)
    TIMSK  &= 2;
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = 0;
    OCR1B  = 0;
    TIFR   = 0x66;
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
    OCR1A  = 0;
    OCR1B  = 0;
    OCR1C  = 0;
    OCR1D  = 0;
    // Disable all Timer1 interrupts & clear the Timer1 interrupt flags
    TIMSK &= ~((1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B) | (1 << OCIE1D));
    TIFR   =  ((1 <<  TOV1) | (1 <<  OCF1A) | (1 <<  OCF1B) | (1 <<  OCF1D));

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
  static void initToneTimerInternal(void) {
    /* This never worked right
    #if (defined(PLLTIMER1) || defined(LOWPLLTIMER1)) && !defined(PLLCSR)
      #error "Chip does not have PLL (only x5, x61 series do), which is selected (somehow) as timer1 clock source. If you have not modified the core, please report this to core maintainer."
    #elif defined(LOWPLLTIMER1) && CLOCK_SOURCE == 6
      #error "Low speed PLL as Timer1 clock source is NOT SUPPORTED when PLL is used as system clock source"
    #elif defined(PLLTIMER1) // option on x5 and x61
      if (!PLLCSR) {
        PLLCSR = (1 << PLLE);
        while (!(PLLCSR & 1)); // wait for lock
        PLLCSR = (1 << PCKE) | (1 << PLLE);
      }
    #elif defined(LOWPLLTIMER1) // option on x5 and x61
      if (!PLLCSR) {
        PLLCSR = (1 << LSM) | ( 1 <<PLLE);
        while (!(PLLCSR & 1)); // wait for lock
        PLLCSR = (1 << PCKE) | (1 << LSM) | (1 << PLLE);
      }
    #endif
    */

    #if defined(__AVR_ATtinyX41__)
      TCCR1A   = (1 << WGM10) | (1 << COM1A1)| (1 << COM1B1); // enable OC1A, OC1B
      TCCR1B   = (ToneTimer_Prescale_Index << CS10); // set the clock
      TCCR2A   = (1 << WGM20) | (1 << COM2A1)| (1 << COM2B1); // enable OC2A, OC2B
      TCCR2B   = (ToneTimer_Prescale_Index << CS10); // set the clock
      TOCPMSA0 = 0b00010000; // PA4: OC0A, PA3: OC1B, PA2: N/A,  PA1: N/A
      TOCPMSA1 = 0b10100100; // PB2: OC2A, PA7: OC2B, PA6: OC1A, PA5: OC0B
      // TOCPMCOE = 0; // keep these disabled!
    #elif defined(__AVR_ATtiny828__)
      TCCR1A   = (1 << WGM10) | (1 << COM1A1)| (1 << COM1B1); // enable OC1A, OC1B
      TCCR1B   = (ToneTimer_Prescale_Index << CS10); // set the clock
      TOCPMSA0 = (0b11100100);  // PC3: OC1B, PC2: OC1A, PC1: OC0B, PC0 OC0A.
      TOCPMSA1 = (0b11001001);  // PC7: OC1B, PC6: OC0A, PC5: OC1A, PC4,OC0B
      // TOCPMCOE = 0; // keep these disabled!
    #elif (TIMER_TO_USE_FOR_TONE == 0)
      #warning "ATTinyCore only supports using Timer1 for tone - this is untested code!"

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
      /* Like the 841/441/828 we turn on the output compare, and analogWrite() only twiddles the enable bits */
      TCCR1A = (1 << COM1A1)  |(1 << COM1B1) | (1 << WGM10);
      TCCR1B = (ToneTimer_Prescale_Index << CS10);
    #elif (TIMER_TO_USE_FOR_TONE == 1 && defined(__AVR_ATtiny26__))
      OCR1C = 0xFE;
      TCCR1A = (1 << PWM1A) | (1 << PWM1B);
      TCCR1B = ToneTimer_Prescale_Index;
    #elif (TIMER_TO_USE_FOR_TONE == 1) // x4, x8, x313,
      // Use the Tone Timer for phase correct PWM
      TCCR1A = (1<<WGM10);
      TCCR1B = (0<<WGM12) | (0<<WGM13) | (ToneTimer_Prescale_Index << CS10); //set the clock
    #endif
  }
#endif



/*#if (defined(__AVR_ATtinyX41__) && F_CPU == 16000000 && CLOCK_SOURCE == 0 )
  // functions related to the 16 MHz internal option on ATtiny841/441.
  // 174 CALBOOST seems to work very well - it gets almost all of them close enough for USART, which is what matters. It was empirically determined from a few parts I had lying around.
  #define TINYX41_CALBOOST 174
  static uint8_t tinyx41_cal16m = 0;
  static uint8_t saveTCNT = 0;
  void oscBoost() {
    OSCCAL0 = (origOSC>MAXINITCAL?255:(origOSC + CALBOOST));
    _NOP();
  }
  void oscSafeNVM() {      // called immediately prior to writing to EEPROM.
    //TIMSK0& = ~(_BV(TOIE0)); // turn off millis interrupt - let PWM keep running (Though at half frequency, of course!)
    //saveTCNT = TCNT0;
    //if (TIFR0&_BV(TOV0)) { // might have just missed interrupt - recording as 255 is good enough (this may or may not have been what we recorded, but if it was still set, interrupt didn't fire)
    //  saveTCNT = 255;
    //}
    #ifndef DISABLEMILLIS
      saveMillis = millis(); //save low bytes of millis
    #endif
    set_OSCCAL(read_factory_calibration());
  }
*/
  //void oscDoneNVM(uint8_t bytes_written) {
    /* That's the number of bytes of eeprom written, at 3.3ms or so each.
     * EEPROM does it one at a time, but user code could call these two methods when doing block writes (up to 64 bytes). Just be sure to do the eeprom_busy_wait(); at the end, as in EEPROM.h.
     * Not so much because it's a prerequisite for this stupid correction to timing but because cranking the oscillator back up during the write kinda defeats the point of slowing it doewn...
     * 3.3ms is good approximation of the duration of writing a byte - it'll be about 3~4% faaster since we're running around 5V at default call - hence, we're picking 3.3ms - the oscillator
     * adjustment loops and these calculations should be fast enough that the time they dont take long enough to worry about...
     * relies on assumptions from implementation above of millis on this part at 16MHz!
     * millis interrupt was disabled when oscSaveNVM() was called - so we don't need to do anything fancy to access the volatile variables related to it.
     * 1 millis interrupt fires every 1.024ms, so we want 3.3/1.024= 3.223 overflows; there are 256 timer ticksin an overflow, so 57 timer ticks...
     */
    // FRACT_MAX = 125, FRACT_INC =3
    //set_OSCCAL(tinyx41_cal16m); //stored when we initially tuned
    //#ifndef DISABLEMILLIS
    /*
    uint8_t m = 3 * bytes_written; // the 3 whole overflows
    uint16_t tickcount = 57*bytes_written + saveTCNT;
    m += (tickcount >> 8); // overflows from theose extra /0.223's
    millis_timer_overflow_count += m; // done with actual overflows, so record that.
    uint16_t f = FRACT_INC*m + millis_timer_fract; // (m could be up to 207)
    while(f > FRACT_MAX){ // at most 621 + 124 = 745
      f -= FRACT_MAX;
      m++;
    }
    // now we're adding up the contributions to millis from the 0.024 part...
    // save the results
    millis_timer_fract = f;
    millis_timer_millis += m;
    TCNT0   = 0;
    TIFR0  |= _BV(TOV0);   // clear overflow flag
    TIMSK0 |= _BV(TOIE0); // enable overflow interrupt
    TCNT0 = tickcount;    // restore new tick count
    // wonder if it was worth all that just to write to the EEPROM while running at 16MHz off internal oscillator without screwing up millis and micros...
    */
    // I don't think it was, gonna go with a quicker dirtier method - we instead leave it running at half speed, saving the low byte of millis. Longest time at half-speed
    // is 3.3 * 64 around 200 ms for a max length block write. So if we leave millis running, and know that it's running at half speed... just take difference of the LSB
    // and add that much to millis.
    //(uint8_t)millis_timer_overflow_count
    //uint8_t milliserror=((uint8_t) millis())-saveMillis
    //#endif
  //}
//#endif



/* This attempts to grab a tuning constant from flash (if it's USING_BOOTLOADER) or EEPROM (if not). Note that it is not called unless ENABLE_TUNING is set.
 * inlined for flash savings (call overhead) not speed; it is only ever called once, on startup, so I think it would get inlined anyway most of the time
 * addresses for key values:
 * FLASHEND is second byte of bootloader version, FLASHEND-1 is first byte. (all_
 * OFFSET:   Normal:  PLLs:   x41:          1634/828:
 * LASTCAL-0   12.8    16.5   16.0 @ 5V0    12.8 @ 5V0
 * LASTCAL-1   12.0    16.0   12.8 @ 5V0    12.0 @ 5V0
 * LASTCAL-2    8.0   CUSTOM  12.0 @ 5V0     8.0 @ 5V0
 * LASTCAL-3  CUSTOM   0x00   CUST/0x00     CUSTOM
 * LASTCAL-4
 * LASTCAL-5                  0x00
 *
 * Note that a "tuned" value os 0x00 or 0xFF is never treated as acceptable except for 0xFF in the case of 16 MHz tuning for x41.
 *
 * ENABLE_TUNING values:
 * 1 = use for required changes (it started app boot-tuned wrong, so we need to fix it, or we wanted tuned frequency like 12 or 16.5 from non-bootloaded.
 * 2 = use stored cal contents for 8 MHz even if we start up like that.
 * 4 = enable custom tuning (CUST slot above - we make no promises about any timekeeping functions)
 * 8 = paranoid - don't trust that boot tuning actually happened
 */
#if USING_BOOTLOADER
  #define read_tuning_byte(x) pgm_read_byte_near(x)
  #define LASTCAL FLASHEND
#else // without a bootloader, we have to store calibration in the EEPROM
  #define read_tuning_byte(x) eeprom_read_byte((uint8_t*) x)
  #define LASTCAL E2END
#endif

#if defined(__SPM_REG)
  uint8_t read_factory_calibration(void) {
    uint8_t value = boot_signature_byte_get(1);
    return value;
  }
#endif

// only called in one place most likely, but we'll try to make sure the compiler inlines it just the same. It's 3 instructions, so it absolutely ought to be.
inline void __attribute__((always_inline)) set_OSCCAL(uint8_t cal) {
  #ifdef OSCCAL0
    OSCCAL0 = cal;
  #else
    OSCCAL = cal;
  #endif
  __asm__ __volatile__ ("nop" "\n\t"); /* This is the "trick" that micronucleus uses to avoid crashes from sudden frequency change. Since micronucleus works, I'll copy that trick. */
}

static inline bool __attribute__((always_inline)) check_tuning() {
  // It is almost inconceivable that 0 would be the desired tuning
  // If this is still 0 by the end, we didn't have a valid tuning constant.
  uint8_t tuneval = 0;
  #if (CLOCK_SOURCE == 6)
  /* start PLL timer tuning */
    #if (F_CPU == 16500000)
      tuneval = read_tuning_byte(LASTCAL - 0);
    #elif (F_CPU == 16000000 || F_CPU == 8000000 || F_CPU) // 16 or divided down 16.
      tuneval = read_tuning_byte(LASTCAL - 1);
    #else // Custom tuning
      tuneval = read_tuning_byte(LASTCAL - 2);
    #endif
  /* end PLL timer tuning */
  #elif (CLOCK_SOURCE == 0)
  /* start internal tuning */
    #if defined(__AVR_ATtinyX41__)
      // 441/841 can be pushed all the way to 16!!
      #if  (F_CPU == 16000000) // 16 MHz - crazy internal oscillator, no?
        uint8_t tune8 = read_tuning_byte(LASTCAL - 3)
        if (tune8 != 0xFF)
          // Need this specific test because 0xFF is valid tuning here
          // everywhere else, we assume 0xFF means no tuning saved.
          tuneval = read_tuning_byte(LASTCAL - 0);
          // if tuning stored is 0, then tuner determined that chip can't get to 16. or even within 2%.
          // return success, but tune for 8 MHz, because otherwise the routine would think it meant no tuning stored
          // and guess - likely resulting in broken 15.x clock. Tuned to 8, they have more chance to figure out what
          // happened, and user code can check for OSCCAL < 128 - that would mean the chip isn't tuned to 16.
          // docs will advise that if ENABLE_TUNING is defined, F_CPU is 16 MHz and OSCCAL0 < 128, that means that
          // the chip in question can't hit 16, please use a different chip, and that they can halve the serial
          // baud to see what it's printing to serial. On the other hand, if F_CPU is 16 MHz, OSCCAL0 > 128, whether
          // or not ENABLE_TUNING is defined, if it's not running close enough to 16 for serial to work, it guessed at
          // cal because no tuning or not enabled, and the guess didn't work; running tuning sketch should fix it.;
          set_OSCCAL(tuneval != 0 ? tuneval : tune8);
          return 1;
        } else {
          uint8_t osccal=OSCCAL0;
          if (osccal < 82) {
            set_OSCCAL(OSCCAL0+174);
            return 1;
          } else if (osccal < 88) {
            set_OSCCAL(255);
            return 1;
          }
          return 0;
        }
      #elif (F_CPU == 12800000)
        tuneval = read_tuning_byte(LASTCAL - 1);
      #elif (F_CPU == 12000000)
        tuneval = read_tuning_byte(LASTCAL - 2);
      #elif (F_CPU ==  8000000 &&  defined(LOWERCAL)) // if LOWERCAL is defined user says Vcc is closer to 5v0, so use tuned cal.
        tuneval = read_tuning_byte(LASTCAL - 3);
      #elif (F_CPU ==  8000000 && !defined(LOWERCAL)) // if LOWERCAL is not defined user says Vcc is closer to 3v3, so use factory cal.
        // do nothing
      #else
        tuneval = read_tuning_byte(LASTCAL - 4);
      #endif
    #else
      // Everything else uses the same tuning locations
      #if   (F_CPU == 12800000)
        tuneval = read_tuning_byte(LASTCAL - 0);
      #elif (F_CPU == 12000000)
        tuneval = read_tuning_byte(LASTCAL - 1);
      #elif (F_CPU ==  8000000 && ( defined(LOWERCAL) || !(defined(__AVR_ATtiny1634__) || defined(__AVR_ATtiny828__))))
        // On those parts, oscillator frequency depends dramatically on voltage, but factory cal is very good at 3V3
        // If they say it's running at ~5V we define LOWERCAL, and we should use the (lower) tuned cal or guess if we
        // don't have such a constant stored.
        tuneval = read_tuning_byte(LASTCAL - 2);
      #elif (F_CPU ==  8000000 && (!defined(LOWERCAL) &&  (defined(__AVR_ATtiny1634__) || defined(__AVR_ATtiny828__))))
        // do nothing - user says Vcc is closer to 3v3, so factory cal is best.
      #else // Custom tuning
        tuneval = read_tuning_byte(LASTCAL - 3);
      #endif
    #endif
  /* end internal tuning */
  #else
  /* start non-tunable timer tuning */
    badCall("Call to check_tuning() detected, but we are using an external oscillator which we can't tune. This is a defect in ATTinyCore and should be reported to the developers promptly");
    return false;
  /* end non-tunable timer tuning */
  #endif
  #if (!defined(__AVR_ATtinyX41__) || F_CPU != 16000000)
    // Success here is special case handled above as 0xFF is not an unreasonable tuning value
    // otherwise 0 = above derermined it shouldn't be tuned, and 0xFF = no tuning constant present (blank flash/eeprom)
    if (tuneval != 0 && tuneval !=0xFF) {
      set_OSCCAL(tuneval);
      return 1;
    }
  #endif
  return 0;
}




void init_clock() {
  /*  If clocked from the PLL (CLOCK_SOURCE == 6) then there are three special cases all involving
   *  the 16.5 MHz clock option used to support VUSB on PLL-clocked parts.
   *  If F_CPU is set to 16.5, and the Micronucleus bootloader is in use (indicated by BOOTTUNED165
   *  being defined), the bootloader has already set OSCCAL to run at 16.5; if the board was told
   *  that was the context it would be started in,
   *  If it is set to 16.5 but BOOTTUNED165 is not set, we're not using the micronucleus bootloader
   *  and we need to increase OSCCAL; this is a guess, but works better than nothing.
   *
   *  Note that this horrendous mess gets preprocessed down to only a couple of lines. check_tuning()
   *  is only called once, at most, which is why we inline it.
   */

  #if (CLOCK_SOURCE == 6)
    /* Start PLL prescale and tuning */
    #if (defined(BOOTTUNED165))       // If it's a micronucleus board, it will either run at 16.5 after
                                      // adjusting the internal oscillator for that speed (in which case
                                      // we are done) or we want it to be set to run at 16.
      #if (F_CPU == 16000000L || F_CPU == 8000000L || F_CPU == 4000000L || F_CPU == 2000000L || F_CPU == 1000000L || F_CPU == 500000L || F_CPU == 250000L || F_CPU == 125000L || F_CPU == 62500L )
        #if defined(ENABLE_TUNING)  && (ENABLE_TUNING & 3)
                                      // This is "necessary" tuning, if ENABLE_TUNING & 1, we try to tune - unlike cases where we only tune if ENABLE_TUNING & 2 (tune always)
          if (!check_tuning())        // because it was tuned to a different speed than what we want by the bootloader (or we suspect it was)
        #endif                        // failing that, or if tuning isn't enabled in the first place we just get the factory cal.
          #if defined(__SPM_REG)
            set_OSCCAL(read_factory_calibration());
          #endif
        #if (F_CPU     != 16000000L)  // 16MHz is speed of unprescaled PLL clock - if we don't want that, means we want a prescaled one
          #ifdef CCP
            CCP=0xD8; //enable change of protected register
          #else
            CLKPR=1<<CLKPCE; //enable change of protected register
          #endif
          #if   (F_CPU ==  8000000L)
            CLKPR = 1;                // prescale by 2 for 8MHz
          #elif (F_CPU ==  4000000L)
            CLKPR = 2;                // prescale by 4 for 4MHz
          #elif (F_CPU ==  2000000L)
            CLKPR = 3;                // prescale by 8 for 2MHz
          #elif (F_CPU ==  1000000L)
            CLKPR = 4;                // prescale by 16 for 1MHz
          #elif (F_CPU ==   500000L)
            CLKPR = 5;                // prescale by 32 for 0.5MHz
          #elif (F_CPU ==   250000L)  // these extremely slow speeds are of questionable value.
            CLKPR = 6;                // prescale by 64 for 0.25MHz
          #elif (F_CPU ==   125000L)  // but if using micronucleus to get code onto chip, no choice
            CLKPR = 7;                // prescale by 128 for 125kHz
          #else // (F_CPU ==   62500L)  // This is far too slow for anything to work right!
            CLKPR = 8;                // prescale by 256 for 62.50kHz
          #endif
        #endif // Not 16 MHz
      #elif (F_CPU != 16500000L)  // second case - first was not 16.5 AND one of the normal ones - hence it's a custom tuning on a part that's starting
        #if defined(ENABLE_TUNING) && ((ENABLE_TUNING) & 4)   // from VUSB and tuned it's osc for 16.5... You know what a normal, every day occurrence that is right?
          check_tuning();   // if tuning is enabled for custom speed (ENABLE_TUNING & 4) - use tuned value - if we have one.
                            // If we don't, we're up a certain creek without a useful implement, as we don't know 'til runtime, when we can't tell the user!
        #else
          // If we're here, though, we didn't even have a boat because custom tuning is disabled, so we can tell them at compile time.
          #error "Requested PLL-derived frequency is a custom tuning, which is not enabled."
        #endif // end of check for custom tuning enabled.
      #else
        // Chip was tuned by bootloader to 16.5 MHz, and that's what we're telling it to run at - that was easy. But - if we're paranoid, we can get our stored one!
        #if defined(ENABLE_TUNING) && ((ENABLE_TUNING) & 8)
          check_tuning(); // Don't trust that boot tuning happened.
        #endif
      #endif
    #elif (F_CPU == 16500000L)    // not using a bootloader configured to leave OSC at 16.5 MHz - but that's what we want to run at...
      #if defined(ENABLE_TUNING) && ((ENABLE_TUNING) & 3) // if tuning is enabled, grab the tuning constant, if present
      if (!check_tuning())                                // and only grab default and guess if we didn't find one.
      #endif
      { // if that fails, or tuning isn't enabled, we do the guess :-(
        #if defined(__SPM_REG)
          if (OSCCAL == read_factory_calibration()) { // adjust the calibration up from 16.0mhz to 16.5mhz
            if (OSCCAL >= 128) {
              set_OSCCAL(OSCCAL + 7); // maybe 8 is better? oh well - only about 0.3% out anyway
            } else {
              set_OSCCAL(OSCCAL + 5);
            }
          }
        #else
          if (OSCCAL >= 128) {
            set_OSCCAL(OSCCAL + 7); // maybe 8 is better? oh well - only about 0.3% out anyway
          } else {
            set_OSCCAL(OSCCAL + 5);
          }
        #endif
      }
    #else // We're using PLL, and we are neither tuned to nor seeking 16.5...
      #if (F_CPU == 16000000L || F_CPU == 8000000L || F_CPU == 4000000L || F_CPU == 2000000L || F_CPU == 1000000L || F_CPU == 500000L || F_CPU == 250000L || F_CPU == 125000L || F_CPU == 62500L )
        #if defined(ENABLE_TUNING) && ((ENABLE_TUNING) & 2) // if we 'always tune' (ENABLE_TUNING == 2)
          check_tuning();      // Use tuned value if set to always tune, otherwise assume factory tuning is OK.
        #endif
        // 16MHz is speed of unprescaled PLL clock.
        #if     (F_CPU != 16000000)
          #ifdef CCP
            CCP   = 0xD8; // enable change of protected register
          #else
            CLKPR = (1 << CLKPCE); // enable change of protected register
          #endif
          // One really wonders why someone would use the PLL as clock source if they weren't using VUSB or running at 16MHz, it's got to burn more power...
          #if   (F_CPU ==  8000000L)
            CLKPR = 1;                // prescale by 2 for 8MHz
          #elif (F_CPU ==  4000000L)
            CLKPR = 2;                // prescale by 4 for 4MHz
          #elif (F_CPU ==  2000000L)  // could also be done with fuses, but this is such a weird situation,
            CLKPR = 3;                // prescale by 8 for 2MHz
          #elif (F_CPU ==  1000000L)
            CLKPR = 4;                // prescale by 16 for 1MHz
          #elif (F_CPU ==   500000L)
            CLKPR = 5;                // prescale by 32 for 0.5MHz
          #elif (F_CPU ==   250000L)  // these extremely slow speeds are of questionable value.
            CLKPR = 6;                // prescale by 64 for 0.25MHz
          #elif (F_CPU ==   125000L)
            CLKPR = 7;                // prescale by 128 for 125kHz
          #else// (F_CPU ==   62500L)
            CLKPR = 8; // prescale by 256 for 62.50kHz
          #endif  // end of checking F_CPUs
        #endif    // end not 16 MHz
      #else // else not one of 16 MHz derived ones
        #if defined(ENABLE_TUNING) && ((ENABLE_TUNING) & 4)
          check_tuning(); // if tuning is enabled for custom speed (ENABLE_TUNING & 4)
        #else             // Use tuned value - if we have one.
          #error "Requested PLL-derived frequency is a custom tuning, which is not enabled."
        #endif // end of check for custom tuning enabled.
      #endif // end if not 16 MHz derived
    #endif // end check for VUSB-related special cases for the PLL
  /* End of PLL prescale and tuning */
  #elif (CLOCK_SOURCE == 0) // system clock is internal, so we may want to tune it, or it may be boot tuned.
    /* Start internal osc prescale and tuning */
    #if (F_CPU == 8000000L || F_CPU == 4000000L || F_CPU == 2000000L || F_CPU == 1000000L || F_CPU == 500000L || F_CPU == 250000L || F_CPU == 125000L || F_CPU == 62500L || F_CPU == 31250L)
      #if defined(__SPM_REG)
        // on parts without self programming no bootloader can set the speed. This whole section is skipped because on the one part like this, we do not support these weird clocks.
        #if (defined(BOOTTUNED128) || defined(BOOTTUNED120) || defined(BOOTTUNED160))
          // if the bootloader tuned for 12, 12.8, or even 16, but we want a normal speed,  grab the cal byte if enabled and we have it, otherwise grab default cal.
          #if defined(ENABLE_TUNING)  && (ENABLE_TUNING & 3)  // necessary tuning, if ENABLE_TUNING & 3, we try to tune - unlike cases where we only check ENABLE_TUNING & 2 (tune always)
            if (!check_tuning())  // try to use stored calibration
          #endif
          #if defined(LOWERCAL)   // means it is 1634, 828, or 441/841 so we probably have LOWERCAL set (if running at 5V it should be). Use it to guess at correct cal.
            set_OSCCAL(read_factory_calibration() - LOWERCAL);
          #else
            set_OSCCAL(read_factory_calibration());
          #endif
        #else
          // bootloader hasn't tuned it, and it's a normal frequency. Use tuning if set to always tune, otherwise don't do anything.
          // BUT if LOWERCAL is defined, that means it is 1634, 828, or 441/841 and running at 5V, treat as "necessary" tuning
          // and use LOWERCAL to adjust factory cal if no tuning.
          #if defined(ENABLE_TUNING) && (((ENABLE_TUNING) & 2) || (defined(LOWERCAL) && (ENABLE_TUNING & 3)))
            if (!check_tuning())  // try to use stored calibration
          #endif
          #if defined(LOWERCAL)
              set_OSCCAL(read_factory_calibration() - LOWERCAL);
          #else
              set_OSCCAL(read_factory_calibration());
          #endif
        #endif
      #endif
      // apply prescaling to get desired frequency if not set by fuses
      #if (F_CPU != 8000000L && F_CPU != 1000000L)
        // normal oscillator, and we want a setting that fuses won't give us,
        // so need to set prescale.
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
          CLKPR = 8;                // prescale by 256 for 31.25kHz
        #endif
      #endif // End if not 1/8 MHz
    #elif (F_CPU == 12800000 || F_CPU == 12000000 || F_CPU == 16000000)
      // we want a 12.8, 12, or 16 MHz (the non-custom tuning frequencies)
      #if F_CPU == 16000000
        #if !defined(__AVR_ATtinyX41__)
          #error "Only 841/441 can reach 16 MHz with internal oscillator"
        #elif defined(BOOTTUNED160)
          // boot tuned to desired frequency, don't need to do anything unless paranoid tuning.
          #if defined(ENABLE_TUNING) && (ENABLE_TUNING & 8)
            check_tuning(); // Don't trust that boot tuning happened; maybe it may get uploaded with programmer (paranoid tuning)
          #endif
        #else
          #if defined(ENABLE_TUNING)  && (ENABLE_TUNING & 3)  // necessary tuning, if ENABLE_TUNING & 3, we try to tune - unlike cases where we only check ENABLE_TUNING & 2 (tune always)
            if (!check_tuning())      // here it started up at one tuned speed, we want a speed that isn't tuned.
          #endif
          {
            // since we're only working with one chip with a non-split osc, *sigh* we can guess....
          }
        #endif
      #elif (F_CPU == 12800000 && defined(BOOTTUNED128)) || (F_CPU == 12000000 && defined(BOOTTUNED120))
        // boot tuned to desired frequency, don't need to do anything unless paranoid tuning.
        #if defined(ENABLE_TUNING) && (ENABLE_TUNING & 8)
          check_tuning(); // Don't trust that boot tuning happened; maybe it may get uploaded with programmer (paranoid tuning)
        #endif
      #else // 12.8 or 12.0 without boot tuning, so we use the tuning constants.
        #if defined(ENABLE_TUNING) && (ENABLE_TUNING & 3)  // necessary tuning, if ENABLE_TUNING & 3, we try to tune - unlike cases where we only check ENABLE_TUNING & 2 (tune always)
          check_tuning();      // here it started up at one tuned speed, we want a speed that isn't tuned.
        #else
          // can't make a general guess here, due to the high/low split in OSCCAL and variety of parts
          #error "The selected frequency requires tuning or a bootloader that leaves internal so tuned."
        #endif
      #endif
    #else
      CLKPR=1<<CLKPCE; //enable change of protected register
    #endif
    /* End of internal osc prescale and tuning */
  #elif (CLOCK_SOURCE == 0x11 || CLOCK_SOURCE == 0x12)
    // external 16MHz CLOCK or Crystal, but maybe they want to go slower to save power...
    // This is used only for board definitions where the crystal is forced to be 16 MHz and nothing else.
    // That is, on Micronucleus boards that use it for the bootloader. Many people use these as just an easy way
    // to program the part, and want to use them at low voltages or for low power applications after uploading
    // code via USB, so the answer of "use an appropriate crystal then" doesn't hold water. That, by the way
    // is my answer to anyone who wants this support for other crystal speeds.
    // Used for the commercially available MH-ET tiny88 (16 MHz external clock) and Digispark Pro
    /* Start of 16 MHz external with prescale */
    #if     (F_CPU != 16000000) // 16MHz is speed of external clock on these
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

void init() {
  init_clock(); // initialize the main system clock
  #ifdef SET_REMAP
    REMAP = SET_REMAP;
  #endif
  #ifdef SET_REMAPUSI
    USIPP = 1
  #endif
  /* Initialize Primary Timer */
  #if (TIMER_TO_USE_FOR_MILLIS == 0)
    #if defined(WGM01) // if Timer0 has PWM
      TCCR0A = (1<<WGM01) | (1<<WGM00);
    #endif
    #if defined(TCCR0B) //The x61 has a wacky Timer0!
      TCCR0B = (MillisTimer_Prescale_Index << CS00);

    #elif defined(TCCR0A)  // Tiny x8 has no PWM from timer0
      TCCR0A = (MillisTimer_Prescale_Index << CS00);
    #else // tiny26 has no TCCR0A at all, only TCCR0
      TCCR0 = (MillisTimer_Prescale_Index << CS00);
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
  #if defined( INITIALIZE_ADC ) && INITIALIZE_ADC
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


/* Okay, timer registers:
 * It is arguable whether it's actually better to check for these - the way we're doing it in these files,
 * we are often not checking for features, but specific families of parts handled case-by-case, and there
 * will never be new classic AVRs released.... so why not just test for part families when that's what we're doing?
 *
 * TCCR1E is only on x61.
 * TCCR1D is only on x7 and x61.
 * TCCR1 is only on x5
 * All non-85 have TCCR1A.
 *
 * Check for COM0xn bits to know if TIMER0 has PWM (it doesn't on x61 (it's a weird timer there - can be 16-bit) or
 * the x8 (because they cheaped out)
 */
