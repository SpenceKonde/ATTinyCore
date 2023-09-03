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


#if defined(__AVR_ATtiny167__) || defined(__AVR_ATtiny87__)
  #if F_CPU < 4000000L
    #define timer0Prescaler (0b010)
    #define timer0_Prescale_Value  (8)
  #elif F_CPU <= 8000000L
    #define timer0Prescaler (0b011)
    #define timer0_Prescale_Value  (32)
  #elif F_CPU < 16000000L
    #define timer0Prescaler (0b100)
    #define timer0_Prescale_Value  (64)
  #else
    #define timer0Prescaler (0b101)
    #define timer0_Prescale_Value  (128)
  #endif
#else
  #if F_CPU <= 4000000L
    #define timer0Prescaler (0b010)
    #define timer0_Prescale_Value  (8)
  #else
    #define timer0Prescaler (0b011)
    #define timer0_Prescale_Value  (64)
  #endif
#endif
/* So that's TC0 done */


#if defined(TCCR1E)  //  x61
  #if F_CPU < 2000000L
    #define timer1Prescaler (0b0011)
    #define timer1_Prescale_Value  (4)
  #elif F_CPU <= 4000000L
    #define timer1Prescaler (0b0100)
    #define timer1_Prescale_Value  (8)
  #elif F_CPU <  8000000L
    #define timer1Prescaler (0b0101)
    #define timer1_Prescale_Value  (16)
  #elif F_CPU <= 16000000L
    #define timer1Prescaler (0b0110)
    #define timer1_Prescale_Value  (32)
  #else
    #define timer1Prescaler (0b0111)
    #define timer1_Prescale_Value  (64)
  #endif
#elif (defined(TCCR1)) // x5
  #define TIMER1_USE_FAST_PWM
  #if F_CPU < 2000000L
    #define timer1Prescaler (0b0100)
    #define timer1_Prescale_Value  (8)
  #elif F_CPU <= 4000000L
    #define timer1Prescaler (0b0101)
    #define timer1_Prescale_Value  (16)
  #elif F_CPU <  8000000L
    #define timer1Prescaler (0b0110)
    #define timer1_Prescale_Value  (32)
  #elif F_CPU <= 16000000L
    #define timer1Prescaler (0b0111)
    #define timer1_Prescale_Value  (64)
  #else
    #define timer1Prescaler (0b1000)
    #define timer1_Prescale_Value  (128)
  #endif
#else
  #if F_CPU < 8000000L
    #define timer1Prescaler (0b010)
    #define timer1_Prescale_Value  (8)
    #if F_CPU < 4000000L
      #define TIMER1_USE_FAST_PWM
    #endif
  #else
    #define timer1Prescaler (0b011)
    #define timer1_Prescale_Value  (64)
    #if F_CPU <= 16000000L
      #define TIMER1_USE_FAST_PWM
    #endif
  #endif
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
  // 20 MHz / 128 = 157 kHz
  // 16 MHz / 128 = 125 kHz
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
#if INITIALIZE_SECONDARY_TIMERS
static void initToneTimerInternal(void);
#endif
static void initMillisTimer();
static void __empty() {
  // Empty
}
void yield(void) __attribute__ ((weak, alias("__empty")));



#ifndef DISABLEMILLIS
  #include "wiring_millis.inc"
#else //if DISABLEMILLIS is set, need no millis, micros, and different delay
  // delay without millis
  void _delay(uint32_t ms) {
    while(ms--) {
      yield();
      delayMicroseconds(999);
    }
  }
  inline void __attribute__ ((always_inline)) delay(uint32_t ms) { // non-millis-timer-dependent delay()
    if (__builtin_constant_p(ms)) {
      _delay_ms(ms); //if its a compile time known constant use the avrlibc version
    } else {
      _delay(ms);
    }
  }
#endif

