#include <avr/interrupt.h>
#define true 0x1
#define false 0x0

typedef enum {
    sFirstPass,
    sBigSteps,
    sConfirm,
    sFinished
} state_t;

typedef enum {
    pLeft,
    pThis,
    pRight,
    pMax
} position_t;

typedef struct {
    int16_t     OsccalValue;
    uint16_t    NineBitTime;
    int16_t     Error;
    uint8_t     ConfirmCount;
    uint16_t    ConfirmNineBitTime;
 //   int16_t     ConfirmClocks;  // rmv: Strictly for debugging.
} info_t;

typedef struct {
  uint8_t _state;
  uint8_t _position;
  uint8_t _threshold;
  info_t _info[pMax];
} tuner_t;

#if !defined(__AVR_ATtiny24__)
void putcal();
void putstr_t(const prog_char* str);
#endif
void putch_t(char ch);
void uartDelay_t() __attribute__ ((naked));
static uint16_t TimeNineBits( void );
static void AdjustOSCCAL( uint8_t NewValue );
static int8_t NumberOfBigSteps( int16_t error );
uint8_t update( tuner_t* tuner );
uint8_t FindBest( tuner_t* tuner );
void TransitionToConfirm( tuner_t* tuner );
void fullInit( tuner_t* tuner );

