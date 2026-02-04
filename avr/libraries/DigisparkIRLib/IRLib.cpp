/* IRLib.cpp from IRLib – an Arduino library for infrared encoding and decoding
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

#include "IRLib.h"
#include "IRLibMatch.h"
#include <Arduino.h>


/*
 * Returns a pointer to a flash stored string that is the name of the protocol received. 
 */
#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
const char *Pnames(IRTYPES Type) {
  return(NULL);
};
#else
const __FlashStringHelper *Pnames(IRTYPES Type) {
  if(Type>LAST_PROTOCOL) Type=UNKNOWN;
  const __FlashStringHelper *Names[LAST_PROTOCOL+1]={F("Unknown"),
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NEC)
  F("NEC")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_SONY)
  F("Sony")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5)
  F("RC5")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC6)
  F("RC6")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_PANASONIC_OLD)
  F("Panasonic Old")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_JVC)
  F("JVC")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NECX)
  F("NECx")
#endif
#if defined(ALL_IR_PROTOCOL)
  ,
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_HASH_CODE)
  F("Hash Code")
#endif
  };
#if defined(ALL_IR_PROTOCOL)
  return Names[Type];
#else
  return Names[!!Type];
#endif
};
#endif

#define RC5_T1		889
#define RC5_RPT_LENGTH	46000

#define RC6_HDR_MARK	2666
#define RC6_HDR_SPACE	889
#define RC6_T1		444

#ifdef USE_IR_SEND

#define TOPBIT 0x80000000

/*
 * The IRsend classes contain a series of methods for sending various protocols.
 * Each of these begin by calling enableIROut(int kHz) to set the carrier frequency.
 * It then calls mark(int usec) and space(inc usec) to transmit marks and
 * spaces of varying length of microseconds however the protocol defines.
 */

/*
 * Most of the protocols have a header consisting of a mark/space of a particular length followed by 
 * a series of variable length mark/space signals.  Depending on the protocol they very the links of the 
 * mark or the space to indicate a data bit of "0" or "1". Most also end with a stop bit of "1".
 * The basic structure of the sending and decoding these protocols led to lots of redundant code. 
 * Therefore I have implemented generic sending and decoding routines. You just need to pass a bunch of customized 
 * parameters and it does the work. This reduces compiled code size with only minor speed degradation. 
 * You may be able to implement additional protocols by simply passing the proper values to these generic routines.
 * The decoding routines do not encode stop bits. So you have to tell this routine whether or not to send one.
 */
 
void IRsendBase::sendGeneric(unsigned long data, int Num_Bits, int Head_Mark, int Head_Space, int Mark_One, int Mark_Zero, int Space_One, int Space_Zero, int mHz, bool Use_Stop) {
  data = data << (32 - Num_Bits);
  enableIROut(mHz);
//Some protocols do not send a header when sending repeat codes. So we pass a zero value to indicate skipping this.
  if(Head_Mark) mark(Head_Mark); 
  if(Head_Space) space(Head_Space);
  for (int i = 0; i <Num_Bits; i++) {
    if (data & TOPBIT) {
      mark(Mark_One);  space(Space_One);
    } 
    else {
      mark(Mark_Zero);  space(Space_Zero);
    }
    data <<= 1;
  }
  if (Use_Stop) mark(Mark_One);  space(0) ; //stop bit of "1"
};

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NEC)
void IRsendNEC::send(unsigned long data)
{
  ATTEMPT_MESSAGE(F("sending NEC"));
  if (data==REPEAT) {
    mark (564* 16); space (564*4); mark(564);space (96000); 
  }
  else {
    sendGeneric(data,32, 564*16, 564*8, 564, 564, 564*3, 564, 38, true);
  }
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_SONY)
/*
 * Sony is backwards from most protocols. It uses a variable length mark and a fixed length space rather than
 * a fixed mark and a variable space. Our generic send will still work. According to the protocol you must send
 * Sony commands at least three times so we automatically do it here.
 */
void IRsendSony::send(unsigned long data, int nbits) {
  for(int i=0; i<3;i++)
     sendGeneric(data,nbits, 600*4, 600, 600*2, 600, 600, 600, 40, false); 
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NECX)
/*
 * This next section of send routines were added by Chris Young. They all use the generic send.
 */
