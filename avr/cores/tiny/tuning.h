#include <avr/io.h>
#ifndef __TUNING_H__
#include <avr/boot.h>


#ifndef SIGRD
  #define SIGRD 5
#endif

#if USING_BOOTLOADER
  #define read_tuning_byte(x) pgm_read_byte_near(x)
  #define LASTCAL FLASHEND
  #include <avr/pgmspace.h>
#else
  // without a bootloader, we have to store calibration in the EEPROM
  #define read_tuning_byte(x) eeprom_read_byte(x)
  #define LASTCAL E2END
  #include <avr/eeprom.h>
#endif


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

  void oscSafeNVM()
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

void oscDoneNVM(uint8_t bytes_written);
uint8_t read_factory_calibration(void);
  void oscSafeNVM(); //called immediately prior to

static inline bool __attribute__((always_inline)) check_tuning();

inline void __attribute__((always_inline)) set_OSCCAL(uint8_t cal);
