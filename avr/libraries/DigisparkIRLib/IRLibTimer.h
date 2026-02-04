/* IRLibTimer.h from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 *
 * This library is a major rewrite of IRemote by Ken Shirriff which was covered by
 * GNU LESSER GENERAL PUBLIC LICENSE which as I read it allows me to make modified versions.
 * That same license applies to this modified version. See his original copyright below.
 * The latest Ken Shirriff code can be found at https://github.com/shirriff/Arduino-IRremote
 * My purpose was to reorganize the code to make it easier to add or remove protocols.
 * As a result I have separated the act of receiving a set of raw timing codes from the act of decoding them
 * by making them separate classes. That way the receiving aspect can be more black box and implementers
 * of decoders and senders can just deal with the decoding of protocols.
 * Also added provisions to make the classes base classes that could be extended with new protocols
 * which would not require recompiling of the original library nor understanding of its detailed contents.
 * Some of the changes were made to reduce code size such as unnecessary use of long versus bool.
 * Some changes were just my weird programming style. Also extended debugging information added.
 */
/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */
/* This file defines which timer you wish to use. Different versions of Arduino and related boards
  * have different timers available to them. For various reasons you might want to use something other than
  * timer 2. Some boards do not have timer 2. This attempts to detect which type of board you are using.
  * You need uncomment wish to use on your board. You probably will not need to include this in your
  * program unless you want to see which timer is being used and which board has been detected.
  * This information came from an alternative fork of the original Ken Shirriff library found here
  * https://github.com/TKJElectronics/Arduino-IRremote
  */
#ifndef IRLibTimer_h
#define IRLibTimer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif

// Arduino Mega
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  //#define IR_USE_TIMER1   // tx = pin 11
  #define IR_USE_TIMER2     // tx = pin 9
  //#define IR_USE_TIMER3   // tx = pin 5
  //#define IR_USE_TIMER4   // tx = pin 6
  //#define IR_USE_TIMER5   // tx = pin 46

// Teensy 1.0
#elif defined(__AVR_AT90USB162__)
  #define IR_USE_TIMER1     // tx = pin 17

// Teensy 2.0 versus Leonardo
// These boards use the same chip but the pinouts are different.
#elif defined(__AVR_ATmega32U4__)
#ifdef CORE_TEENSY
  // it's Teensy 2.0
  //#define IR_USE_TIMER1   // tx = pin 14
  //#define IR_USE_TIMER3   // tx = pin 9
  #define IR_USE_TIMER4_HS  // tx = pin 10
#else
  // it's probably Leonardo
  #define IR_USE_TIMER1   // tx = pin 9
  //#define IR_USE_TIMER3   // tx = pin 5
  //#define IR_USE_TIMER4_HS  // tx = pin 13
#endif

// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
  //#define IR_USE_TIMER1   // tx = pin 25
  #define IR_USE_TIMER2     // tx = pin 1
  //#define IR_USE_TIMER3   // tx = pin 16

// Sanguino
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
  //#define IR_USE_TIMER1   // tx = pin 13
  #define IR_USE_TIMER2     // tx = pin 14

// Tested with ATtiny85, presumably works with ATtiny45, possibly with ATtiny25
// The attiny core uses Timer 0 for millis() etc., so using timer 1 is advisable
// for IR out. Pin 4 also conveniently is not used in any role for ISP.
// The Arduino-tiny core uses Timer 1 for millis(), so using timer 0 is advisable
// for IR out. Pin 0 is used by default for IR out in the Tinyspark IR shield, but 
// is not usable for PWM waveforms where the frequency, not just the duty cycle,
// needs to be controlled.
#elif defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
// #define IR_USE_TIMER1_TINY	// tx = pin 4 (OC1B)
  #define IR_USE_TIMER0			// tx = pin 1 (OC0B)
// Arduino Duemilanove, Diecimila, LilyPad, Mini, Fio, etc
#else
  //#define IR_USE_TIMER1   // tx = pin 9
  #define IR_USE_TIMER2     // tx = pin 3
#endif


// defines for timer0 (8 bits). Tested on ATtiny85, may also work on other 
// processors, but most of them use Timer 0 to keep system time
#if defined(IR_USE_TIMER0)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR0A |= _BV(COM0B1))
#define TIMER_DISABLE_PWM    (TCCR0A &= ~(_BV(COM0B1)))
#define TIMER_ENABLE_INTR    (TIMSK = _BV(OCIE0A))
#define TIMER_DISABLE_INTR   (TIMSK = 0)
#define TIMER_INTR_NAME      TIMER0_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR0A = _BV(WGM00); \
  TCCR0B = _BV(WGM02) | _BV(CS00); \
  OCR0A = pwmval; \
  OCR0B = pwmval / 3; \
})
#define TIMER_COUNT_TOP      (SYSCLOCK * USECPERTICK / 1000000)
#if (TIMER_COUNT_TOP < 256)
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR0A = _BV(WGM01); \
  TCCR0B = _BV(CS00); \
  OCR0A = TIMER_COUNT_TOP; \
  TCNT0 = 0; \
})
#else
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR0A = _BV(WGM01); \
  TCCR0B = _BV(CS01); \
  OCR0A = TIMER_COUNT_TOP / 8; \
  TCNT0 = 0; \
})
#endif
#if defined(CORE_OC0A_PIN)
#define TIMER_PWM_PIN        CORE_OC0B_PIN 
#else
#define TIMER_PWM_PIN        1  /* Attiny core */
#endif