void IRsendNECx::send(unsigned long data)
{
  sendGeneric(data,32, 564*8, 564*8, 564, 564, 564*3, 564, 38, true);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_PANASONIC_OLD)
void IRsendPanasonic_Old::send(unsigned long data)
{
  sendGeneric(data,22, 833*4, 833*4, 833, 833, 833*3, 833,57, true);
};
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_JVC)
/*
 * JVC omits the Mark/space header on repeat sending. Therefore we multiply it by 0 if it's a repeat.
 * The only device I had to test this protocol was an old JVC VCR. It would only work if at least
 * 2 frames are sent separated by 45us of "space". Therefore you should call this routine once with
 * "First=true"and it will send a first frame followed by one repeat frame. If First== false,
 * it will only send a single repeat frame.
 */
void IRsendJVC::send(unsigned long data, bool First)
{
  sendGeneric(data, 16,525*16*First, 525*8*First, 525, 525,525*3, 525, 38, true);
  delayMicroseconds(45);
  if(First) sendGeneric(data, 16,0,0, 525, 525,525*3, 525, 38, true);
}
#endif

/*
 * The remaining protocols require special treatment. They were in the original IRremote library.
 */
void IRsendRaw::send(unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } 
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}
/*
 * The RC5 protocol uses a phase encoding of data bits. A space/mark pair indicates "1"
 * and a mark/space indicates a "0". It begins with a single "1" bit which is not encoded
 * in the data. The high order data bit is a toggle bit that indicates individual
 * keypresses. You must toggle this bit yourself when sending data.
 */

