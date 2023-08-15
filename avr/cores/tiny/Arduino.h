#ifndef Arduino_h
#define Arduino_h

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "BasicMacros.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "binary.h"

#ifdef __cplusplus
  extern "C"{
#endif

#ifndef ATTINYCORE
  #define ATTINYCORE "2.x.x+ unknown"
#endif

void yield(void);

#define ADC_ERROR_NO_ADC          -32768
#define ADC_ERROR_DISABLED        -32767 /* ADC_ERROR_NO_ADC + 1 */
#define ADC_ERROR_BUSY            -32766 /* ADC_ERROR_NO_ADC + 2 */
#define ADC_ERROR_NOT_A_CHANNEL   -32765 /* ADC_ERROR_NO_ADC + 3 */
#define ADC_ERROR_SINGLE_END_IPR  -32764 /* ADC_ERROR_NO_ADC + 4 */

#ifndef USING_BOOTLOADER
  #define USING_BOOTLOADER 0
#endif


#if (F_CPU < 1000000L)
  //Prevent a divide by 0 is
  #warning "Clocks per microsecond < 1. To prevent divide by 0, it is rounded up to 1."
  #define clockCyclesPerMicrosecond() 1UL
#else
  #define clockCyclesPerMicrosecond() (F_CPU / 1000000UL)
#endif

#define ADC_CH(x) (0x80 | (x))

//#define clockCyclesToMicroseconds(a) (((a) * 1000L) / (F_CPU / 1000L))
//#define microsecondsToClockCycles(a) (((a) * (F_CPU / 1000L)) / 1000L)

// Compile-time error checking functions

void badArg(const char*) __attribute__((error("")));
// badArg is when we can determine at compile time that an argument is inappropriate.

void badCall(const char*) __attribute__((error("")));
// badCall is used when the function should not be called, period (for the selected part with the currently selected options for all tools submenus)
// and calling it with different arguments can't change that.

typedef unsigned int word;


typedef uint8_t boolean;
typedef uint8_t byte;

void initToneTimer(void);
void init(void);

int main() __attribute__((weak));

void pinMode(uint8_t pinNumber, uint8_t mode);
void digitalWrite(uint8_t pinNumber, uint8_t val);
int8_t digitalRead(uint8_t pinNumber);
/* Copy of the ones on the modern AVR cores I maintain */
void openDrain(uint8_t pin, uint8_t mode);




int analogRead(uint8_t pinNumber);
#ifdef SLEEP_MODE_ADC
  int analogRead_NR(uint8_t pin);
  int _analogRead(uint8_t pin, bool use_noise_reduction);
#else
  int _analogRead(uint8_t pin);
#endif

void analogReference(uint8_t mode);
void analogWrite(uint8_t, int);

void setADCDiffMode(bool bipolar);
void analogGain(uint8_t gain);

#if !defined(DISABLE_MILLIS)
  unsigned long millis(void);
  unsigned long micros(void);
  // So you can test #ifdef millis
  #define millis millis
  #define micros micros
#else
  unsigned long millis(void) {
    badCall("Millis is disabled from the tools menu");
    return -1;
  }
  unsigned long micros(void) {
    badCall("Millis is disabled from the tools menu");
    return -1;
  }
#endif
void delay(unsigned long);

// Shamelessly stolen from @nerdralph's picoCore
// delays a specified number of microseconds
// works for clock frequencies of 1Mhz and up
__attribute((always_inline))
static inline void delayMicroseconds(uint16_t us)
{
    // if us is a compile-time constant result is accurate to 1 cycle
    if (__builtin_constant_p(us)) {
        _delay_us(us);
        return;
    }

    // when us is not known at compile time, delay is accurate to +/- 2us
    // plus an overhead of 3 CPU cycles
    const float fMHz = (F_CPU/1000000.0);
    // subtract two for rounding before dividing by 4
    us -= 2;
    delay4us:
        // delay 4us per loop, less 4 cycles for overhead
        _delay_us(4.0 - (4.0 / fMHz));
        asm volatile ("sbiw %[us], 4" : [us]"+w"(us));
    asm goto( "brpl %l[delay4us]" :::: delay4us);
}


unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
#ifndef DISABLEMILLIS
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);
#endif

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