void tinyTuner(){
  //watchdogConfig(WATCHDOG_OFF);
  
#if defined(__AVR_ATtiny85__)
  TCCR1 = _BV(CS13) | _BV(CS12); // div 2048 as it is only 8bit, so having to half TCNT1 value
#elif !defined(__AVR_ATtiny24__)
  TCCR1B = _BV(CS12) | _BV(CS10); // div 1024
#endif
  /* Set LED pin as output */
  LED_DDR |= _BV(LED);
#if !defined(__AVR_ATtiny24__)
  LED_PORT &= ~ _BV(LED);
#endif
#ifdef SOFT_UART
  /* Set TX pin as output */
  UART_DDR |= _BV(UART_TX_BIT);
  UART_PORT |= _BV(UART_TX_BIT); //set high!
#endif
  //Tunes the oscillator (this code is overwritten by sketch files)
#if !defined(__AVR_ATtiny24__)
  putstr_t(PSTR("Entering Tiny Tuner\r\n\r\n"));
  putstr_t(PSTR("Poor Man's Internal Oscillator Tuner\r\nSlowly send lowercase 'x' to tune the oscillator...\r\n\r\n"));
  putstr_t(PSTR("Current Calibration = "));
  putch_t(OSCCAL);
  putstr_t(PSTR("\r\nLets see if we can do better\r\n\r\n"));

  flash_led(LED_START_FLASHES * 10);
#endif
  
  tuner_t tuner;
  tuner._state = sFirstPass;
  
  uint8_t running = true;
  
  while ( running )
  {
    running = update(&tuner);
    
#if !defined(__AVR_ATtiny24__)
	  putcal();
    uint8_t i=0;
    for ( ; i < 4; i++ ){
      if(!(i & 1)){
        LED_PIN |= _BV(LED); //toggle pin
      }
    #if defined(__AVR_ATtiny85__)
      TCNT1 = -(char)(F_CPU/(2048U*16U)); //clocks/sec * 1/16th second
      while(!(TIFR & _BV(TOV1)));
      TIFR = _BV(TOV1); //For tiny85, sbi() doesn't reach this register, so it saves memory to use an assignment rather than a bitwise or.
    #else
      TCNT1 = -(F_CPU/(1024*16)); //clocks/sec * 1/16th second
      //TIFR1 = _BV(TOV1);
      TIFR1 |= _BV(TOV1); //at boot TIFR1 is 0, so can just set the TOV1 bit - save memory.
      while(!(TIFR1 & _BV(TOV1)));
    #endif
    }
#else 
    LED_PIN |= _BV(LED); //toggle pin
#endif
  }
  
#if !defined(__AVR_ATtiny24__)
  putstr_t(PSTR("\r\n\r\nFinal Cal = "));
  putch_t(OSCCAL);
  putstr_t(PSTR("\r\nSaving Calibration to Program Memory...\r\n"));
#else
  putch_t('C');
  putch_t('=');
  putch_t(OSCCAL);
#endif
  
  
  uint16_t addrPtr;
  
  //For this code we are assuming that the cleared value of each byte in the temporary page buffer is 0xFF
  //This is important as we have to write a page at a time which means that we will be overwriting bytes we
  //don't want to change - by using 0xFF this isn't an issue as programming can only convert a bit from a 1
  //to a 0 (otherwise it needs to erase which is not being done here). So if say you had 0b00100101, and reprogrammed
  //it with 0b11111111, the result would be 0b00100101 as none on the 0's can be turned into 1's. 
  addrPtr = (uint16_t)(void*)ver;
  
  SPMCSR = CTPB; //clear the temporary page buffer - this sets all bytes to 0xFF so as not to change any bytes we don't want to
  twoByte oscProg;
  oscProg.array[1] = OSCCAL; //store the new OSCCAL value in the program memory so it can be restored by the bootloader at startup.
  oscProg.array[0] = (uint8_t)0x00; //prevents tinyTuner ever being called again (good as it will be overwritten by bootloader later.
  __boot_page_fill_short((uint16_t)(void*)addrPtr,oscProg.integer); //store the two oscProg bytes to the temporary buffer
  __boot_page_write_short((uint16_t)(void*)addrPtr); //program the whole page. Any byte where temp=0xFF will remain as they were.
  boot_spm_busy_wait(); //wait for completion

#if !defined(__AVR_ATtiny24__)
  putstr_t(PSTR("Removing call to TinyTuner to reduce bootloader size by 2.3kbytes\r\n"));
#endif
  
  addrPtr = (uint16_t)(void*)bootloader; //get the page on which to bootloader starts;
  addrPtr += 0x0A; //move to the correct place in the bootloader (where the RCALL to tinyTuner() is)
  
  SPMCSR = CTPB; //clear the temporary page buffer - this sets all bytes to 0xFF so as not to change any bytes we don't want to
  __boot_page_fill_short((uint16_t)(void*)addrPtr,(uint16_t)0x00); //write a NOP instruction to prevent calling tinyTuner when it doesn't exist anymore.
  __boot_page_write_short((uint16_t)(void*)addrPtr); //program the whole page. Any byte where temp=0xFF will remain as they were.
  boot_spm_busy_wait(); //wait for completion
  
#if !defined(__AVR_ATtiny24__)
  putstr_t(PSTR("Calibration saved and TinyTuner Deleted\r\n"));
  putstr_t(PSTR("\r\n\r\nEnabling Bootloader and Rebooting\r\n\r\n"));
#endif

  __asm__ __volatile__ (
#ifdef VIRTUAL_BOOT_PARTITION
    // Jump to WDT vector
    "ldi r30,4\n"
    "clr r31\n"
#else
    // Jump to RST vector
    "clr r30\n"
    "clr r31\n"
#endif
    "ijmp\n"
  );
  while(1); //to shut the compiler up - really the code doesn't return.
}

#if !defined(__AVR_ATtiny24__)
void putcal(){
  putstr_t(PSTR("Current Cal = "));
  putch_t(OSCCAL);
  putstr_t(PSTR("\r\n"));
}

void putstr_t(const prog_char *str){
  unsigned char c = pgm_read_byte(str++);
  while (c) {
    putch_t(c);
	c = pgm_read_byte(str++);
  }
}

#endif
#define BAUD_VALUE (((F_CPU/9600)-29)/6)
#if BAUD_VALUE > 255
#error Baud rate too slow for soft UART
#endif

void uartDelay_t() {
  __asm__ __volatile__ (
    "ldi r25,%[count]\n"
    "1:dec r25\n"
    "brne 1b\n"
    "ret\n"
    ::[count] "M" (BAUD_VALUE):"r25"
  );
}