/* This attempts to grab a tuning constant from flash (if it's USING_BOOTLOADER) or EEPROM (if not). Note that it is not called unless ENABLE_TUNING is set.
 * inlined for flash savings (call overhead) not speed; it is only ever called once, on startup, so I think it would get inlined anyway most of the time
 * addresses for key values:
 * FLASHEND is second byte of bootloader version, FLASHEND-1 is first byte. (all)
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
 * 1 = use for required changes (it started app boot-tuned wrong, so we need to fix it, or we wanted tuned frequency like 12 or 16.5 from non-bootloaded).
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
            CLKPR=1 << CLKPCE; //enable change of protected register
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
      { // if that fails, or tuning isn't enabled, we do the guess :-/
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
          CLKPR=1 << CLKPCE; //enable change of protected register
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
      CLKPR=1 << CLKPCE; //enable change of protected register
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
        CLKPR=1 << CLKPCE; //enable change of protected register
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
}
void init() {
  init_clock(); // initialize the main system clock
  #ifdef SET_REMAP
    REMAP = SET_REMAP;
  #endif
  #ifdef SET_REMAPUSI
    USIPP = 1;
  #endif
  initMillisTimer();
  // Initialize the timer used for Tone
  #if INITIALIZE_SECONDARY_TIMERS
    initToneTimerInternal();
  #endif
  // Initialize the ADC
  #if defined(INITIALIZE_ADC) && INITIALIZE_ADC
    #if defined(ADCSRA)
      // set a2d prescale factor
      // ADCSRA = (ADCSRA & ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))) | (ADC_ARDUINO_PRESCALER << ADPS0) | (1 << ADEN);
      // dude, this is being called on startup. We know that ADCSRA is 0! Why add a RMW cycle?!
      ADCSRA = (ADC_ARDUINO_PRESCALER << ADPS0) | (1 << ADEN);
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
 * The x7 has weird things about all it's timers. TC0 is strange, and TC1 has this crazy output mux.
 * The timers on the x61 are MUCH wierder. So both of those need special handling, which is kind of a rook.
 * Gotta jump through hoops like you were a circus animal just to get an 'x7 to give you just three channels at the
 * same frequency....
 *
 * TCCR1 is only on x5
 *
 * All non-85 have TCCR1A.
 *
 * Check for COM0xn bits to know if TIMER0 has PWM (it doesn't on x61 - it's a weird timer there - can be 16-bit,
 * and has output compare units that just generate interrupts. General freakshow like everything else on those parts.
 * And it doesn't on an x8 'cause Atmel cheaped out).
 */