void setup(void);
void loop(void);



// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.

extern const uint8_t PROGMEM port_to_mode_PGM[];
extern const uint8_t PROGMEM port_to_input_PGM[];
extern const uint8_t PROGMEM port_to_output_PGM[];
#if defined PUEA
  extern const uint8_t PROGMEM port_to_pullup_PGM[];
#endif

extern const uint8_t PROGMEM digital_pin_to_port_PGM[];
extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[];
extern const uint8_t PROGMEM digital_pin_to_timer_PGM[];

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
//
// These perform slightly better as macros compared to inline functions
//
#define const_array_or_pgm_(FUNC,ARR,IDX) ({size_t idx_ = (IDX); __builtin_constant_p((ARR)[idx_]) ? (ARR)[idx_] : FUNC((ARR)+idx_); })
#define digitalPinToPort(P)     (((P) < NUM_DIGITAL_PINS) ? const_array_or_pgm_(pgm_read_byte, digital_pin_to_port_PGM, (P)) : NOT_A_PORT)
#define digitalPinToBitMask(P)  (((P) < NUM_DIGITAL_PINS) ? const_array_or_pgm_(pgm_read_byte, digital_pin_to_bit_mask_PGM, (P)) : NOT_A_PIN)
#define digitalPinToTimer(P)    (const_array_or_pgm_(pgm_read_byte, digital_pin_to_timer_PGM, (P)))
#define analogInPinToBit(P)     (P)
#define portOutputRegister(P)   ((volatile uint8_t *)(uint16_t)(const_array_or_pgm_(pgm_read_byte, port_to_output_PGM, (P))))
#define portInputRegister(P)    ((volatile uint8_t *)(uint16_t)(const_array_or_pgm_(pgm_read_byte, port_to_input_PGM, (P))))
#define portModeRegister(P)     ((volatile uint8_t *)(uint16_t)(const_array_or_pgm_(pgm_read_byte, port_to_mode_PGM, (P))))
#if defined(PUEA)
  #define portPullupRegister(P) ((volatile uint8_t *)(uint16_t)(const_array_or_pgm_(pgm_read_byte, port_to_pullup_PGM, (P))))
#endif

#define NOT_A_PIN     255
#define NOT_A_TIMER   255 /* Not the same as NOT_ON_TIMER */
#define NOT_A_PORT    0   /* Different from modern AVR!   */
#define NOT_A_CHANNEL 127

#define PA 1
#define PB 2
#define PC 3
#define PD 4

#define NOT_ON_TIMER 0
#define TIMER0A 1   /* most parts - but not x61, or x8. On the x61, normal timers wouldn't put up with the weird timer1, and they had to find a timer just as weird for timer0. In contrast, the x8 just cheaped out */
#define TIMER0B 2   /* most parts - but not x61, x7, or x8. As above, but for reasons that I don't quite understand timer0 only got 1 channel on the x7 */
#define TIMER1A 3   /* all parts? If not all, almost all */
#define TIMER1B 4   /* all parts? If not all, almost all */
#define TIMER2A 5   /* x41 only - nothing else has 3 timers */
#define TIMER2B 6   /* x41 only - nothing else has 3 timers */

#define TIMER1D 7   /* x61 only - the usable half of the third phase of the 3-phase motor driving functionality.  */