void putch_t(char ch) {
  __asm__ __volatile__ (
    "   com %[ch]\n" // ones complement, carry set
    "   sec\n"
    "1: brcc 2f\n"
    "   cbi %[uartPort],%[uartBit]\n"
    "   rjmp 3f\n"
    "2: sbi %[uartPort],%[uartBit]\n"
    "   nop\n"
    "3: rcall uartDelay_t\n"
    "   rcall uartDelay_t\n"
    //"   rcall uartDelay_t\n"
    //"   rcall uartDelay_t\n"
    "   lsr %[ch]\n"
    "   dec %[bitcnt]\n"
    "   brne 1b\n"
    :
    :
      [bitcnt] "d" ((uint8_t)10),
      [ch] "r" (ch),
      [uartPort] "I" (_SFR_IO_ADDR(UART_PORT)),
      [uartBit] "I" (UART_TX_BIT)
    :
      "r25"
  );
}




uint8_t update( tuner_t* tuner ) {
  // Time the 'x'
  uint16_t nbt = TimeNineBits();
  
  // Calculate the number of clock cycles spent in TimeNineBits
  int16_t clocks = (nbt-1)*5 + 5;
  
  // Calculate the difference between the actual number of cycles spent in TimeNineBits and the expected number of cycles
  int16_t error = clocks - 7500;
  
  if ( tuner->_state == sFirstPass ) {
    tuner->_info[pLeft].OsccalValue  = -1;
    tuner->_info[pThis].OsccalValue  = OSCCAL & 0x7F;
    tuner->_info[pRight].OsccalValue = 0x80;
    tuner->_position = pThis;
    tuner->_state = sBigSteps;
    tuner->_threshold = 3;
  }
  
  if ( tuner->_state == sConfirm ) {
    uint16_t delta;
    info_t* info = &(tuner->_info[tuner->_position]);
    
    if ( nbt > info->NineBitTime ){
      delta = nbt - info->NineBitTime;
    } else {
      delta = info->NineBitTime - nbt;
    }
    
    info->NineBitTime = nbt;
    
    if ( (delta <= 2) || (info->ConfirmCount == 0) ) {
      ++info->ConfirmCount;
      info->ConfirmNineBitTime += nbt;
      
      if ( info->ConfirmCount >= tuner->_threshold ) {
        for ( tuner->_position=pLeft; tuner->_position < pMax; tuner->_position=(tuner->_position+1) ) {
          if ( tuner->_info[tuner->_position].ConfirmCount < tuner->_threshold ) {
            break;
          }
        }
        if ( tuner->_position == pMax ) {
          if ( FindBest(tuner) ) {
            tuner->_state = sFinished;
          } else {
            tuner->_threshold += 2;

            // fix? tuner->_threshold is unbounded.  At some point it may be prudent to just pick one of the two choices.
          }
        }
      }
    } else {
      info->ConfirmCount = 0;
      info->ConfirmNineBitTime = 0;
    }
  }

  if ( tuner->_state == sBigSteps ) {
    int8_t nobs = NumberOfBigSteps( error );

    if ( error < 0 ) {
      tuner->_info[pLeft].OsccalValue = tuner->_info[pThis].OsccalValue;
      tuner->_info[pLeft].NineBitTime = nbt;
      tuner->_info[pLeft].Error = -error;
      tuner->_info[pThis].OsccalValue += nobs;

      if ( tuner->_info[pThis].OsccalValue >= tuner->_info[pRight].OsccalValue ) {
        tuner->_info[pThis].OsccalValue = tuner->_info[pRight].OsccalValue - 1;

        if ( tuner->_info[pThis].OsccalValue <= tuner->_info[pLeft].OsccalValue ) {
          // fix? Do something special about the greater-than case?  If everything else is correct, it will never occur.
          if ( tuner->_info[pLeft].OsccalValue + 1 == tuner->_info[pRight].OsccalValue ) {
            TransitionToConfirm(tuner);
          } else {
            tuner->_info[pThis].OsccalValue = tuner->_info[pLeft].OsccalValue + 1;
          }
        }
      }
    } else {
      tuner->_info[pRight].OsccalValue = tuner->_info[pThis].OsccalValue;
      tuner->_info[pRight].NineBitTime = nbt;
      tuner->_info[pRight].Error = +error;
      tuner->_info[pThis].OsccalValue -= nobs;

      if ( tuner->_info[pThis].OsccalValue <= tuner->_info[pLeft].OsccalValue ) {
        tuner->_info[pThis].OsccalValue = tuner->_info[pLeft].OsccalValue + 1;

        if ( tuner->_info[pThis].OsccalValue >= tuner->_info[pRight].OsccalValue ) {
          // fix? Do something special about the less-than case?  If everything else is correct, it will never occur.
          if ( tuner->_info[pLeft].OsccalValue + 1 == tuner->_info[pRight].OsccalValue ) {
            TransitionToConfirm(tuner);
          } else {
            tuner->_info[pThis].OsccalValue = tuner->_info[pRight].OsccalValue - 1;
          }
        }
      }
    }
  }
  
  AdjustOSCCAL( (uint8_t)(tuner->_info[tuner->_position].OsccalValue) );
  
  if ( tuner->_state == sFinished ){
    return( false );
  }
  return( true );
}

