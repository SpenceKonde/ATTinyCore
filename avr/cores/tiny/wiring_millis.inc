/* This code is very long and very confusing. It was making it very difficult to work with
 * wiring.c, so it was moved into a .inc file that gets included in wiring.c unless millis
 * is disabled */


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
  #if MillisTimer_Prescale_Value <= 64 | (MillisTimer_Prescale_Value == 128 && F_CPU <= 20000000UL)
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
#define EXACT_REMAINDER (EXACT_NUMERATOR - (EXACT_NUMERATOR / EXACT_DENOMINATOR) * EXACT_DENOMINATOR)

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
  #define CORRECT_EXACT_MANY ((2U * 256U * EXACT_REMAINDER + EXACT_DENOMINATOR) / (2U * EXACT_DENOMINATOR))
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
    uint8_t t;
    uint8_t oldSREG = SREG;

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
        return (m - (m >> 5));        // but it's not - we want 32/33nds of that. We can't divide an unsigned long by 33 in a time sewnsitive function. So we do 31/32nds, and that's goddamned close.
      #elif (MillisTimer_Prescale_Value == 64 && F_CPU == 24000000L) // 2.6875 vs real value 2.67
        m = (m << 8) + t;
        return (m << 1) + (m >> 1) + (m >> 3) + (m >> 4); // multiply by 2.6875
      #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 20) // 3.187 vs real value 3.2
        m=(m << 8) + t;
        return m | (m << 1) | (m >> 2) | (m >> 4);
      #elif (MillisTimer_Prescale_Value == 64 && F_CPU == 18432000L) // 3.5 vs real value 3.47
        m=(m << 8) + t;
        return m | (m << 1) | (m >> 1);
      #elif (MillisTimer_Prescale_Value == 64 && F_CPU==14745600L) //4.375  vs real value 4.34
        m=(m << 8) + t;
        return (m << 2) | (m >> 1) | (m >> 3);
      #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 14) //4.5 - actual 4.57 for 14.0mhz, 4.47 for the 14.3 crystals scrappable from everything
        m=(m << 8) + t;
        return (m << 2) | (m >> 1);
      #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 12) // 5.3125 vs real value 5.333
        m=(m << 8) + t;
        return m | (m << 2) | (m >> 2) | (m >> 4);
      #elif (MillisTimer_Prescale_Value == 64 && clockCyclesPerMicrosecond() == 11) // 5.75 vs real value 5.818 (11mhz) 5.78 (11.059)
        m=(m << 8) + t;
        return m | (m << 2) | (m >> 1) | (m >> 2);
      #elif (MillisTimer_Prescale_Value == 64 && F_CPU==7372800L) // 8.625, vs real value 8.68
        m=(m << 8) + t;
        return (m << 3) | (m >> 2) | (m >> 3);
      #elif (MillisTimer_Prescale_Value == 64 && F_CPU==6000000L) // 10.625, vs real value 10.67
        m=(m << 8) + t;
        return (m << 3) | (m << 1) | (m >> 2) | (m >> 3);
      #elif (MillisTimer_Prescale_Value == 32 && F_CPU==7372800L) // 4.3125, vs real value 4.34 - x7 now runs timer0 twice as fast at speeds under 8 MHz
        m=(m << 8) + t;
        return (m << 2) | (m >> 3) | (m >> 4);
      #elif (MillisTimer_Prescale_Value == 32 && F_CPU==6000000L) // 5.3125, vs real value 5.33 - x7 now runs timer0 twice as fast at speeds under 8 MHz
        m=(m << 8) + t;
        return (m << 2) | (m) | (m >> 3) | (m >> 4);
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
        //return ((m << 8 )/clockCyclesPerMicrosecond()* MillisTimer_Prescale_Value) + ((t | (((m << 8)%clockCyclesPerMicrosecond())) * MillisTimer_Prescale_Value / clockCyclesPerMicrosecond()));
        return ((m << 8 )/clockCyclesPerMicrosecond()* MillisTimer_Prescale_Value) + (t * MillisTimer_Prescale_Value / clockCyclesPerMicrosecond());
      #endif
    #endif
  #endif // !CORRECT_EXACT_MICROS
  }

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
