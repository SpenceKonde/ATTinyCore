/* IRLibMatch.h from IRLib – an Arduino library for infrared encoding and decoding
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

#ifndef IRLibMatch_h
#define IRLibMatch_h

/*
 * This is some miscellaneous definition that is needed by the decoding routines. 
 * You need not include this file unless you are creating custom decode routines 
 * which will require these macros and definitions. Even if you include it, you probably
 * don't need to be intimately familiar with the internal details.
 */

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100
#define USECPERTICK 50  // microseconds per clock interrupt tick
#if 0
#define TOLERANCE 25  // percent tolerance in measurements
#define TICKS_LOW(us) (int) (((us)*(1.0 - TOLERANCE/100.)/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*(1.0 + TOLERANCE/100.)/USECPERTICK + 1))
#else
#define TICKS_LOW(us)  (((us) - (us>>2))/USECPERTICK)
#define TICKS_HIGH(us) (((us) + (us>>2))/USECPERTICK)
#endif

#ifdef DEBUG
int MATCH(int measured, int desired); 
int MATCH_MARK(int measured_ticks, int desired_us);
int MATCH_SPACE(int measured_ticks, int desired_us);
#else
  #define MATCH(measured_ticks, desired_us) ((measured_ticks) >= TICKS_LOW(desired_us) && (measured_ticks) <= TICKS_HIGH(desired_us))
  #define MATCH_MARK(measured_ticks, desired_us) MATCH(measured_ticks, (desired_us) + MARK_EXCESS)
  #define MATCH_SPACE(measured_ticks, desired_us) MATCH((measured_ticks), (desired_us) - MARK_EXCESS)
#endif


#ifdef TRACE
void ATTEMPT_MESSAGE(const __FlashStringHelper * s);
byte REJECTION_MESSAGE(const __FlashStringHelper * s);
#define RAW_COUNT_ERROR REJECTION_MESSAGE(F("number of raw samples"));
#define HEADER_MARK_ERROR REJECTION_MESSAGE(F("header mark"));
#define HEADER_SPACE_ERROR REJECTION_MESSAGE(F("header space"));
#define DATA_MARK_ERROR REJECTION_MESSAGE(F("data mark"));
#define DATA_SPACE_ERROR REJECTION_MESSAGE(F("data space"));
#define TRAILER_BIT_ERROR REJECTION_MESSAGE(F("RC5/RC6 trailer bit length"));
#else
#define ATTEMPT_MESSAGE(s)
#define REJECTION_MESSAGE(s) false
#define RAW_COUNT_ERROR false
#define HEADER_MARK_ERROR false
#define HEADER_SPACE_ERROR false
#define DATA_MARK_ERROR false
#define DATA_SPACE_ERROR false
#define TRAILER_BIT_ERROR false
#endif

#endif //IRLibMatch_h
