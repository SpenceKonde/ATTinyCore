/*-------------------------------------------------------------------------*
 * tinySPI.h - Arduino hardware SPI master library for ATtiny24/44/84,     *
 * ATtiny25/45/85, and Attiny2313/4313.                                    *
 *                                                                         *
 * Original version of tinyISP by Jack Christensen 24Oct2013               *
 *                                                                         *
 * Added support for Attiny24/25, and Attiny2313/4313                      *
 * by Leonardo Miliani 28Nov2014                                           *
 *                                                                         *
 * Reworked to add support for 87/167, 261/461/861, and include the normal *
 * SPI library on chips that have hardware SPI.                            *
 * Spence Konde 2016 (in progress)                                         *
 *                                                                         *
 * CC BY-SA-NC:                                                            *
 * This work is licensed under the Creative Commons Attribution-           *
 * ShareAlike- Not Commercial 4.0 Unported License. To view a copy of this *
 * license, visit                                                          *
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a                *
 * letter to Creative Commons, 171 Second Street, Suite 300,               *
 * San Francisco, California, 94105, USA.                                  *
 *-------------------------------------------------------------------------*/

#ifndef tinycoreSPI_h
#define tinycoreSPI_h

#include <stdint.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <Arduino.h>

#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif

// define SPI_AVR_EIMSK for AVR boards with external interrupt pins
#if defined(EIMSK)
  #define SPI_AVR_EIMSK  EIMSK
#elif defined(GICR)
  #define SPI_AVR_EIMSK  GICR
#elif defined(GIMSK)
  #define SPI_AVR_EIMSK  GIMSK
#endif


#ifdef SPDR //Then we have hardware SPI, let's use it:

// SPI_HAS_TRANSACTION means SPI has beginTransaction(), endTransaction(),
// usingInterrupt(), and SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// SPI_HAS_NOTUSINGINTERRUPT means that SPI has notUsingInterrupt() method
#define SPI_HAS_NOTUSINGINTERRUPT 1

// SPI_ATOMIC_VERSION means that SPI has atomicity fixes and what version.
// This way when there is a bug fix you can check this define to alert users
// of your code if it uses better version of this library.
// This also implies everything that SPI_HAS_TRANSACTION as documented above is
// available too.
#define SPI_ATOMIC_VERSION 1

// Uncomment this line to add detection of mismatched begin/end transactions.
// A mismatch occurs if other libraries fail to use SPI.endTransaction() for
// each SPI.beginTransaction().  Connect an LED to this pin.  The LED will turn
// on if any mismatch is ever detected.
//#define SPI_TRANSACTION_MISMATCH_LED 5

#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01  // SPI2X = bit 0 on SPSR

class SPISettings {
public:
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    if (__builtin_constant_p(clock)) {
      init_AlwaysInline(clock, bitOrder, dataMode);
    } else {
      init_MightInline(clock, bitOrder, dataMode);
    }
  }
  SPISettings() {
    init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
  }
private:
  void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    init_AlwaysInline(clock, bitOrder, dataMode);
  }
  void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
    __attribute__((__always_inline__)) {
    // Clock settings are defined as follows. Note that this shows SPI2X
    // inverted, so the bits form increasing numbers. Also note that
    // fosc/64 appears twice
    // SPR1 SPR0 ~SPI2X Freq
    //   0    0     0   fosc/2
    //   0    0     1   fosc/4
    //   0    1     0   fosc/8
    //   0    1     1   fosc/16
    //   1    0     0   fosc/32
    //   1    0     1   fosc/64
    //   1    1     0   fosc/64
    //   1    1     1   fosc/128

    // We find the fastest clock that is less than or equal to the
    // given clock rate. The clock divider that results in clock_setting
    // is 2 ^^ (clock_div + 1). If nothing is slow enough, we'll use the
    // slowest (128 == 2 ^^ 7, so clock_div = 6).
    uint8_t clockDiv;

    // When the clock is known at compile time, use this if-then-else
    // cascade, which the compiler knows how to completely optimize
    // away. When clock is not known, use a loop instead, which generates
    // shorter code.
    if (__builtin_constant_p(clock)) {
      if (clock >= F_CPU / 2) {
        clockDiv = 0;
      } else if (clock >= F_CPU / 4) {
        clockDiv = 1;
      } else if (clock >= F_CPU / 8) {
        clockDiv = 2;
      } else if (clock >= F_CPU / 16) {
        clockDiv = 3;
      } else if (clock >= F_CPU / 32) {
        clockDiv = 4;
      } else if (clock >= F_CPU / 64) {
        clockDiv = 5;
      } else {
        clockDiv = 6;
      }
    } else {
      uint32_t clockSetting = F_CPU / 2;
      clockDiv = 0;
      while (clockDiv < 6 && clock < clockSetting) {
        clockSetting /= 2;
        clockDiv++;
      }
    }

    // Compensate for the duplicate fosc/64
    if (clockDiv == 6)
    clockDiv = 7;

    // Invert the SPI2X bit
    clockDiv ^= 0x1;

    // Pack into the SPISettings class
    spcr = _BV(SPE) | _BV(MSTR) | ((bitOrder == LSBFIRST) ? _BV(DORD) : 0) |
      (dataMode & SPI_MODE_MASK) | ((clockDiv >> 1) & SPI_CLOCK_MASK);
    spsr = clockDiv & SPI_2XCLOCK_MASK;
  }
  uint8_t spcr;
  uint8_t spsr;
  friend class SPIClass;
};