void IRsendRC5::send(unsigned long data)
{
  enableIROut(36);
  data = data << (32 - 13);
  mark(RC5_T1); // First start bit
//Note: Original IRremote library incorrectly assumed second bit was always a "1"
//bit patterns from this decoder are not backward compatible with patterns produced
//by original library. Ucomment the following two lines to maintain backward compatibility.
  //space(RC5_T1); // Second start bit
  //mark(RC5_T1); // Second start bit
  for (int i = 0; i < 13; i++) {
    if (data & TOPBIT) {
      space(RC5_T1); mark(RC5_T1);// 1 is space, then mark
    } 
    else {
      mark(RC5_T1);  space(RC5_T1);// 0 is mark, then space
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}

/*
 * The RC6 protocol also phase encodes databits although the phasing is opposite of RC5.
 */
void IRsendRC6::send(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC6_HDR_MARK); space(RC6_HDR_SPACE);
  mark(RC6_T1);  space(RC6_T1);// start bit "1"
  int t;
  for (int i = 0; i < nbits; i++) {
    if (i == 3) {
      t = 2 * RC6_T1;       // double-wide trailer bit
    } 
    else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      mark(t); space(t);//"1" is a Mark/space
    } 
    else {
      space(t); mark(t);//"0" is a space/Mark
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}
/*
 * This method can be used to send any of the supported types except for raw and hash code.
 * There is no hash code send possible. You can call sendRaw directly if necessary.
 */
void IRsend::send(IRTYPES Type, unsigned long data, int nbits) {
  switch(Type) {
    case NEC:           IRsendNEC::send(data); break;
    case SONY:          IRsendSony::send(data,nbits); break;
    case RC5:           IRsendRC5::send(data); break;
    case RC6:           IRsendRC6::send(data,nbits); break;
    case PANASONIC_OLD: IRsendPanasonic_Old::send(data); break;
    case NECX:          IRsendNECx::send(data); break;    
    case JVC:           IRsendJVC::send(data,(bool)nbits); break;
  }
}
#endif

/*
 * Although I tried to keep all of the interrupt handling code at the bottom of this file
 * so you didn't have to mess with it if you didn't want to, I needed to move this definition
 * forwarded so it could be used by IRdecodeBase constructor.
 */
// receiver states
enum rcvstate_t {STATE_UNKNOWN, STATE_IDLE, STATE_MARK, STATE_SPACE, STATE_STOP};
// information for the interrupt handler
typedef struct {
  uint8_t recvpin;           // pin for IR data from detector
  rcvstate_t rcvstate;       // state machine
  uint8_t blinkflag;         // TRUE to enable blinking of pin 13 on IR processing
  unsigned int timer;     // state timer, counts 50uS ticks.
  unsigned int rawbuf[RAWBUF]; // raw data
  uint8_t rawlen;         // counter of entries in rawbuf
} 
irparams_t;
volatile irparams_t irparams;

/*
 * We've chosen to separate the decoding routines from the receiving routines to isolate
 * the technical hardware and interrupt portion of the code which should never need modification
 * from the protocol decoding portion that will likely be extended and modified.
 */
IRdecodeBase::IRdecodeBase(void) {
  rawbuf=(volatile unsigned int*)irparams.rawbuf;
  Reset();
};
/*
 * Normally the decoder uses irparams.rawbuf but if you want to resume receiving while
 * still decoding you can define a separate buffer and pass the address here. 
 * Then IRrecv::GetResults will copy the raw values from its buffer to yours allowing you to
 * call IRrecv::resume immediately before you call decode.
 */
void IRdecodeBase::UseExtnBuf(void *P){
  rawbuf=(volatile unsigned int*)P;
};
/*
 * Copies rawbuf and rawlen from one decoder to another. See IRhashdecode example
 * for usage.
 */
void IRdecodeBase::copyBuf (IRdecodeBase *source){
   memcpy((void *)rawbuf,(const void *)source->rawbuf,sizeof(irparams.rawbuf));
   rawlen=source->rawlen;
};

/*
 * This routine is actually quite useful. See the Samsung36 sketch in the examples
 */
bool IRdecodeBase::decode(void) {
  return false;
};

void IRdecodeBase::Reset(void) {
  decode_type= UNKNOWN;
  value=0;
  bits=0;
  rawlen=0;
};
/*
 * This method dumps useful information about the decoded values.
 */
void IRdecodeBase::DumpResults(void) {
  int i;
  if(decode_type<=LAST_PROTOCOL){
    Serial.print(F("Decoded ")); Serial.print(Pnames(decode_type));
    Serial.print(F(": Value:")); Serial.print(value, HEX);
  };
#ifdef DETAILLED_DUMP
  Serial.print(F(" ("));  Serial.print(bits, DEC); Serial.println(F(" bits)"));
  Serial.print(F("Raw samples(")); Serial.print(rawlen, DEC);
  Serial.print(F("): Gap:")); Serial.println(Interval_uSec(0), DEC);
  Serial.print(F("  Head: m")); Serial.print(Interval_uSec(1), DEC);
  Serial.print(F("  s")); Serial.println(Interval_uSec(2), DEC);
  int LowSpace= 32767; int LowMark=  32767;
  int HiSpace=0; int HiMark=  0;
  for (i = 3; i < rawlen; i++) {
    int interval= Interval_uSec(i);
    if (i % 2) {
      LowMark=min(LowMark, interval);  HiMark=max(HiMark, interval);
      Serial.print(i/2-1,DEC);  Serial.print(F(":m"));
    } 
    else {
       if(interval>0)LowSpace=min(LowSpace, interval);  HiSpace=max (HiSpace, interval);
       Serial.print(F(" s"));
    }
    Serial.print(interval, DEC);
    int j=i-1;
    if ((j % 2)==1)Serial.print(F("\t"));
    if ((j % 4)==1)Serial.print(F("\t "));
    if ((j % 8)==1)Serial.println();
    if ((j % 32)==1)Serial.println();
  }
  Serial.println();
  Serial.print(F("Mark  min:")); Serial.print(LowMark,DEC);Serial.print(F("\t max:")); Serial.println(HiMark,DEC);
  Serial.print(F("Space min:")); Serial.print(LowSpace,DEC);Serial.print(F("\t max:")); Serial.println(HiSpace,DEC);
#endif
  Serial.println();
}

/*
 * This handy little routine converts ticks from rawbuf[index] into uSec intervals adjusting for the
 * Mark/space bias.
*/
unsigned long IRdecodeBase::Interval_uSec(int index)
{
   return rawbuf[index]*USECPERTICK+( (index%2)?-MARK_EXCESS: MARK_EXCESS);
};

/*
 * Again we use a generic routine because most protocols have the same basic structure. However we need to
 * indicate whether or not the protocol varies the length of the mark or the space to indicate a "0" or "1".
 * If "Mark_One" is zero. We assume that the length of the space varies. If "Mark_One" is not zero then
 * we assume that the length of Mark varies and the value passed as "Space_Zero" is ignored.
 * When using variable length Mark, assumes Head_Space==Space_One. If it doesn't, you need a specialized decoder.
 */
bool IRdecodeBase::decodeGeneric(/*int*/int8_t Raw_Count, int Head_Mark,int Head_Space, int Mark_One, int Mark_Zero, int Space_One,int Space_Zero) {
// If raw samples count or head mark are zero then don't perform these tests.
// Some protocols need to do custom header work.
  long data = 0;  /*int*/int8_t Max; /*int*/int8_t offset;
  if (Raw_Count) {if (rawlen != Raw_Count) return RAW_COUNT_ERROR;}
  if (Head_Mark) {if (!MATCH_MARK(rawbuf[1],Head_Mark))    return HEADER_MARK_ERROR;}
  if (Head_Space) {if (!MATCH_SPACE(rawbuf[2],Head_Space)) return HEADER_SPACE_ERROR;}

  if (Mark_One) {//Length of a mark indicates data "0" or "1". Space_Zero is ignored.
    offset=2;//skip initial gap plus header Mark.
    Max=rawlen;
    while (offset < Max) {
      if (!MATCH_SPACE(rawbuf[offset], Space_One)) return DATA_SPACE_ERROR;
      offset++;
      if (MATCH_MARK(rawbuf[offset], Mark_One)) {
        data = (data << 1) | 1;
      } 
      else if (MATCH_MARK(rawbuf[offset], Mark_Zero)) {
        data <<= 1;
      } 
      else return DATA_MARK_ERROR;
      offset++;
    }
    bits = (offset - 1) / 2;
  }
  else {//Mark_One was 0 therefore length of a space indicates data "0" or "1".
    Max=rawlen-1; //ignore stop bit
    offset=3;//skip initial gap plus two header items
    while (offset < Max) {
      if (!MATCH_MARK (rawbuf[offset],Mark_Zero)) return DATA_MARK_ERROR;
      offset++;
      if (MATCH_SPACE(rawbuf[offset],Space_One)) {
        data = (data << 1) | 1;
      } 
      else if (MATCH_SPACE (rawbuf[offset],Space_Zero)) {
        data <<= 1;
      } 
      else return DATA_SPACE_ERROR;
      offset++;
    }
    bits = (offset - 1) / 2 -1;//didn't encode stop bit
  }
  // Success
  value = data;
  return true;
}

/*
 * This routine has been modified significantly from the original IRremote.
 * It assumes you've already called IRrecvBase::GetResults and it was true.
 * The purpose of GetResults is to determine if a complete set of signals
 * has been received. It then copies the raw data into your decode_results
 * structure. By moving the test for completion and the copying of the buffer
 * outside of this "decode" method you can use the individual decode
 * methods or make your own custom "decode" without checking for
 * protocols you don't use.
 * Note: Don't forget to call IRrecvBase::resume(); after decoding is complete.
 */
bool IRdecode::decode(void) {
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NEC)
  if (IRdecodeNEC::decode()) return true;
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_SONY)
  if (IRdecodeSony::decode()) return true;
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5)
  if (IRdecodeRC5::decode()) return true;
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC6)
  if (IRdecodeRC6::decode()) return true;
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_PANASONIC_OLD)
  if (IRdecodePanasonic_Old::decode()) return true;
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NECX)
  if (IRdecodeNECx::decode()) return true;