uint8_t FindBest( tuner_t* tuner ) {
  // rmv uint16_t nbt;
  int16_t clocks;
  int16_t error;
  uint8_t position;
  int16_t BestError;
  uint8_t NeedToTryHarder;
  
  BestError = 0x7FFF;
  NeedToTryHarder = false;
  
  for ( position=pLeft; position < pMax; position++ ) {
    //rmv nbt = ( ( 2 * _info[position].ConfirmNineBitTime / _info[position].ConfirmCount ) + 1 ) / 2;
    //rmv clocks = (nbt-1)*5 + 5;
    clocks = (((((((uint32_t)(tuner->_info[position].ConfirmNineBitTime) - 1) * 5ul ) + 5ul) * 2ul) / tuner->_info[position].ConfirmCount) + 1ul) / 2ul;
    error = clocks - 7500;
    
    if ( error < 0 ) {
      error = -error;
    }
    
    // rmv: Strictly for debugging...
    // rmv _info[position].NineBitTime = nbt;
    //tuner->_info[position].ConfirmClocks = clocks;
    tuner->_info[position].Error = error;
    // ...rmv

    if ( error < BestError ) {
      BestError = error;
      tuner->_position = position;
      NeedToTryHarder = false;
    } else if ( error == BestError ) {
      tuner->_position = position;
      NeedToTryHarder = true;
    }
  }

  if ( NeedToTryHarder ) {
    return( false );
  }
  return( true );
}

void TransitionToConfirm( tuner_t* tuner ) {
  if ( tuner->_info[pLeft].Error < tuner->_info[pRight].Error ) {
    tuner->_info[pThis].OsccalValue = tuner->_info[pLeft].OsccalValue;
    tuner->_info[pThis].NineBitTime = tuner->_info[pLeft].NineBitTime;
    tuner->_info[pThis].Error = tuner->_info[pLeft].Error;
    
    tuner->_info[pLeft].OsccalValue = tuner->_info[pThis].OsccalValue - 1;
    tuner->_info[pLeft].ConfirmCount = 0;
    tuner->_info[pLeft].ConfirmNineBitTime = 0;
    
    tuner->_info[pThis].ConfirmCount = 1;
    tuner->_info[pThis].ConfirmNineBitTime = tuner->_info[pThis].NineBitTime;
    
    tuner->_info[pRight].ConfirmCount = 1;
    tuner->_info[pRight].ConfirmNineBitTime = tuner->_info[pRight].NineBitTime;
  } else {
    tuner->_info[pThis].OsccalValue = tuner->_info[pRight].OsccalValue;
    tuner->_info[pThis].NineBitTime = tuner->_info[pRight].NineBitTime;
    tuner->_info[pThis].Error = tuner->_info[pRight].Error;
    
    tuner->_info[pLeft].ConfirmCount = 1;
    tuner->_info[pLeft].ConfirmNineBitTime = tuner->_info[pLeft].NineBitTime;
    
    tuner->_info[pThis].ConfirmCount = 1;
    tuner->_info[pThis].ConfirmNineBitTime = tuner->_info[pThis].NineBitTime;
    
    tuner->_info[pRight].OsccalValue = tuner->_info[pThis].OsccalValue + 1;
    tuner->_info[pRight].ConfirmCount = 0;
    tuner->_info[pRight].ConfirmNineBitTime = 0;
  }
  tuner->_state = sConfirm;
}