/* This is TRICKY
  We need the bitmask, not bit position when we use this.
  The analogWrite() function is going to need to know the
  timer + channel AND the TOCC bitmask, with the minimum
  possible effort. In this case, the byte breaks down as:

  0b tttt s ccc

  Where:
  c is the timer channel (see above defines)
  t is (1 << (TOCCnumber & 0x03)) - that is 0b0001, 0b0010, 0b0100, or 0b1000
  s is 0 if TOCCnumber < 4, otherwise 1
  The left/right shift 4 positions is only 2 clocks - 1 for a swp instruction
  1 for andi 0xF0/0x0F (depending on direction of shift).

  So timer number = TOCCn & 0x07, and
  TOCC mask = TOCCn & 0xF0 >> (TOCCn & 0x08)?4:0
*/
#if defined(TOCPMCOE)
  #define TOCC0  (0x10)
  #define TOCC1  (0x20)
  #define TOCC2  (0x40)
  #define TOCC3  (0x80)
  #define TOCC4  (0x18)
  #define TOCC5  (0x28)
  #define TOCC6  (0x48)
  #define TOCC7  (0x88)
#endif

/*  Timer1 on the ATtiny167/ATtiny87 can output an
  identical waveform on one or more of 4 pins per
  channel. Implementing this was faster than making
  an actual "decision" */
#if defined(OC1AX)
  #define TIM1AU (0x18 | TIMER1A)
  #define TIM1AV (0x28 | TIMER1A)
  #define TIM1AW (0x48 | TIMER1A)
  #define TIM1AX (0x88 | TIMER1A)
  #define TIM1BU (0x14 | TIMER1B)
  #define TIM1BV (0x24 | TIMER1B)
  #define TIM1BW (0x44 | TIMER1B)
  #define TIM1BX (0x84 | TIMER1B)
#endif
#include "pins_arduino.h"

// 99% of parts have the whole USI on one port. 1634 doesn't! Here, if pins_arduino.h has
// defined a separate clock DDR, we leave it, otherwise point these defines at same port as
// the data lines. We also use the pin definitions to generate SPI and TWI pin mappings.

#ifndef USI_CLOCK_DDR
  #define USI_CLOCK_DDR   USI_DDR
  #define USI_CLOCK_PORT  USI_PORT
  #define USI_CLOCK_PIN   USI_PIN
#endif
#if defined(USI_PUE) && !defined(USI_CLOCK_PUE)
  #define USI_CLOCK_PUE   USI_PUE
#endif

// If these are not defined in pins_arduino, and we have a USI, then that's what is used for these interfaces.

#if !defined(SCK) && defined(USIDR)
  #define MOSI  USI_DO
  #define MISO  USI_DI
  #define SCK   USI_SCK
#endif

#if !defined(SCL) && defined(USIDR)
  #define SDA   USI_DI
  #define SCL   USI_SCK
#endif


/*=============================================================================
 * From what we got in pins_arduino.h as well as defines passed to the compiler from board submenu
 * options, determine which features to enable.
=============================================================================*/


#ifndef USE_SOFTWARE_SERIAL
  // Don't use builtin software serial unless the variant asked for it because there wasn't a hardware one.
  #define USE_SOFTWARE_SERIAL                       0
#endif

/* As of ATTinyCore 2.0.0, we have abandoned the pretense of the variant being
 * able to choose whichever timer it wants for millis. I'm not sure how far back
 * one would need to go to find a version of ATTinyCore where that worked
 * but I am damned sure it doesn't work correctly now. So we will no longer
 * include a TIMER_TO_USE_FOR_MILLIS option that might as well be named
 * SET_THIS_TO_0_OR_NOTHING_WORKS.
 *---------------------------------------------------------------------------*/

#ifndef TIMER_TO_USE_FOR_MILLIS
  #define TIMER_TO_USE_FOR_MILLIS                   0
#endif

#if defined(DISABLE_MILLIS)
  #define MILLIS_USE_TIMERNONE
  // forwards compatibility
#endif

#if TIMER_TO_USE_FOR_MILLIS != 0
  #if !defined(DISABLE_MILLIS)
    #warning "Millis not disabled, and set to use a non-standard timer. This code has not been exercised since at least 2016!"
  #else
    #warning "Non-standard timer arrangement selected. This code has not been exercised since at least 2016!"
  #endif