#endif
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_JVC)  
  if (IRdecodeJVC::decode()) return true;
#endif
//Deliberately did not add hash code decoding. If you get decode_type==UNKNOWN and
// you want to know a hash code you can call IRhash::decode() yourself.
// BTW This is another reason we separated IRrecv from IRdecode.
  return false;
}

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NEC)
#define NEC_RPT_SPACE	2250
bool IRdecodeNEC::decode(void) {
  ATTEMPT_MESSAGE(F("NEC"));
  // Check for repeat
  if (rawlen == 4 && MATCH_SPACE(rawbuf[2], NEC_RPT_SPACE) &&
    MATCH_MARK(rawbuf[3],564)) {
    bits = 0;
    value = REPEAT;
    decode_type = NEC;
    return true;
  }
  if(!decodeGeneric(68, 564*16, 564*8, 0, 564, 564*3, 564)) return false;
  decode_type = NEC;
  return true;
}
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_SONY)
// According to http://www.hifi-remote.com/johnsfine/DecodeIR.html#Sony8 
// Sony protocol can only be 8, 12, 15, or 20 bits in length.
bool IRdecodeSony::decode(void) {
  ATTEMPT_MESSAGE(F("Sony"));
  if(rawlen!=2*8+2 && rawlen!=2*12+2 && rawlen!=2*15+2 && rawlen!=2*20+2) return RAW_COUNT_ERROR;
  if(!decodeGeneric(0, 600*4, 600, 600*2, 600, 600,0)) return false;
  decode_type = SONY;
  return true;
}
#endif