// defines for timer2 (8 bits)
#elif defined(IR_USE_TIMER2)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR2A |= _BV(COM2B1))
#define TIMER_DISABLE_PWM    (TCCR2A &= ~(_BV(COM2B1)))
#define TIMER_ENABLE_INTR    (TIMSK2 = _BV(OCIE2A))
#define TIMER_DISABLE_INTR   (TIMSK2 = 0)
#define TIMER_INTR_NAME      TIMER2_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR2A = _BV(WGM20); \
  TCCR2B = _BV(WGM22) | _BV(CS20); \
  OCR2A = pwmval; \
  OCR2B = pwmval / 3; \
})
#define TIMER_COUNT_TOP      (SYSCLOCK * USECPERTICK / 1000000)
#if (TIMER_COUNT_TOP < 256)
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR2A = _BV(WGM21); \
  TCCR2B = _BV(CS20); \
  OCR2A = TIMER_COUNT_TOP; \
  TCNT2 = 0; \
})
#else
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR2A = _BV(WGM21); \
  TCCR2B = _BV(CS21); \
  OCR2A = TIMER_COUNT_TOP / 8; \
  TCNT2 = 0; \
})
#endif
#if defined(CORE_OC2B_PIN)
#define TIMER_PWM_PIN        CORE_OC2B_PIN  /* Teensy */
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define TIMER_PWM_PIN        9  /* Arduino Mega */
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#define TIMER_PWM_PIN        14 /* Sanguino */
#else
#define TIMER_PWM_PIN        3  /* Arduino Duemilanove, Diecimila, LilyPad, etc */
#endif


// defines for timer1 (16 bits)
#elif defined(IR_USE_TIMER1)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR1A |= _BV(COM1A1))
#define TIMER_DISABLE_PWM    (TCCR1A &= ~(_BV(COM1A1)))
#define TIMER_ENABLE_INTR    (TIMSK1 = _BV(OCIE1A))
#define TIMER_DISABLE_INTR   (TIMSK1 = 0)
#define TIMER_INTR_NAME      TIMER1_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR1A = _BV(WGM11); \
  TCCR1B = _BV(WGM13) | _BV(CS10); \
  ICR1 = pwmval; \
  OCR1A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR1A = 0; \
  TCCR1B = _BV(WGM12) | _BV(CS10); \
  OCR1A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT1 = 0; \
})
#if defined(CORE_OC1A_PIN)
#define TIMER_PWM_PIN        CORE_OC1A_PIN  /* Teensy */
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define TIMER_PWM_PIN        11  /* Arduino Mega */
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#define TIMER_PWM_PIN        13 /* Sanguino */
#else
#define TIMER_PWM_PIN        9  /* Arduino Duemilanove, Diecimila, LilyPad, etc */
#endif


// defines for timer3 (16 bits)
#elif defined(IR_USE_TIMER3)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR3A |= _BV(COM3A1))
#define TIMER_DISABLE_PWM    (TCCR3A &= ~(_BV(COM3A1)))
#define TIMER_ENABLE_INTR    (TIMSK3 = _BV(OCIE3A))
#define TIMER_DISABLE_INTR   (TIMSK3 = 0)
#define TIMER_INTR_NAME      TIMER3_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR3A = _BV(WGM31); \
  TCCR3B = _BV(WGM33) | _BV(CS30); \
  ICR3 = pwmval; \
  OCR3A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR3A = 0; \
  TCCR3B = _BV(WGM32) | _BV(CS30); \
  OCR3A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT3 = 0; \
})
#if defined(CORE_OC3A_PIN)
#define TIMER_PWM_PIN        CORE_OC3A_PIN  /* Teensy */
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define TIMER_PWM_PIN        5  /* Arduino Mega */
#elif defined(__AVR_ATmega32U4__)
#define TIMER_PWM_PIN        5  /* Arduino Leonardo note already checked for Teensy */
#else
#error "Please add OC3A pin number here\n"
#endif