#endif

#if NUM_ANALOG_INPUTS > 0
  #ifndef ADCSRA
    #error "STOP - NUM_ANALOG_INPUTS is not 0, but no ADC present on chip. This indicates a critical defect in ATTinyCore which should be reported promptly."
  #endif
  #define HAVE_ADC                                  1
  #ifndef INITIALIZE_ADC
    #ifndef DEFAULT_INITIALIZE_ADC
      #define INITIALIZE_ADC                        1
    #else
      #define INITIALIZE_ADC                        DEFAULT_INITIALIZE_ADC
    #endif
  #endif
#else
  #if defined(INITIALIZE_ADC)
    #if INITIALIZE_ADC != 0
      #error "STOP - Variant requested that the ADC be initialized, but it also said that the chip didn't have an ADC (NUM_ANALOG_INPUTS == 0). This indicates a critical defect in ATTinyCore which should be reported promptly"
    #endif
    #undef INITIALIZE_ADC
  #endif
  #define INITIALIZE_ADC                            0
#endif

/*=============================================================================
  Allow the "secondary timers" to be optional for low-power applications
=============================================================================*/

#ifndef INITIALIZE_SECONDARY_TIMERS
  #ifdef DEFAULT_INITIALIZE_SECONDARY_TIMERS
    #define INITIALIZE_SECONDARY_TIMERS             DEFAULT_INITIALIZE_SECONDARY_TIMERS
  #else
    #define INITIALIZE_SECONDARY_TIMERS             1
  #endif
#endif

#ifndef TIMER_TO_USE_FOR_TONE
  #if TIMER_TO_USE_FOR_MILLIS == 0
    #define TIMER_TO_USE_FOR_TONE                   1
  #else
    #define TIMER_TO_USE_FOR_TONE                   0
  #endif
#else
  #if TIMER_TO_USE_FOR_TONE == TIMER_TO_USE_FOR_MILLIS && !defined(DISABLE_MILLIS)
    #error "Tone and millis are set to use the same timer. This is unsupported. If you did not modify the core files, this is defect in ATTinyCore and should be reported promptly."
  #endif
#endif

/* check_*_pin() truth table:
 *
 * pin       | pin constant? | return value | check_constant_pin() |
 * ----------|---------------|--------------|----------------------|
 * anything  | No            | True         | badArg()             |
 * NOT_A_PIN | Yes           | False        | no error             |
 * valid pin | Yes           | True         | no error             |
 * other     | Yes           | badArg()     | no error             |
 *
 * Fast Digital I/O requires both to pass, and for check_valid_digital_pin() to return true.
 *
 * pinModeFast(pin, INPUT, OUTPUT, or INPUT_PULLUP)
 * openDrainFast(pin,FLOATING or LOW)
 * digitalReadFast(pin)
 * digitalWriteFast(pin, value)
 * digitalWriteFaster(pin, value)
 *
 * The fast digital I/O functions otherwise behave like their conventional brethren, but write the pin in the most optimal way possible
 * typically via SBI or CBI, or SBIS or SBIC for digitalRead(), rather than using the lookup tables every time. The cost of this is in
 * flexibility - they must used with a constant pin number. In the case of the two "hard" ones, (pinMode and openDrain) the mode must
 * also be constant.
 * These also make the assumption that a pin is not currently being used to output PWM, as that would add a considerable amount of code
 * mostly to handle the case of turning off PWM. Note that the overhead to turnOffPWM is much smaller on the 828 and x41.
 *
 * turnOffPWM(timer) - does what it says on the package. Called by digitalWrite(). Takes a TIMER as argument, not a pin!
 *      NOT called by digitalRead in a departure from the stock cores, because "read" doesn't mean "change it's state and then read"
 * openDrain(pin, LOW or FLOATING) - LOW sets the pin OUTPUT and LOW. FLOATING sets the pin INPUT and LOW
 *
 *
 */