/*
 * The next several decoders were added by Chris Young. They illustrate some of the special cases
 * that can come up when decoding using the generic decoder.
 */
#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_PANASONIC_OLD)
/*
 * A very good source for protocol information is… http://www.hifi-remote.com/johnsfine/DecodeIR.html
 * I used that information to understand what they call the "Panasonic old" protocol which is used by
 * Scientific Atlanta cable boxes. That website uses a very strange notation called IRP notation.
 * For this protocol, the notation was:
 * {57.6k,833}<1,-1|1,-3>(4,-4,D:5,F:6,~D:5,~F:6,1,-???)+ 
 * This indicates that the frequency is 57.6, the base length for the pulse is 833
 * The first part of the <x,-x|x,-x> section tells you what a "0" is and the second part
 * tells you what a "1" is. That means "0" is 833 on, 833 off while an "1" is 833 on
 * followed by 833*3=2499 off. The section in parentheses tells you what data gets sent.
 * The protocol begins with header consisting of 4*833 on and 4*833 off. The other items 
 * describe what the remaining data bits are.
 * It reads as 5 device bits followed by 6 function bits. You then repeat those bits complemented.
 * It concludes with a single "1" bit followed by and an undetermined amount of blank space.
 * This makes the entire protocol 5+6+5+6= 22 bits long since we don't encode the stop bit.
 * The "+" at the end means you only need to send it once and it can repeat as many times as you want.
 */
bool IRdecodePanasonic_Old::decode(void) {
  ATTEMPT_MESSAGE(F("Panasonic_Old"));
  if(!decodeGeneric(48,833*4,833*4,0,833,833*3,833)) return false;
  /*
   * The protocol spec says that the first 11 bits described the device and function.
   * The next 11 bits are the same thing only it is the logical Bitwise complement.
   * Many protocols have such check features in their definition but our code typically doesn't
   * perform these checks. For example NEC's least significant 8 bits are the complement of 
   * of the next more significant 8 bits. While it's probably not necessary to error check this, 
   * here is some sample code to show you how.
   */
  long S1= (value & 0x0007ff);       // 00 0000 0000 0111 1111 1111 //00000 000000 11111 111111
  long S2= (value & 0x3ff800)>> 11;  // 11 1111 1111 1000 0000 0000 //11111 111111 00000 000000
  S2= (~S2) & 0x0007ff;
  if (S1!=S2) {REJECTION_MESSAGE(F("inverted bit redundancy")); return false;};
  // Success
  decode_type = PANASONIC_OLD;
  return true;
}
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_NECX)
bool IRdecodeNECx::decode(void) {
  ATTEMPT_MESSAGE(F("NECx"));  
  if(!decodeGeneric(68,564*8,564*8,0,564,564*3,564)) return false;
  decode_type = NECX;
  return true;
}
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_JVC)
// JVC does not send any header if there is a repeat.
bool IRdecodeJVC::decode(void) {
  ATTEMPT_MESSAGE(F("JVC"));
  if(!decodeGeneric(36,525*16,525*8,0,525,525*3,525)) 
  {
     ATTEMPT_MESSAGE(F("JVC Repeat"));
     if (rawlen==34) 
     {
        if(!decodeGeneric(0,525,0,0,525,525*3,525))
           {REJECTION_MESSAGE(F("JVC repeat failed generic")); return false;}
        else {
 //If this is a repeat code then IRdecodeBase::decode fails to add the most significant bit
           if (MATCH_SPACE(rawbuf[4],(525*3))) 
           {
              value |= 0x8000;
           } 
           else
           {
             if (!MATCH_SPACE(rawbuf[4],525)) return DATA_SPACE_ERROR;
           }
        }
        bits++;
     }
     else return RAW_COUNT_ERROR;
  } 
  decode_type =JVC;
  return true;
}
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5) || (MY_IR_PROTOCOL == PROTO_RC6)
/*
 * The remaining protocols from the original IRremote library require special handling
 * This routine gets one undecoded level at a time from the raw buffer.
 * The RC5/6 decoding is easier if the data is broken into time intervals.
 * E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
 * successive calls to getRClevel will return MARK, MARK, SPACE.
 * offset and used are updated to keep track of the current position.
 * t1 is the time interval for a single bit in microseconds.
 * Returns ERROR if the measured time interval is not a multiple of t1.
 */