static int8_t NumberOfBigSteps( int16_t error ) {
  error = error / 100;
  
  switch ( error )
  {
    case -7:  return( 20 );
    case -6:  return( 17 );
    case -5:  return( 15 );
    case -4:  return( 12 );
    case -3:  return(  9 );
    case -2:  return(  6 );
    case -1:  return(  3 );
    case  0:  return(  1 );
    case +1:  return(  3 );
    case +2:  return(  6 );
    case +3:  return(  9 );
    case +4:  return( 11 );
    case +5:  return( 13 );
    case +6:  return( 15 );
    case +7:  return( 17 );
  }
  return( error < 0 ? 21 : 18 );
}

static void AdjustOSCCAL( uint8_t NewValue ) {
  uint8_t Temp;
  uint8_t Value;
  uint8_t Range;
  
  Temp = OSCCAL;
  
  Value = Temp & 0x7F;
  Range = Temp & 0x80;
  
  if ( NewValue < Value ){
    while ( NewValue != Value ){
      --Value;
      OSCCAL = Range | Value;
    }
  } else if ( NewValue > Value ) {
    while ( NewValue != Value ) {
      ++Value;
      OSCCAL = Range | Value;
    }
  }
}

static uint16_t TimeNineBits( void ){
  // We need a fast (8 MHz) clock to maximize the accuracy
  #if (F_CPU != 8000000)
  uint8_t ClockDivisor = CLKPR;
  cli();
  CLKPR = _BV(CLKPCE);
  CLKPR = (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
  sei();
  #endif
  
  uint16_t Temp = 0;
  
  // lowercase 'x' on the wire...
  // ...1111111111 0 0001 1110 1 111111111...
  
  asm volatile
  (
    // Wait for a start bit
      "L%=wfsb: "
      "sbic  %[calreg], %[calbit]"              "\n\t"
      "rjmp  L%=wfsb"                           "\n\t"
      "cli"                                     "\n\t"

    // Ensure we can get exactly 7500 cycles (result = 1500)
      "nop"                                     "\n\t"
      "nop"                                     "\n\t"

    // Time the first segment (start bit + 3 least-significant bits of 'x'; lo bits)
      "L%=wfs1: "
      "adiw  %[reseult], 1"                     "\n\t"
      "sbis  %[calreg], %[calbit]"              "\n\t"
      "rjmp  L%=wfs1"                           "\n\t"

    // Time the second segment (middle bits of 'x'; hi bits)
      "L%=wfs2: "
      "adiw  %[reseult], 1"                     "\n\t"
      "sbic  %[calreg], %[calbit]"              "\n\t"
      "rjmp  L%=wfs2"                           "\n\t"

    // Finish at the third segment (most significant bit of 'x'; lo bit; hi stop bit terminates)
      "L%=wfs3: "
      "adiw  %[reseult], 1"                     "\n\t"
      "sbis  %[calreg], %[calbit]"              "\n\t"
      "rjmp  L%=wfs3"                           "\n\t"

      "sei"                                     "\n\t"

      : 
        [reseult] "+w" ( Temp )
      : 
      #ifdef UART_RX_PIN
        [calreg] "I" (_SFR_IO_ADDR(UART_RX_PIN)), //rx is on a different port.
      #else
        [calreg] "I" (_SFR_IO_ADDR(UART_PIN)),
      #endif
        [calbit] "I" ( UART_RX_BIT )
  );
  
  // Put the clock back the way we found it
  #if (F_CPU != 8000000)
  cli();
  CLKPR = _BV(CLKPCE);
  CLKPR = ClockDivisor;
  sei();
  #endif
  
  return( Temp );
}

void fullInit( tuner_t* tuner ) {
  tuner->_position = pMax;
  tuner->_threshold = 0;
  
  uint8_t p = pLeft;
  for ( ; p < pMax; p=(p+1) ) {
    tuner->_info[p].OsccalValue = 0;
    tuner->_info[p].NineBitTime = 0;
    tuner->_info[p].Error = 0;
    tuner->_info[p].ConfirmCount = 0;
    tuner->_info[p].ConfirmNineBitTime = 0;
    //tuner->_info[p].ConfirmClocks = 0;
  }
}