inline __attribute__((always_inline)) void check_constant_pin(uint8_t pin)
{
  if(!__builtin_constant_p(pin))
    badArg("Fast digital pin must be a constant");
}

inline __attribute__((always_inline)) uint8_t check_valid_digital_pin(uint8_t pin) {
  if(__builtin_constant_p(pin)) {
    if (pin >= NUM_DIGITAL_PINS && pin != NOT_A_PIN)
    // Exception made for NOT_A_PIN - code exists which relies on being able to pass this and have nothing happen.
    // While IMO very poor coding practice, these checks aren't here to prevent lazy programmers from intentionally
    // taking shortcuts we disapprove of, but to call out things that are virtually guaranteed to be a bug.
    // Passing -1/255/NOT_A_PIN to the digital I/O functions is at least as likely as not to be fully intended and required for the desired behavior,
    // not least because a few very popular libraries, including SoftwareSerial (it is not our policy to try to fix the implementation of that library.
    // Largely because it's far beyond redemption.)
      badArg("Digital pin is constant, and neither NOT_A_PIN nor a valid pin");
    return pin != NOT_A_PIN;
  }
  return 1;
}
inline __attribute__((always_inline)) void pinModeFast(uint8_t pin, uint8_t mode) {
  check_constant_pin(pin);
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    badArg("Fast I/O functions must be called with a valid pin number. NOT_A_PIN is not a valid pin number for these purposes.");
  }
  if (!__builtin_constant_p(mode)) {
    badArg("pinModeFast requires the mode to be compile time known");
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  if (port == NOT_A_PIN) return;
  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);
  #if defined(PUEA)
    out = portPullupRegister(port);
  #else
    out = portOutputRegister(port);
  #endif
  if (mode == INPUT) {
    uint8_t oldSREG = SREG;
    cli();
    *reg &= ~mask;
    *out &= ~mask;
    SREG = oldSREG;
  } else if (mode == INPUT_PULLUP) {
    uint8_t oldSREG = SREG;
    cli();
    *reg &= ~mask;
    *out |= mask;
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
    cli();
    *reg |= mask;
    SREG = oldSREG;
  }
}
inline __attribute__((always_inline)) void openDrainFast(uint8_t pin, uint8_t mode) {
  check_constant_pin(pin);
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    badArg("Fast I/O functions must be called with a valid pin number. NOT_A_PIN is not a valid pin number for these purposes.");
  }
  if (!__builtin_constant_p(mode)) {
    badArg("openDrainFast requires the mode to be compile time known");
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *ddr, *out;
  ddr = portModeRegister(port);
  out = portOutputRegister(port);
  if (mode == FLOATING) {
    *out &= ~mask;
    *ddr &= ~mask;
  } else if (mode == LOW) {
    *out &= ~mask;
    *ddr |= mask;
  }
}
inline __attribute__((always_inline)) int8_t digitalReadFast(uint8_t pin) {
  check_constant_pin(pin);
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    badArg("Fast I/O functions must be called with a valid pin number. NOT_A_PIN is not a valid pin number for these purposes.");
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  //if (port == NOT_A_PORT) return NOT_A_PIN;  // This check is not needed, as we reject non-constant pins but constant pins that are never valid get rejected by check valid digital pin!
  return !!(*portInputRegister(port) & mask);
}