IRdecodeRC::RCLevel IRdecodeRC::getRClevel(int *offset, int *used, int t1) {
  if (*offset >= rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  int width = rawbuf[*offset];
  IRdecodeRC::RCLevel val;
  if ((*offset) % 2) val=MARK; else val=SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  int avail;
  if (MATCH(width, t1 + correction)) {
    avail = 1;
  } 
  else if (MATCH(width, 2*t1 + correction)) {
    avail = 2;
  } 
  else if (MATCH(width, 3*t1 + correction)) {
    avail = 3;
  } 
  else {
    return ERROR;
  }
  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
#ifdef DEBUG
  if (val == MARK) Serial.println("MARK"); else Serial.println("SPACE"); 
#endif
  return val;   
}
#endif

#define MIN_RC5_SAMPLES 11
#define MIN_RC6_SAMPLES 1

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC5)
bool IRdecodeRC5::decode(void) {
  ATTEMPT_MESSAGE(F("RC5"));
  if (rawlen < MIN_RC5_SAMPLES + 2) return RAW_COUNT_ERROR;
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  // Get start bits
  if (getRClevel(&offset, &used, RC5_T1) != MARK) return HEADER_MARK_ERROR;
//Note: Original IRremote library incorrectly assumed second bit was always a "1"
//bit patterns from this decoder are not backward compatible with patterns produced
//by original library. Ucomment the following two lines to maintain backward compatibility.
  //if (getRClevel(&offset, &used, RC5_T1) != SPACE) return HEADER_SPACE_ERROR;
  //if (getRClevel(&offset, &used, RC5_T1) != MARK) return HEADER_MARK_ERROR;
  int nbits;
  for (nbits = 0; offset < rawlen; nbits++) {
    RCLevel levelA = getRClevel(&offset, &used, RC5_T1); 
    RCLevel levelB = getRClevel(&offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      // 1 bit
      data = (data << 1) | 1;
    } 
    else if (levelA == MARK && levelB == SPACE) {
      // zero bit
      data <<= 1;
    } 
    else return DATA_MARK_ERROR;
  }
  // Success
  bits = 13;
  value = data;
  decode_type = RC5;
  return true;
}
#endif

