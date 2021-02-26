#include "tuning.h"




uint8_t read_factory_calibration(void)
{
  uint8_t value = boot_signature_byte_get(1);
  return value;
}

#if ((F_CPU==16000000 || defined(LOWERCAL)) && CLOCK_SOURCE==0 )
  static uint8_t origOSC=0;

  uint8_t read_factory_calibration(void)
  {
    uint8_t SIGRD = 5; //Yes, this variable is needed. boot.h is looking for SIGRD but the io.h calls it RSIG... (unlike where this is needed in the other half of this core, at least the io.h file mentions it... ). Since it's actually a macro, not a function call, this works...
    uint8_t value = boot_signature_byte_get(1);
    return value;
  }
  void oscSlow(uint8_t newcal) {
    OSCCAL0=newcal;
    _NOP(); //this is all micronucleus does, and it seems to work fine...
  }

#endif


#if ((defined(__AVR_ATtinyX41__) && F_CPU==16000000) && CLOCK_SOURCE==0 )
  //functions related to the 16 MHz internal option on ATtiny841/441.
  // 174 CALBOOST seems to work very well - it gets almost all of them close enough for USART, which is what matters. It was empirically determined from a few parts I had lying around.
  #define CALBOOST 174
  #define MAXINITCAL (255-CALBOOST)
  static uint8_t saveTCNT=0;
  void oscBoost() {
    OSCCAL0=(origOSC>MAXINITCAL?255:(origOSC+CALBOOST));
    _NOP();
  }

  void oscSafeNVM() {      //called immediately prior to writing to EEPROM.
    TIMSK0&=~(_BV(TOIE0)); //turn off millis interrupt - let PWM keep running (Though at half frequency, of course!)
    saveTCNT=TCNT0;
    if (TIFR0&_BV(TOV0)) { // might have just missed interrupt - recording as 255 is good enough (this may or may not have been what we recorded, but if it was still set, interrupt didn't fire)
      saveTCNT=255;
    }
    oscSlow(origOSC);
  }
  void oscDoneNVM(uint8_t bytes_written) {
    /* That's the number of bytes of eeprom written, at 3.3ms or so each.
     * EEPROM does it one at a time, but user code could call these two methods when doing block writes (up to 64 bytes). Just be sure to do the eeprom_busy_wait(); at the end, as in EEPROM.h.
     * Not so much because it's a prerequisite for this stupid correction to timing but because cranking the oscillator back up during the write kinda defeats the point of slowing it doewn...
     * 3.3ms is good approximation of the duration of writing a byte - it'll be about 3~4% faaster since we're running around 5V at default call - hence, we're picking 3.3ms - the oscillator
     * adjustment loops and these calculations should be fast enough that the time they dont take long enough to worry about...
     * Srelies on assumptions from implementation above of millis on this part at 16MHz!
     * millis interrupt was disabled when oscSaveNVM() was called - so we don't need to do anything fancy to access the volatile variables related to it.
     * 1 millis interrupt fires every 1.024ms, so we want 3.3/1.024= 3.223 overflows; there are 256 timer ticksin an overflow, so 57 timer ticks...
     */
    oscBoost();
    uint8_t m = 3*bytes_written; //the 3 whole overflows
    uint16_t tickcount=57*bytes_written+saveTCNT;
    m+=(tickcount>>8); //overflows from theose extra /0.223's
    millis_timer_overflow_count+=m; //done with actual overflows, so record that.
    uint16_t f = FRACT_INC*m+millis_timer_fract; //(m could be up to 207)
    while(f>FRACT_MAX){ //at most 621+124=745
      f-=FRACT_MAX;
      m++;
    }
    // now we're adding up the contributions to millis from the 0.024 part...
    // save the results
    millis_timer_fract=f;
    millis_timer_millis+=m;
    TCNT0=0;
    TIFR0|=_BV(TOV0);   //clear overflow flag
    TIMSK0|=_BV(TOIE0); //enable overflow interrupt
    TCNT0=tickcount;    //restore new tick count
    // wonder if it was worth all that just to write to the EEPROM while running at 16MHz off internal oscillator without screwing up millis and micros...
  }
#endif
/* This attempts to grab a tuning constant from flash (if it's USING_BOOTLOADER) or EEPROM (if not). Note that it is not called unless ENABLE_TUNING is set.
 * inline for flash savings (call overhead) not speed; it is only ever called once, on startup, so I think it would get inlined anyway most of the time
 * addresses for key values:
 * FLASHEND is second byte of bootloader version, FLASHEND-1 is first byte. (all_
 * OFFSET:      Normal: PLLs: x41:      1634/828
 * LASTCAL-0   12.8    16.5  16.0 @ 5V0  12.8 @ 5V0
 * LASTCAL-1   12.0    16.0  12.8 @ 5V0  12.0 @ 5V0
 * LASTCAL-2    8.0    CUST  12.0 @ 5V0   8.0 @ 5V0
 * LASTCAL-3   CUST    0x00   8.0 @ 5V0   8.0 @ 3V3
 * LASTCAL-4                  8.0 @ 3V3  CUST
 * LASTCAL-5                  8.0 @ 3V3  0x00
 * ENABLE_TUNING values:
 * 1 = use for required changes (it started app boot-tuned wrong, so we need to fix it, or we wanted tuned frequency like 12 or 16.5 from non-bootloaded.
 * 2 = use stored cal contents for 8 MHz even if we start up like that.
 * 4 = enable custom tuning (CUST slot above)
 * 8 = paranoid - don't truest that boot tuning actually happened
 */
#if USING_BOOTLOADER
  #define read_tuning_byte(x) pgm_read_byte_near(x)
  #define LASTCAL FLASHEND
#else
  // without a bootloader, we have to store calibration in the EEPROM
  #define read_tuning_byte(x) eeprom_read_byte(x)
  #define LASTCAL E2END
#endif



static inline bool __attribute__((always_inline)) check_tuning() {
  // It is almost inconceivable that 0 would be desired tuning
  // If this is still 0 by the end, we didn't have a valid tuning constant.
  uint8_t tuneval = 0;
  #if (CLOCK_SOURCE == 6)
  /* start PLL timer tuning */
    #if (F_CPU == 16500000)
      tuneval=read_tuning_byte(LASTCAL - 0)
    #elif (F_CPU == 16000000) //16 or divided down 16.
      tuneval=read_tuning_byte(LASTCAL - 1)
    #else //
      #if (ENABLE_TUNING & 4)
        if (read_tuning_byte(LASTCAL - 3) != 0xFF) {
          // that is written to signify that custom tuning is present
          // Assume that any tuning value here is legitimate
          tuneval=read_tuning_byte(LASTCAL - 2)
        }
      #else
        #error "Custom tuning requested, but custom tuning not enabled!"
      #endif
    #endif
  /* end PLL timer tuning */
  #elif (CLOCK_SOURCE == 0)


  #else
  /* start non-tunable timer tuning */
    badCall("Call to check_tuning() detected, but we are using an external oscillator which we can't tune. This is a defect in ATTinyCore and should be reported to the drvelopers promptly");
    return false;
  /* end non-tunable timer tuning */
  #endif
  if (tuneval != 0) {
    set_OSCCAL(tuneval);
    return 1;
  }
  return 0;
}

inline void __attribute__((always_inline)) set_OSCCAL(uint8_t cal) {
  #ifdef OSCCAL0
    OSCCAL0=cal;
    _NOP();
    _NOP();
  #else
    OSCCAL=cal;
    _NOP();
    _NOP();
  #endif
}