class SPIClass {
public:
  // Initialize the SPI library
  static void begin();

  // If SPI is used from within an interrupt, this function registers
  // that interrupt with the SPI library, so beginTransaction() can
  // prevent conflicts.  The input interruptNumber is the number used
  // with attachInterrupt.  If SPI is used from a different interrupt
  // (eg, a timer), interruptNumber should be 255.
  static void usingInterrupt(uint8_t interruptNumber);
  // And this does the opposite.
  static void notUsingInterrupt(uint8_t interruptNumber);
  // Note: the usingInterrupt and notUsingInterrupt functions should
  // not to be called from ISR context or inside a transaction.
  // For details see:
  // https://github.com/arduino/Arduino/pull/2381
  // https://github.com/arduino/Arduino/pull/2449

  // Before using SPI.transfer() or asserting chip select pins,
  // this function is used to gain exclusive access to the SPI bus
  // and configure the correct settings.
  inline static void beginTransaction(SPISettings settings) {
    if (interruptMode > 0) {
      uint8_t sreg = SREG;
      noInterrupts();

      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        interruptSave = SPI_AVR_EIMSK;
        SPI_AVR_EIMSK &= ~interruptMask;
        SREG = sreg;
      } else
      #endif
      {
        interruptSave = sreg;
      }
    }

    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 1;
    #endif

    SPCR = settings.spcr;
    SPSR = settings.spsr;
  }

  // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
  inline static uint8_t transfer(uint8_t data) {
    SPDR = data;
    /*
     * The following NOP introduces a small delay that can prevent the wait
     * loop form iterating when running at the maximum speed. This gives
     * about 10% more speed, even if it seems counter-intuitive. At lower
     * speeds it is unnoticed.
     */
    asm volatile("nop");
    while (!(SPSR & _BV(SPIF))) ; // wait
    return SPDR;
  }
  inline static uint16_t transfer16(uint16_t data) {
    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
    in.val = data;
    if (!(SPCR & _BV(DORD))) {
      SPDR = in.msb;
      asm volatile("nop"); // See transfer(uint8_t) function
      while (!(SPSR & _BV(SPIF))) ;
      out.msb = SPDR;
      SPDR = in.lsb;
      asm volatile("nop");
      while (!(SPSR & _BV(SPIF))) ;
      out.lsb = SPDR;
    } else {
      SPDR = in.lsb;
      asm volatile("nop");
      while (!(SPSR & _BV(SPIF))) ;
      out.lsb = SPDR;
      SPDR = in.msb;
      asm volatile("nop");
      while (!(SPSR & _BV(SPIF))) ;
      out.msb = SPDR;
    }
    return out.val;
  }
  inline static void transfer(void *buf, size_t count) {
    if (count == 0) return;
    uint8_t *p = (uint8_t *)buf;
    SPDR = *p;
    while (--count > 0) {
      uint8_t out = *(p + 1);
      while (!(SPSR & _BV(SPIF))) ;
      uint8_t in = SPDR;
      SPDR = out;
      *p++ = in;
    }
    while (!(SPSR & _BV(SPIF))) ;
    *p = SPDR;
  }
  // After performing a group of transfers and releasing the chip select
  // signal, this function allows others to access the SPI bus
  inline static void endTransaction(void) {
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (!inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 0;
    #endif

    if (interruptMode > 0) {
      #ifdef SPI_AVR_EIMSK
      uint8_t sreg = SREG;
      #endif
      noInterrupts();
      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        SPI_AVR_EIMSK = interruptSave;
        SREG = sreg;
      } else
      #endif
      {
        SREG = interruptSave;
      }
    }
  }

  // Disable the SPI bus
  static void end();

  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setBitOrder(uint8_t bitOrder) {
    if (bitOrder == LSBFIRST) SPCR |= _BV(DORD);
    else SPCR &= ~(_BV(DORD));
  }
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setDataMode(uint8_t dataMode) {
    SPCR = (SPCR & ~SPI_MODE_MASK) | dataMode;
  }
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setClockDivider(uint8_t clockDiv) {
    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (clockDiv & SPI_CLOCK_MASK);
    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((clockDiv >> 2) & SPI_2XCLOCK_MASK);
  }
  // These undocumented functions should not be used.  SPI.transfer()
  // polls the hardware flag which is automatically cleared as the
  // AVR responds to SPI's interrupt
  inline static void attachInterrupt() { SPCR |= _BV(SPIE); }
  inline static void detachInterrupt() { SPCR &= ~_BV(SPIE); }