#if defined(ALL_IR_PROTOCOL) || (MY_IR_PROTOCOL == PROTO_RC6)
bool IRdecodeRC6::decode(void) {
  ATTEMPT_MESSAGE(F("RC6"));
  if (rawlen < MIN_RC6_SAMPLES) return RAW_COUNT_ERROR;
  // Initial mark
  if (!MATCH_MARK(rawbuf[1], RC6_HDR_MARK)) return HEADER_MARK_ERROR;
  if (!MATCH_SPACE(rawbuf[2], RC6_HDR_SPACE)) return HEADER_SPACE_ERROR;
  int offset=3;//Skip gap and header
  long data = 0;
  int used = 0;
  // Get start bit (1)
  if (getRClevel(&offset, &used, RC6_T1) != MARK) return DATA_MARK_ERROR;
  if (getRClevel(&offset, &used, RC6_T1) != SPACE) return DATA_SPACE_ERROR;
  int nbits;
  for (nbits = 0; offset < rawlen; nbits++) {
    RCLevel levelA, levelB; // Next two levels
    levelA = getRClevel(&offset, &used, RC6_T1); 
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != getRClevel(&offset, &used, RC6_T1)) return TRAILER_BIT_ERROR;
    } 
    levelB = getRClevel(&offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != getRClevel(&offset, &used, RC6_T1)) return TRAILER_BIT_ERROR;
    } 
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    } 
    else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    } 
    else {
      return DATA_MARK_ERROR; 
    } 
  }
  // Success
  bits = nbits;
  value = data;
  decode_type = RC6;
  return true;
}
#endif

#if defined(ALL_IR_PROTOCOL) //|| (MY_IR_PROTOCOL == HASH_CODE)
/*
 * This Hash decoder is based on IRhashcode
 * Copyright 2010 Ken Shirriff
 * For details see http://www.arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 * Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
 * Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 */
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261
// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
int IRdecodeHash::compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) return 0;
  if (oldval < newval * .8) return 2;
  return 1;
}

bool IRdecodeHash::decode(void) {
  hash = FNV_BASIS_32;
  for (int i = 1; i+2 < rawlen; i++) {
    hash = (hash * FNV_PRIME_32) ^ compare(rawbuf[i], rawbuf[i+2]);
  }
//note: does not set decode_type=HASH_CODE nor "value" because you might not want to.
  return true;
}
#endif

/*
 * This section is all related to interrupt handling and hardware issues. It has nothing to do with IR protocols.
 * You need not understand this is all you're doing is adding new protocols or improving the decoding and sending
 * of protocols.
 *
 */

// Provides ISR
#include <avr/interrupt.h>
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define CLKFUDGE 5      // fudge factor for clock interrupt overhead
#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif
#define PRESCALE 8      // timer clock prescale
#define CLKSPERUSEC (SYSCLOCK/PRESCALE/1000000)   // timer clocks per microsecond

#include <IRLibTimer.h>


IRrecv::IRrecv(int recvpin)
{
  irparams.recvpin = recvpin;
  irparams.blinkflag = 0;
}
/* If your hardware is set up to do both output and input but your particular sketch
 * doesn't do any output, this method will ensure that your output pin is low
 * and doesn't turn on your IR LED or any output circuit.
 */
void IRrecv::No_Output (void) {
 pinMode(TIMER_PWM_PIN, OUTPUT);  
 digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low    
}