inline __attribute__((always_inline)) void digitalWriteFast(uint8_t pin, uint8_t val) {
  check_constant_pin(pin);
  if (pin > 127) {
    pin = analogInputToDigitalPin((pin & 127));
  }
  if (!check_valid_digital_pin(pin)) {
    badArg("Fast I/O functions must be called with a valid pin number. NOT_A_PIN is not a valid pin number for these purposes.");
  }
  uint8_t mask = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;
  out = portOutputRegister(port);
  #if defined(__AVR_ATtinyX41__)      //Uniquely bad
    volatile uint8_t *pue, *mode;     //
    pue = portPullupRegister(port);   //
    mode = out + 1;
    if (val == LOW) {                 // 2 instructions

      if (*mode & mask) {
        *out &= ~mask;                // 1/1 atomic
      } else {                        // 1/1 +1
        uint8_t oldSREG = SREG;       // 1/1
        cli();                        // 1/1
        *pue &= ~mask;                // 3/5 in 5
        SREG = oldSREG;               // 1/1
      }
    } else {                          // total subtotal 8/10 in 5/9
      if (*mode & mask) {             // 1/1
        *out &= ~mask;                // 1/1
      } else {                        // 1/1 in 2
        uint8_t oldSREG = SREG;       // 1/1
        cli();                        // 1/1
        *pue &= ~mask;                // 3/5 in 5
        SREG = oldSREG;               // 1/1
      }                               // 17 instruction, 21 word, and and execution time of 8-12 clocks?
                                      // A whole new definition of fast (now it means "slow") - who wants to inline that!?
    }
  #elif defined(PUEA)
    volatile uint8_t *pue;
    pue = portPullupRegister(port);   //

    if (val == LOW) {
      *pue &= ~mask;                  // 1/1
      *out &= ~mask;                  // 1/1
    } else {                          // 1/1
      *pue &= ~mask;                  // 1/1
      *out &= ~mask;                  // 1/1
    }
      // constant pin -> constant out register.
      // constant val -> constant mask. Combined with above means we will get CBI/SBI (only one bit at a time will be set in the mask
  #else                               // total 13 and 13 and execution time of ~8 eitherway
    if (val == LOW) {
      *out &= ~mask;                  // 1/1
    } else {                          // 1/1
      *out &= ~mask;                  // 1/1
    }   // and we know which one at compile time, so
  #endif
}


#if defined(__AVR_ATtinyX41__)
  inline __attribute__((always_inline)) void digitalWriteFaster(uint8_t pin, uint8_t val) {
    // "Pullup enable? Never heard if it!" option for x41 only, because access to the PUEx registers is slow and non-atomic on those parts, and it would not be surprising
    // if someone wanted to avoid using them when doing straight digtial I/O. On all other parts - including other ones with PUEx registers (1634 and 828, which do have the
    // registers at sane addresses) - it is a normal call to digitalWriteFast(), and does manipulate the pullup state. Only on the x41 where the overhead is significant does
    // this have distinct functionality. Crucially, on the x41, you
    check_constant_pin(pin);
    if (pin > 127) {
      pin = analogInputToDigitalPin((pin & 127));
    }
    if (!check_valid_digital_pin(pin)) {
      badArg("Fast I/O functions must be called with a valid pin number. NOT_A_PIN is not a valid pin number for these purposes.");
    }
    uint8_t mask = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    volatile uint8_t *out;
    out = portOutputRegister(port);
    if (val == LOW) {                  // 2 instruction
      *out &= ~mask;                   // 1/1 atomic
    } else {                           //
      *out |= mask;                    // 1/1
    }
  }
#else // silently fall back to standard implementation.
  inline __attribute__((always_inline)) void digitalWriteFaster(uint8_t pin, uint8_t val) {
    digitalWriteFast(pin,val);
  }
#endif

#ifdef __cplusplus
  } // extern "C"
#endif

#ifdef __cplusplus
#include "WCharacter.h"
#include "WString.h"
#include "HardwareSerial.h"
#include "TinySoftwareSerial.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

  #define word(...) makeWord(__VA_ARGS__)
  unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
  unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);


void tone(uint8_t _pin, unsigned long frequency, unsigned long duration = 0);
void noTone(uint8_t _pin = 255);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long map(long, long, long, long, long);

#endif