private:
  static uint8_t initialized;
  static uint8_t interruptMode; // 0=none, 1=mask, 2=global
  static uint8_t interruptMask; // which interrupts to mask
  static uint8_t interruptSave; // temp storage, to restore state
  #ifdef SPI_TRANSACTION_MISMATCH_LED
  static uint8_t inTransactionFlag;
  #endif
};

extern SPIClass SPI;


#else

#ifdef USICR //if we have a USI instead, use that

//SPI data modes
#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

#define SPI_CLOCK_DIV2       2
#define SPI_CLOCK_DIV4       4
#define SPI_CLOCK_DIV8       8
#define SPI_CLOCK_DIV16     16
#define SPI_CLOCK_DIV32     32
#define SPI_CLOCK_DIV64     64
#define SPI_CLOCK_DIV128   128

//This implementation does have transaction:
#define SPI_HAS_TRANSACTION 1
#define SPI_HAS_NOTUSINGINTERRUPT 1
// Settings for default USI based SPI bus for different chips

namespace USI_impl {
    using ClockOut = uint8_t (*)(uint8_t,uint8_t);
    uint8_t clockoutUSI(uint8_t data, uint8_t delay);
    uint8_t clockoutUSI2(uint8_t data, uint8_t delay);
    uint8_t clockoutUSI4(uint8_t data, uint8_t delay);
    uint8_t clockoutUSI8(uint8_t data, uint8_t delay);

    __attribute__((always_inline))
    inline ClockOut dispatchClockout(uint8_t div, uint8_t* delay)
    {
      *delay = 0;
      if (div <= 2) {
          return clockoutUSI2;
      } else if (div <= 4) {
          return clockoutUSI4;
      } else if (div <= 8) {
          return clockoutUSI8;
      } else {
        // Slow mode, convert clockdiv into delay loop count.
        // Calculated inline to allow compile-time evaluation.
        *delay = ((uint16_t)div*100 - 780) / 59;
        // Round it to nearest integer.
        *delay = *delay < 10 ? 1 : (*delay + 5) / 10;
        return clockoutUSI;
      }
    }

    ClockOut dispatchClockout_slow(uint8_t div, uint8_t* delay)
    __attribute__((warning("SPI clock is not a runtime constant, increasing code size a lot.")));
}

class SPISettings {
public:
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    init_AlwaysInline(clock, bitOrder, dataMode);
  }
  SPISettings() {
    init_AlwaysInline(F_CPU / 16, MSBFIRST, SPI_MODE0);
  }
private:
  void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
    __attribute__((always_inline)) {
    usicr = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK);
    if (dataMode == SPI_MODE1 || dataMode == SPI_MODE3) {
        usicr |= _BV(USICS0);
    }
    msb1st = bitOrder;
    cpol = dataMode == SPI_MODE2 || dataMode == SPI_MODE3;
    // Round up.
    uint8_t div = F_CPU / clock + (F_CPU % clock ? 1 : 0);
    if (__builtin_constant_p(clock)) {
      clockoutfn = USI_impl::dispatchClockout(div, &delay);
    } else {
      clockoutfn = USI_impl::dispatchClockout_slow(div, &delay);
    }
  }

  uint8_t msb1st;
  uint8_t cpol;
  uint8_t usicr;
  uint8_t delay;
  USI_impl::ClockOut clockoutfn;
  friend class SPIClass;
};

class SPIClass
{
 public:
  SPIClass();
  static void begin();
  static void beginTransaction(SPISettings settings);
  static uint8_t transfer(uint8_t data);
  static uint16_t transfer16(uint16_t data);
  static void transfer(void *buf, size_t count);
  static void endTransaction(void);
  static void end();

  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  static void setBitOrder(uint8_t bitOrder) {msb1st = bitOrder;}
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  static void setDataMode(uint8_t dataMode);
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  static void setClockDivider(uint8_t div) {
      if (__builtin_constant_p(div)) {
        clockoutfn = USI_impl::dispatchClockout(div, &delay);
      } else {
        clockoutfn = USI_impl::dispatchClockout_slow(div, &delay);
      }
  }

  static void usingInterrupt(uint8_t interruptNumber);
  static void notUsingInterrupt(uint8_t interruptNumber);

private:
  static void applySettings(SPISettings settings);

  static uint8_t msb1st;
  static uint8_t delay;
  static USI_impl::ClockOut clockoutfn;
  static uint8_t interruptMode; // 0=none, 1=mask, 2=global
  static uint8_t interruptMask; // which interrupts to mask
  static uint8_t interruptSave; // temp storage, to restore state
};

extern SPIClass SPI;



#else
//if no USICR and no TWBR

#error No supported hardware

#endif //end if USICR
#endif //end if TWBR
#endif //end of module include guard
