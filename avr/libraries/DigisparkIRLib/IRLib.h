/* IRLib.h from IRLib  an Arduino library for infrared encoding and decoding
 * Version 1.1   April 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * 
 * Port to Digispark (size optimization) August 2013
 * by RC Navy http://p.loussouarn.free.fr
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

#ifndef IRLib_h
#define IRLib_h
#include <Arduino.h>

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// If TRACE is defined, some debugging information about the decode will be printed
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// If DETAILLED_DUMP is defined the dump informations are more detailled
// If ALL_IR_PROTOCOL is defined, it allows to discover the protocol used by the IR transmitter (eg, with an arduino UNO)
// Once the protocol used by the IR transmitter discovered, set MY_IR_PROTOCOL to PROTO_xxxx and comment ALL_IR_PROTOCOL
// this will reduce dramatically the size of the sketch
// #define DEBUG
// #define TRACE
// #define TEST

//#define USE_IR_SEND
//#define DETAILLED_DUMP
//#define ALL_IR_PROTOCOL
#define MY_IR_PROTOCOL		PROTO_NEC /* Set Here the protocol you want to use among the following ones */

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

#define RAWBUF 70//100 // Length of raw duration buffer
#define USE_TIMER1 1  //should be "1" for timer 1, should be "0" for timer 2

/* Use one of the protocol in the list below if you want to support a single one */
#define PROTO_UNKNOWN		0
#define PROTO_NEC		1
#define PROTO_SONY		2
#define PROTO_RC5		3
#define PROTO_RC6		4
#define PROTO_PANASONIC_OLD	5
#define PROTO_JVC		6
#define PROTO_NECX		7
#define PROTO_HASH_CODE		8

enum IRTYPES {UNKNOWN, NEC, SONY, RC5, RC6, PANASONIC_OLD, JVC, NECX, HASH_CODE, LAST_PROTOCOL=HASH_CODE};

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
const char *Pnames(IRTYPES Type); //Returns a character string that is name of protocol.
#else
const __FlashStringHelper *Pnames(IRTYPES Type); //Returns a character string that is name of protocol.
#endif
// Base class for decoding raw results
class IRdecodeBase
{
public:
  IRdecodeBase(void);
  IRTYPES decode_type;           // NEC, SONY, RC5, UNKNOWN etc.
  unsigned long value;           // Decoded value
  int bits;                      // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  /*int*/uint8_t rawlen;                    // Number of records in rawbuf.
  virtual void Reset(void);      // Initializes the decoder
  virtual bool decode(void);     // This base routine always returns false override with your routine
  bool decodeGeneric(/*int*/int8_t Raw_Count,int Head_Mark,int Head_Space, int Mark_One, int Mark_Zero, int Space_One,int Space_Zero);
  unsigned long Interval_uSec(int index);
  virtual void DumpResults (void);
  void UseExtnBuf(void *P); //Normally uses same rawbuf as IRrecv. Use this to define your own buffer.
  void copyBuf (IRdecodeBase *source);//copies rawbuf and rawlen from one decoder to another
};

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_HASH_CODE)
class IRdecodeHash: public virtual IRdecodeBase
{
public:
  unsigned long hash;
  virtual bool decode(void);//made virtual in case you want to substitute your own hash code
protected:
  int compare(unsigned int oldval, unsigned int newval);//used by decodeHash
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NEC)
class IRdecodeNEC: public virtual IRdecodeBase 
{
public:
  virtual bool decode(void);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_SONY)
class IRdecodeSony: public virtual IRdecodeBase 
{
public:
  virtual bool decode(void);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5) || (MY_IR_PROTOCOL == PROTO_RC6)
class IRdecodeRC: public virtual IRdecodeBase 
{
public:
  enum RCLevel {MARK, SPACE, ERROR};//used by decoders for RC5/RC6
  // These are called by decode
  RCLevel getRClevel(int *offset, int *used, int t1);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5)
class IRdecodeRC5: public virtual IRdecodeRC 
{
public:
  virtual bool decode(void);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC6)
class IRdecodeRC6: public virtual IRdecodeRC
{
public:
  virtual bool decode(void);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_PANASONIC_OLD)
class IRdecodePanasonic_Old: public virtual IRdecodeBase 
{
public:
  virtual bool decode(void);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_JVC)
class IRdecodeJVC: public virtual IRdecodeBase 
{
public:
  virtual bool decode(void);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NECX)
class IRdecodeNECx: public virtual IRdecodeBase 
{
public:
  virtual bool decode(void);
};
#endif

// main class for decoding all supported protocols
class IRdecode: 
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NEC)
public virtual IRdecodeNEC
#endif
#if defined(ALL_IR_PROTOCOL)
,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_SONY)
public virtual IRdecodeSony
#endif
#if defined(ALL_IR_PROTOCOL)
,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5)
public virtual IRdecodeRC5
#endif
#if defined(ALL_IR_PROTOCOL)
,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC6)
public virtual IRdecodeRC6
#endif
#if defined(ALL_IR_PROTOCOL)
,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_PANASONIC_OLD)
public virtual IRdecodePanasonic_Old
#endif
#if defined(ALL_IR_PROTOCOL)
,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_JVC)
public virtual IRdecodeJVC
#endif
#if defined(ALL_IR_PROTOCOL)
,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NECX)
public virtual IRdecodeNECx
#endif
{
public:
  virtual bool decode(void);    // Calls each decode routine individually
};

#ifdef USE_IR_SEND
//Base class for sending signals
class IRsendBase
{
public:
  IRsendBase() ;
  void sendGeneric(unsigned long data, int Num_Bits, int Head_Mark, int Head_Space, int Mark_One, int Mark_Zero, int Space_One, int Space_Zero, int mHz, bool Stop_Bits);
protected:
  void enableIROut(int khz);
  VIRTUAL void mark(int usec);
  VIRTUAL void space(int usec);
};

class IRsendNEC: public virtual IRsendBase
{
public:
  void send(unsigned long data);
};

class IRsendSony: public virtual IRsendBase
{
public:
  void send(unsigned long data, int nbits);
};

class IRsendRaw: public virtual IRsendBase
{
public:
  void send(unsigned int buf[], int len, int hz);
};

class IRsendRC5: public virtual IRsendBase
{
public:
  void send(unsigned long data);
};

class IRsendRC6: public virtual IRsendBase
{
public:
  void send(unsigned long data, int nbits);
};

class IRsendPanasonic_Old: public virtual IRsendBase
{
public:
  void send(unsigned long data);
};

class IRsendJVC: public virtual IRsendBase
{
public:
  void send(unsigned long data, bool First);
};

class IRsendNECx: public virtual IRsendBase
{
public:
  void send(unsigned long data);
};

class IRsend: 
public virtual IRsendNEC,
public virtual IRsendSony,
public virtual IRsendRaw,
public virtual IRsendRC5,
public virtual IRsendRC6,
public virtual IRsendPanasonic_Old,
public virtual IRsendJVC,
public virtual IRsendNECx
{
public:
  void send(IRTYPES Type, unsigned long data, int nbits);
};
#endif

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(int recvpin);
  void No_Output(void);
  void blink13(int blinkflag);
  bool GetResults(IRdecodeBase *decoder);
  void enableIRIn();
  void resume();
};

// Some useful constants
// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff


#endif //IRLib_h