/* SIGRD is missing from some of the IO headers, where it should
 * be defined as 5 for most parts. It is sometimes omitted entirely
 * and sometimes named RSIG. It is not present at all for some parts
 * and it's unknown at this time whether that feature didn't exist
 * and there was literally no way for the part to read it's own signature
 * or if they just couldn't be bothered to document it. Because it is often
 * missing from the headers for parts that do have it, we define it everywhere.

These call it SIGRD:
ATtiny167, ATtiny87

These call it RSIG:
ATtiny1634, ATtiny24*, ATtiny25, ATtiny4313, ATtiny44*, ATtiny441, ATtiny45, ATtiny828, ATtiny84*, ATtiny841, ATtiny85

* datasheet doesn't mention it, and in fact lists just 5 valid combinations of of SPMCSR bits, that will have any effect
...

These don't list it, but the datasheet says they support it:
ATtiny43U, ATtiny24A, ATtiny44A, ATtiny84A

These don't list it in either place:
ATtiny48, ATtiny88, All x61 family

It really makes one wonder about the documentation folks (and this section is all copypasta). There's only one way to get the answer:

#define SECTION_FUSE_LOCK 0x09
#define SECTION_SIGROW 0x21

uint8_t readSpecialSection(uint16_t addr, uint8_t section) { /
  if((section == 0x21 || section == 0x09))
  uint8_t result = (1 << section) | 1;
  __asm__ __volatile__ (
    "sts %[spmcsr], %[result]"  "\n\t"
    "lpm %[result], Z"          "\n\t"
  : [result] "+r" (result)
  : [spmcsr] "i" (_SFR_MEM_ADDR(__SPM_REG)),
    "z" (addr)
  );
  return result;
}

void setup() {
  Serial.begin(9600);
  for (uint16 i; i < 16; i++) {
    uint8_t result = readSpecialSection(i, SECTION_FUSE_LOCK)
    Serial.printHex(result);
  }
  for (uint16 i; i < 16; i++) {
    uint8_t result = readSpecialSection(i, SECTION_FUSE_LOCK)
    Serial.printHex(result);
  }
}
void loop() {
  ;
}


 * so it's unfortunate that they did named it so inconsistently.   */
#ifndef SIGRD
  #ifndef RSIG // Nieither defined, so we define for both.
    #define SIGRD 5
    #define RSIG 5
  #else
    #define SIGRD RSIG //RSIG defined but SIGRD not -> alias SIGRD to RSIG
  #endif
#else
  #ifndef RSIG
    #define RSIG SIGRD // other way around.
  #endif
#endif

// Thcase where both SIGRD and RSIG is defined is unlikely to exist in the wild.

/*---------------------------------------------------------------------------
 * Aliases for the interrupt service routine vector numbers so the code
 * (core and app) doesn't have to be riddled with quite as many ifdefs
 * Why couldn't they have picked one naming convention and stuck to it?
 * TIMn or TIMERn - they're both perfectly fine naming conventions...
 * And it would also have been okay if the decision of which to use was based
 * on whether it was a normal timer or a funky one (like the high speed timers)
 * But as far as I can tell, it was based on release date or coin flip.
 * For minimum pain to users, we go both directions.
 *---------------------------------------------------------------------------*/

// On the 26, there's an extra number on the vectors, get rid of it.
#if defined(TIMER1_OVF1_vect)
  #define TIMER1_OVF_vect_num TIMER1_OVF1_vect_num
  #define TIMER1_OVF_vect TIMER1_OVF1_vect
  #define TIMER0_OVF_vect_num TIMER0_OVF0_vect_num
  #define TIMER0_OVF_vect TIMER0_OVF0_vect
#endif

#if defined(TIM0_CAPT_vect)   && !defined(TIMER0_CAPT_vect)
  #define TIMER0_CAPT_vect TIM0_CAPT_vect
#endif
#if defined(TIM0_COMPA_vect)  && !defined(TIMER0_COMPA_vect)
  #define TIMER0_COMPA_vect TIM0_COMPA_vect
