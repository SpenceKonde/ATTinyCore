/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 20 or 24 MHz clock. */
//Not used anymore, we have the version we stole from nerdralph's picocore!

/*
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
*/


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