void initMillisTimer() {
  /* Initialize Primary Timer */
  #if (TIMER_TO_USE_FOR_MILLIS == 0)
    #if defined(WGM01) // if Timer0 has PWM
      TCCR0A = (1 << WGM01) | (1 << WGM00);
    #endif
    #if defined(TCCR0B) //The x61 has a wacky Timer0!
      TCCR0B = (MillisTimer_Prescale_Index << CS00);

    #elif defined(TCCR0A)  // Tiny x8 has no PWM from timer0
      TCCR0A = (MillisTimer_Prescale_Index << CS00);
    #else // tiny26 has no TCCR0A at all, only TCCR0
      TCCR0 = (MillisTimer_Prescale_Index << CS00);
    #endif
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1) //ATtiny x5
    TCCR1 = (1 << CTC1) | (1 << PWM1A) | (MillisTimer_Prescale_Index << CS10);
    GTCCR = (1 << PWM1B);
    // OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
    // Don't need to write OCR1C - it's already set to 255 on poweron.
  #elif (TIMER_TO_USE_FOR_MILLIS == 1) && defined(TCCR1E) //ATtiny x61
    TCCR1C = 1 << PWM1D;
    TCCR1B = (MillisTimer_Prescale_Index << CS10);
    TCCR1A = (1 << PWM1A) | (1 << PWM1B);
    //cbi(TCCR1E, WGM10); //fast pwm mode
    //cbi(TCCR1E, WGM11);
    OCR1C = 0xFF; //Use 255 as the top to match with the others as this module doesn't have a 8bit PWM mode.
  #elif (TIMER_TO_USE_FOR_MILLIS == 1)
    TCCR1A = 1 << WGM10;
    TCCR1B = (1 << WGM12) | (MillisTimer_Prescale_Index << CS10);
  #endif

  // this needs to be called before setup() or some functions won't work there
  sei();

  #ifndef DISABLEMILLIS
    // Enable the overflow interrupt (this is the basic system tic-toc for millis)
    #if defined(TIMSK) && defined(TOIE0) && (TIMER_TO_USE_FOR_MILLIS == 0)
      TIMSK |= (1 << TOIE0); //sbi(TIMSK,TOIE0);
    #elif defined(TIMSK0) && defined(TOIE0) && (TIMER_TO_USE_FOR_MILLIS == 0)
      TIMSK0 |= (1 << TOIE0); //sbi(TIMSK0,TOIE0);
    #elif defined(TIMSK) && defined(TOIE1) && (TIMER_TO_USE_FOR_MILLIS == 1)
      TIMSK |= (1 << TOIE1); //sbi(TIMSK,TOIE1);
    #elif defined(TIMSK1) && defined(TOIE1) && (TIMER_TO_USE_FOR_MILLIS == 1)
      TIMSK1 |= (1 << TOIE1); //sbi(TIMSK1,TOIE1);
    #else
      #error Millis() Timer overflow interrupt not set correctly
    #endif
  #endif
}
// This clears up the timer settings, and then calls the tone timer initialization function (unless it's been disabled - but in this case, whatever called this isn't working anyway!
// Note that this is used **only** when directly called - and serves only to reset the timers to stock state. The initToneTimerInternal() does the other half, and is called by this
// 2023 - commented out a few register writes in here because we then rewrite the same registers a few moments later.
void initToneTimer(void) {
  // Ensure the timer is in the same state as power-up, except for any registers we write over with straight assignment in initToneTimerInternal() other than
  // TCCRnB and TIMSK/TIMSKn, which are zero'ed to turn off interrupts and stop the timer.
  #if defined(__AVR_ATtiny43__)   // Always uses TC0 for millis and TC1 for Tone(), both are 8-bit
    TIMSK1 = 0;
    TCCR1B = 0;
    //TCCR1A = 0;
    TCNT1  = 0;
    OCR1A  = 0;
    OCR1B  = 0;
    TIFR1  = 0x07;
  #elif defined(__AVR_ATtiny26__) // Always uses low speed timer.
    TIMSK  &= 2;
    //TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = 0;
    OCR1B  = 0;
    TIFR   = 0x66;
  #elif (TIMER_TO_USE_FOR_TONE == 0)
    #if !defined(DISABLE_MILLIS) && defined(TIMER_TO_USE_FOR_TONE) && (!defined(TIMER_TO_USE_FOR_MILLIS) || TIMER_TO_USE_FOR_MILLIS == 0 )
      /* Policy: If we know some combination of options will result in behavior that is not plausaibly intended behavior, and we are in a position
       * to recognize this at compile time, we should do so and error out; this helps compensate for poor debuggability of the tinyAVR parts without
       * uncommon programming hardware and official Microchip software. */
      #error "Core defect - Please report this issue along with the settings that generated it"
    #endif
    // Just zero the registers out, instead of trying to name all the bits, as there are combinations of hardware and settings where that doesn't work
    TCCR0B = 0; //  (0 << FOC0A) | (0 << FOC0B) | (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);
    //TCCR0A = 0; // (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
    // Reset the count to zero
    TCNT0 = 0;
    // Set the output compare registers to zero
    OCR0A = 0;
    OCR0B = 0;
    #if defined(TIMSK)
      // Disable all Timer0 interrupts
      // Clear the Timer0 interrupt flags
      #if defined(TICIE0) // x61-series has an additional input capture interrupt vector...
        TIMSK &= ~((1 << OCIE0B) | (1 << OCIE0A) | (1 << TOIE0) | (1 << TICIE0));
        TIFR = ((1 << OCF0B) | (1 << OCF0A) | (1 << TOV0) | (1 << ICF0));
      #else
        TIMSK &= ~((1 << OCIE0B) | (1 << OCIE0A) | (1 << TOIE0));
        TIFR = ((1 << OCF0B) | (1 << OCF0A) | (1 << TOV0));
      #endif
    #elif defined(TIMSK0)
      // Disable all Timer0 interrupts
      TIMSK0 = 0; //can do this because all of TIMSK0 is timer 0 interrupt masks
      // Clear the Timer0 interrupt flags
      TIFR0 = ((1 << OCF0B) | (1 << OCF0A) | (1 << TOV0)); //no ICF0 interrupt on any supported part with TIMSK0
    #endif
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1) // t85
    // Disable all Timer1 interrupts
    TIMSK &= ~((1 << OCIE1A) | (1 << OCIE1B) | (1 << TOIE1));
    // Turn off Clear on Compare Match, turn off PWM A, disconnect the timer from the output pin, stop the clock
    TCCR1 = 0;
    // 0 out TCCR1
    // Turn off PWM A, disconnect the timer from the output pin, no Force Output Compare Match, no Prescaler Reset
    // GTCCR = GTCCR & (~((1 << PWM1B) | (1 << COM1B1) | (1 << COM1B0) | (1 << FOC1B) | (1 << FOC1A) | (1 << PSR1)));
    //  set in initToneTimerInternal - and hardly any of these values are ever used.

    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    OCR1C = 0xFF;
    // Clear the Timer1 interrupt flags
    TIFR = ((1 << OCF1A) | (1 << OCF1B) | (1 << TOV1));
  #elif (TIMER_TO_USE_FOR_TONE == 1) && defined(TCCR1E) // x61
    // turn off all interrupts
    TIMSK &= ~((1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B) | (1 << OCIE1D));
    // TCCR1A = 0; set in initToneTimerInternal
    TCCR1B = 0;
    //TCCR1C = 0;  set in initToneTimerInternal
    //TCCR1D = 0;  set in initToneTimerInternal
    TCCR1E = 0;
    // Reset the count to zero
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A  = 0;
    OCR1B  = 0;
    OCR1C  = 0xFF;
    OCR1D  = 0;
    // Clear the Timer1 interrupt flags
    TIFR   =  ((1 <<  TOV1) | (1 <<  OCF1A) | (1 <<  OCF1B) | (1 <<  OCF1D));

  #elif (TIMER_TO_USE_FOR_TONE == 1)
    // Normal, well-behaved 16-bit Timer 1.
    // Turn off Input Capture Noise Canceler, Input Capture Edge Select on Falling, stop the clock
    TCCR1B = 0;
    // Disconnect the timer from the output pins, Set Waveform Generation Mode to Normal
    TCCR1A = 0;
    TCNT1 = 0;
    // Set the output compare registers to zero
    OCR1A = 0;
    OCR1B = 0;
    // Disable all Timer1 interrupts
    #if defined(TIMSK)
      TIMSK &= ~((1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B) | (1 << ICIE1));
      // Clear the Timer1 interrupt flags
      TIFR = ((1 << TOV1) | (1 << OCF1A) | (1 << OCF1B) | (1 << ICF1));
    #elif defined(TIMSK1)
      // Disable all Timer1 interrupts
      TIMSK1 = 0; //~((1 << TOIE1) | (1 << OCIE1A) | (1 << OCIE1B) | (1 << ICIE1));
      // Clear the Timer1 interrupt flags
      TIFR1 = ((1 << TOV1) | (1 << OCF1A) | (1 << OCF1B) | (1 << ICF1));
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
    #if defined(TIMER_TO_USE_FOR_TONE)
      #if (TIMER_TO_USE_FOR_TONE == 0)
        #warning "ATTinyCore only supports using Timer1 for tone - this is untested code!"
        // Use the Tone Timer for phase correct PWM
        TCCR0A = (1 << WGM00);
        TCCR0B = (ToneTimer_Prescale_Index << CS00);
      #endif
    #else
      #warning "Core defect: TIMER_TO_USE_FOR_TONE undefined! Please report along with what part and settings you used"
      #define TIMER_TO_USE_FOR_TONE (1)
    #endif
    /* Now the case we expect to work, TC1 */
    #if (TIMER_TO_USE_FOR_TONE == 1)
      #if defined(__AVR_ATtinyX41__)

        TCCR1A   = (1 << WGM10) | (1 << COM1A1)| (1 << COM1B1); // enable OC1A, OC1B
        TCCR2A   = (1 << WGM20) | (1 << COM2A1)| (1 << COM2B1); // enable OC2A, OC2B
        TOCPMSA0 = 0b00010000; // PA4: OC0A, PA3: OC1B, PA2: N/A,  PA1: N/A
        TOCPMSA1 = 0b10100100; // PB2: OC2A, PA7: OC2B, PA6: OC1A, PA5: OC0B
        #if !defined(TIMER1_USE_FAST_PWM)
          TCCR1B   = (ToneTimer_Prescale_Index << CS10); // set the clock - do this last, always!
          TCCR2B   = (ToneTimer_Prescale_Index << CS10); // set the clock - cause it starts the timer!
        #else
          TCCR1B   = (1 << WGM12) | (ToneTimer_Prescale_Index << CS10); // set the clock - do this last, always!
          TCCR2B   = (1 << WGM22) | (ToneTimer_Prescale_Index << CS10); // set the clock - cause it starts the timer!
        #endif
        // TOCPMCOE = 0; // keep these disabled!
      #elif defined(__AVR_ATtiny828__)
        TCCR1A   = (1 << WGM10) | (1 << COM1A1)| (1 << COM1B1); // enable OC1A, OC1B
        TOCPMSA0 = (0b11100100);  // PC3: OC1B, PC2: OC1A, PC1: OC0B, PC0 OC0A.
        TOCPMSA1 = (0b11001001);  // PC7: OC1B, PC6: OC0A, PC5: OC1A, PC4,OC0B
        // TOCPMCOE = 0; // keep these disabled!
        #if !defined(TIMER0_USE_FAST_PWM)
          TCCR1B   =(ToneTimer_Prescale_Index << CS10); // set the clock
        #else
          TCCR1B   =(1 << WGM12) | (ToneTimer_Prescale_Index << CS10); // set the clock
        #endif
      #elif defined(__AVR_ATtiny43__)
        #if !defined(TIMER0_USE_FAST_PWM)
          TCCR1A = 1; //WGM 10=1, WGM11=1 // Phase correct
        #else
          TCCR1A = 3; //WGM 10=1, WGM11=1 // Fast
        #endif
        TCCR1B = (ToneTimer_Prescale_Index << CS10);
      #elif defined(TCCR1) // ATtiny x5
        // Use the Tone Timer for fast PWM as phase correct not supported by this timer
        GTCCR = (1 << PWM1B);
        TCCR1 = (1 << CTC1) | (1 << PWM1A) | (ToneTimer_Prescale_Index << CS10);
        /* Fast mode the only option here! */
      #elif defined(TCCR1E) // ATtiny x61
        // Use the Tone Timer for phase correct PWM
        TCCR1A = (1 << PWM1A) | (1 << PWM1B);
        TCCR1C = (1 << PWM1D);
        #if !defined(TIMER1_USE_FAST_PWM)
          TCCR1D = (1 << WGM10);
        #else
          TCCR1D = 0
        #endif
        TCCR1B = (ToneTimer_Prescale_Index << CS10);
      #elif  defined(__AVR_ATtinyX7__)
        /* Like the 841/441/828 we turn on the output compare, and analogWrite() only twiddles the enable bits */
        TCCR1A = (1 << COM1A1)  |(1 << COM1B1) | (1 << WGM10);
        #if !defined(TIMER1_USE_FAST_PWM)
          TCCR1B = (ToneTimer_Prescale_Index << CS10);
        #else
          TCCR1B = (1 << WGM12) | (ToneTimer_Prescale_Index << CS10);
        #endif
        TCCR1D = 0;
      #elif defined(__AVR_ATtiny26__)
        TCCR1A = (1 << PWM1A) | (1 << PWM1B);
        TCCR1B = ToneTimer_Prescale_Index;
      #else // x4, x8, x313,
        // Use the Tone Timer for PWM
          TCCR1A = (1 << WGM10);
        #if !defined(TIMER1_USE_FAST_PWM)
          TCCR1B = (ToneTimer_Prescale_Index << CS10); //set the clock
        #else
          TCCR1B = (1 << WGM12) | (ToneTimer_Prescale_Index << CS10); //set the clock
        #endif
      #endif
    #else
      #error "The selected tone timer does not exist. "
    #endif
  }
#endif