void IRrecv::enableIRIn() {
  // setup pulse clock timer interrupt
  cli();
  TIMER_CONFIG_NORMAL();
  TIMER_ENABLE_INTR;
  TIMER_RESET;
  sei();
  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
  // set pin modes
  pinMode(irparams.recvpin, INPUT);
}
// enable/disable blinking of pin 13 on IR processing
void IRrecv::blink13(int blinkflag)
{
  irparams.blinkflag = blinkflag;
  if (blinkflag)
     pinMode(BLINKLED, OUTPUT);
}
void IRrecv::resume() {
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
}
bool IRrecv::GetResults(IRdecodeBase *decoder) {
  if (irparams.rcvstate != STATE_STOP) return false;
  decoder->Reset();//clear out any old values.
  decoder->rawlen = (unsigned int)irparams.rawlen;
//By copying the entire array we could call IRrecv::resume immediately while decoding
//is still in progress.
  if(decoder->rawbuf != irparams.rawbuf)
     memcpy((void *)decoder->rawbuf,(const void *)irparams.rawbuf,sizeof(irparams.rawbuf));
  return true;
}
#define _GAP 5000 // Minimum map between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
ISR(TIMER_INTR_NAME)
{
  TIMER_RESET;
  enum irdata_t {IR_MARK=0, IR_SPACE=1};
  irdata_t irdata = (irdata_t)digitalRead(irparams.recvpin);
  irparams.timer++; // One more 50us tick
  if (irparams.rawlen >= RAWBUF) {
    // Buffer overflow
    irparams.rcvstate = STATE_STOP;
  }
  switch(irparams.rcvstate) {
  case STATE_IDLE: // In the middle of a gap
    if (irdata == IR_MARK) {
      if (irparams.timer < GAP_TICKS) {
        // Not big enough to be a gap.
        irparams.timer = 0;
      } 
      else {
        // gap just ended, record duration and start recording transmission
        irparams.rawlen = 0;
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      }
    }
    break;
  case STATE_MARK: // timing MARK
    if (irdata == IR_SPACE) {   // MARK ended, record time
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = STATE_SPACE;
    }
    break;
  case STATE_SPACE: // timing SPACE
    if (irdata == IR_MARK) { // SPACE just ended, record it
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = STATE_MARK;
    } 
    else { // SPACE
      if (irparams.timer > GAP_TICKS) {
        // big SPACE, indicates gap between codes
        // Mark current code as ready for processing
        // Switch to STOP
        // Don't reset timer; keep counting space width
        irparams.rcvstate = STATE_STOP;
      } 
    }
    break;
  case STATE_STOP: // waiting, measuring gap
    if (irdata == IR_MARK) { // reset gap timer
      irparams.timer = 0;
    }
    break;
  }
  if (irparams.blinkflag) {
    if (irdata == IR_MARK) {
      BLINKLED_ON();  // turn pin 13 LED on
    } 
    else {
      BLINKLED_OFF();  // turn pin 13 LED off
    }
  }
}

#ifdef USE_IR_SEND
IRsendBase::IRsendBase () {
 pinMode(TIMER_PWM_PIN, OUTPUT);  
 digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low    
}

void IRsendBase::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (OC2B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
  
  // Disable the Timer2 Interrupt (which is used for receiving IR)
 TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt    
 pinMode(TIMER_PWM_PIN, OUTPUT);  
 digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low    
 TIMER_CONFIG_KHZ(khz);
 }
 
void IRsendBase::mark(int time) {
 TIMER_ENABLE_PWM;
 delayMicroseconds(time);
}

void IRsendBase::space(int time) {
 TIMER_DISABLE_PWM;
 delayMicroseconds(time);
}
#endif

/*
 * Various debugging routines
 */

#ifdef DEBUG
int MATCH(int measured, int desired) {
  Serial.print("Testing: ");  Serial.print(TICKS_LOW(desired), DEC);  
  Serial.print(" <= ");  Serial.print(measured, DEC);  Serial.print(" <= ");  Serial.println(TICKS_HIGH(desired), DEC);
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int MATCH_MARK(int measured_ticks, int desired_us) {
  Serial.print("Testing mark ");  Serial.print(measured_ticks * USECPERTICK, DEC);  Serial.print(" vs ");  Serial.print(desired_us, DEC);  Serial.print(": ");
  Serial.print(TICKS_LOW(desired_us + MARK_EXCESS), DEC);  Serial.print(" <= "); Serial.print(measured_ticks, DEC);  
  Serial.print(" <= ");  Serial.println(TICKS_HIGH(desired_us + MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us + MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us + MARK_EXCESS);
}

int MATCH_SPACE(int measured_ticks, int desired_us) {
  Serial.print("Testing space ");  Serial.print(measured_ticks * USECPERTICK, DEC);  Serial.print(" vs ");  Serial.print(desired_us, DEC);  Serial.print(": ");
  Serial.print(TICKS_LOW(desired_us - MARK_EXCESS), DEC);  Serial.print(" <= ");  Serial.print(measured_ticks, DEC);
  Serial.print(" <= ");  Serial.println(TICKS_HIGH(desired_us - MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us - MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us - MARK_EXCESS);
}
#endif


#ifdef TRACE
void ATTEMPT_MESSAGE(const __FlashStringHelper * s) {Serial.print(F("Attempting ")); Serial.print(s); Serial.println(F(" decode:"));};
byte REJECTION_MESSAGE(const __FlashStringHelper * s) { Serial.print(F(" Protocol failed because ")); Serial.print(s); Serial.println(F(" wrong.")); return false;};
#endif