#endif
#if defined(TIM0_COMPB_vect)  && !defined(TIMER0_COMPB_vect)
  #define TIMER0_COMPB_vect TIM0_COMPB_vect
#endif
#if defined(TIM0_OVF_vect)    && !defined(TIMER0_OVF_vect)
  #define TIMER0_OVF_vect TIM0_OVF_vect
#endif
#if defined(TIM1_CAPT_vect)   && !defined(TIMER1_CAPT_vect)
  #define TIMER1_CAPT_vect TIM1_CAPT_vect
#endif
#if defined(TIM1_COMPA_vect)    && !defined(TIMER1_COMPA_vect)
  #define TIMER1_COMPA_vect TIM1_COMPA_vect
#endif
#if defined(TIM1_COMPB_vect)    && !defined(TIMER1_COMPB_vect)
  #define TIMER1_COMPB_vect TIM1_COMPB_vect
#endif
#if defined(TIM1_OVF_vect)      && !defined(TIMER1_OVF_vect)
  #define TIMER1_OVF_vect TIM1_OVF_vect
#endif
#if defined(TIM2_CAPT_vect)     && !defined(TIMER2_CAPT_vect)
  #define TIMER2_CAPT_vect TIM2_CAPT_vect
#endif
#if defined(TIM2_COMPA_vect)    && !defined(TIMER2_COMPA_vect)
  #define TIMER2_COMPA_vect TIM2_COMPA_vect
#endif
#if defined(TIM2_COMPB_vect)    && !defined(TIMER2_COMPB_vect)
  #define TIMER2_COMPB_vect TIM2_COMPB_vect
#endif
#if defined(TIM2_OVF_vect)      && !defined(TIMER2_OVF_vect)
  #define TIMER2_OVF_vect TIM2_OVF_vect
#endif

// Now the other direction...
#if defined(TIMER0_CAPT_vect)   && !defined(TIM0_CAPT_vect)
  #define TIM0_CAPT_vect TIMER0_CAPT_vect
#endif
#if defined(TIMER0_COMPA_vect)  && !defined(TIM0_COMPA_vect)
  #define TIM0_COMPA_vect TIMER0_COMPA_vect
#endif
#if defined(TIMER0_COMPB_vect)  && !defined(TIM0_COMPB_vect)
  #define TIM0_COMPB_vect TIMER0_COMPB_vect
#endif
#if defined(TIMER0_OVF_vect)    && !defined(TIM0_OVF_vect)
  #define TIM0_OVF_vect TIMER0_OVF_vect
#endif
#if defined(TIMER1_CAPT_vect)   && !defined(TIM1_CAPT_vect)
  #define TIM1_CAPT_vect TIMER1_CAPT_vect
#endif
#if defined(TIMER1_COMPA_vect)  && !defined(TIM1_COMPA_vect)
  #define TIM1_COMPA_vect TIMER1_COMPA_vect
#endif
#if defined(TIMER1_COMPB_vect)  && !defined(TIM1_COMPB_vect)
  #define TIM1_COMPB_vect TIMER1_COMPB_vect
#endif
#if defined(TIMER1_OVF_vect)    && !defined(TIM1_OVF_vect)
  #define TIM1_OVF_vect TIMER1_OVF_vect
#endif
#if defined(TIMER2_CAPT_vect)   && !defined(TIM2_CAPT_vect)
  #define TIM2_CAPT_vect TIMER2_CAPT_vect
#endif
#if defined(TIMER2_COMPA_vect)  && !defined(TIM2_COMPA_vect)
  #define TIM2_COMPA_vect TIMER2_COMPA_vect
#endif
#if defined(TIMER2_COMPB_vect)  && !defined(TIM2_COMPB_vect)
  #define TIM2_COMPB_vect TIMER2_COMPB_vect
#endif
#if defined(TIMER2_OVF_vect)    && !defined(TIM2_OVF_vect)
  #define TIM2_OVF_vect TIMER2_OVF_vect
#endif

#endif