// defines for timer4 (10 bits, high speed option)
#elif defined(IR_USE_TIMER4_HS)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM    (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR    (TIMSK4 = _BV(TOIE4))
#define TIMER_DISABLE_INTR   (TIMSK4 = 0)
#define TIMER_INTR_NAME      TIMER4_OVF_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR4A = (1<<PWM4A); \
  TCCR4B = _BV(CS40); \
  TCCR4C = 0; \
  TCCR4D = (1<<WGM40); \
  TCCR4E = 0; \
  TC4H = pwmval >> 8; \
  OCR4C = pwmval; \
  TC4H = (pwmval / 3) >> 8; \
  OCR4A = (pwmval / 3) & 255; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR4A = 0; \
  TCCR4B = _BV(CS40); \
  TCCR4C = 0; \
  TCCR4D = 0; \
  TCCR4E = 0; \
  TC4H = (SYSCLOCK * USECPERTICK / 1000000) >> 8; \
  OCR4C = (SYSCLOCK * USECPERTICK / 1000000) & 255; \
  TC4H = 0; \
  TCNT4 = 0; \
})
#if defined(CORE_OC4A_PIN)
#define TIMER_PWM_PIN        CORE_OC4A_PIN  /* Teensy */
#elif defined(__AVR_ATmega32U4__)
#define TIMER_PWM_PIN 13 /*Leonardo*/
#else
#error "Please add OC4A pin number here\n"
#endif


// defines for timer4 (16 bits)
#elif defined(IR_USE_TIMER4)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR4A |= _BV(COM4A1))
#define TIMER_DISABLE_PWM    (TCCR4A &= ~(_BV(COM4A1)))
#define TIMER_ENABLE_INTR    (TIMSK4 = _BV(OCIE4A))
#define TIMER_DISABLE_INTR   (TIMSK4 = 0)
#define TIMER_INTR_NAME      TIMER4_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR4A = _BV(WGM41); \
  TCCR4B = _BV(WGM43) | _BV(CS40); \
  ICR4 = pwmval; \
  OCR4A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR4A = 0; \
  TCCR4B = _BV(WGM42) | _BV(CS40); \
  OCR4A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT4 = 0; \
})
#if defined(CORE_OC4A_PIN)
#define TIMER_PWM_PIN        CORE_OC4A_PIN
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define TIMER_PWM_PIN        6  /* Arduino Mega */
#else
#error "Please add OC4A pin number here\n"
#endif


// defines for timer5 (16 bits)
#elif defined(IR_USE_TIMER5)
#define TIMER_RESET
#define TIMER_ENABLE_PWM     (TCCR5A |= _BV(COM5A1))
#define TIMER_DISABLE_PWM    (TCCR5A &= ~(_BV(COM5A1)))
#define TIMER_ENABLE_INTR    (TIMSK5 = _BV(OCIE5A))
#define TIMER_DISABLE_INTR   (TIMSK5 = 0)
#define TIMER_INTR_NAME      TIMER5_COMPA_vect
#define TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR5A = _BV(WGM51); \
  TCCR5B = _BV(WGM53) | _BV(CS50); \
  ICR5 = pwmval; \
  OCR5A = pwmval / 3; \
})
#define TIMER_CONFIG_NORMAL() ({ \
  TCCR5A = 0; \
  TCCR5B = _BV(WGM52) | _BV(CS50); \
  OCR5A = SYSCLOCK * USECPERTICK / 1000000; \
  TCNT5 = 0; \
})
#if defined(CORE_OC5A_PIN)
#define TIMER_PWM_PIN        CORE_OC5A_PIN
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define TIMER_PWM_PIN        46  /* Arduino Mega */
#else
#error "Please add OC5A pin number here\n"
#endif


#else // unknown timer
#error "Internal code configuration error, no known IR_USE_TIMER# defined\n"
#endif


// defines for blinking the LED
#if defined(CORE_LED0_PIN)
#define BLINKLED       CORE_LED0_PIN
#define BLINKLED_ON()  (digitalWrite(CORE_LED0_PIN, HIGH))
#define BLINKLED_OFF() (digitalWrite(CORE_LED0_PIN, LOW))
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define BLINKLED       13
#define BLINKLED_ON()  (PORTB |= B10000000)
#define BLINKLED_OFF() (PORTB &= B01111111)
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)
#define BLINKLED       0
#define BLINKLED_ON()  (PORTD |= B00000001)
#define BLINKLED_OFF() (PORTD &= B11111110)
#elif defined(__AVR_ATmega32U4__) && defined(IR_USE_TIMER4_HS)
//Leonardo not teensy. When using Timer4 output is on 13. Therefore disabling blink LED
//You can add an LED elsewhere if you want
#define BLINKLED       1
#define BLINKLED_ON()  (digitalWrite(BLINKLED, HIGH))
#define BLINKLED_OFF() (digitalWrite(BLINKLED, LOW))
#else
#define BLINKLED       13
#define BLINKLED_ON()  (PORTB |= B00100000)
#define BLINKLED_OFF() (PORTB &= B11011111)
#endif

#endif //IRLibTimer_h
