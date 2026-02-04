/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
/*
 * This example demonstrates how to extend this library to add a new protocol
 * without actually modifying or recompiling the library itself. It implements a 36 bit
 * Samsung protocol that is used on a Blu-ray player that I own.
 * Because a 36 bit value will not fit in the value field (only 32 bits) we have to create
 * a second value field.
 */
#include <IRLib.h>
#include <IRLibMatch.h>

int RECV_PIN = 11;

IRrecv My_Receiver(RECV_PIN);
#define SAMSUNG36 (LAST_PROTOCOL+1) //Unfortunately cannot extend an enum. This is the best we can do.

/*Is the value in this if statement is "1" then we will extend the base decoder.
 *The result is this will only decode Samsung 36 and no others.
 *If the value is "0" we will extend the all-inclusive decoder.
 *Try changing this value and note the effect on code size when compiling.
 */
#if(1)
  #define MY_BASE IRdecodeBase
#else
  #define MY_BASE IRdecode
#endif
class IRdecodeSamsung36: public virtual MY_BASE{
public:
  bool decode(void); 
  unsigned int value2;
  void Reset(void);
  void DumpResults(void);
private:
  bool GetBit(void);
  int offset;
  unsigned long data;
};
void IRdecodeSamsung36::Reset(void) {
  MY_BASE::Reset();//respect your parents
  value2=0;
};

bool IRdecodeSamsung36::GetBit(void) {
  if (!MATCH_MARK (rawbuf[offset],500)) return DATA_MARK_ERROR;
  offset++;
  if (MATCH_SPACE(rawbuf[offset],1500)) {
    data = (data << 1) | 1;
  } 
  else if (MATCH_SPACE (rawbuf[offset],500)) {
    data <<= 1;
  } 
  else return DATA_SPACE_ERROR;
  offset++;
  return true;
};
/*
 * According to http://www.hifi-remote.com/johnsfine/DecodeIR.html#Samsung36
 * The IRP notation for this protocol is:
 * {38k,500}<1,-1|1,-3>(9,-9,D:8,S:8,1,-9,E:4,F:8,-68u,~F:8,1,-118)+ 
 * This means it uses 38k frequency. Base timing is multiples of 500.
 * A "0" is mark(500) space(500). A "1" is mark (500) space(1500)
 * The header is mark(4500) space(4500).
 * The header is followed by 16 bits (8 device, 8 sub device)
 * This is followed by a mark(500) space(4500).
 * This is followed by 12 more bits (4+8)
 * This is followed by 68us ofspace. Followed by eight more bits
 * and a final stop bit.
*/
bool IRdecodeSamsung36::decode(void) {
  if(MY_BASE::decode()) return true;
  ATTEMPT_MESSAGE(F("Samsung36"));
  if (rawlen != 78) return RAW_COUNT_ERROR;
  if (!MATCH_MARK(rawbuf[1],4500))  return HEADER_MARK_ERROR;
  if (!MATCH_SPACE(rawbuf[2],4500)) return HEADER_SPACE_ERROR;
  offset=3; data=0;
  //Get first 18 bits
  while (offset < 16*2+2) if(!GetBit()) return false;
  //Skip middle header
  if (!MATCH_MARK(rawbuf[offset],500))  return DATA_MARK_ERROR;
  offset++;
  if (!MATCH_SPACE(rawbuf[offset],4500)) return DATA_SPACE_ERROR;
  //save first 18 bits in "value" and reset data
  offset++; value=data; data=0;
  rawbuf[62]=(rawbuf[62]*USECPERTICK-68)/USECPERTICK;
  while(offset<77)if(!GetBit()) return false;
  bits =36;//set bit length
  value2 = data;//put remaining bits in value2
  decode_type= static_cast<IRTYPES>SAMSUNG36;
  return true;
};
void IRdecodeSamsung36::DumpResults(void){
  if(decode_type==SAMSUNG36) {
    Serial.print(F("Decoded Samsung36: Value1:")); Serial.print(value, HEX);
    Serial.print(F(": Value2:")); Serial.print(value2, HEX);
  };
  MY_BASE::DumpResults();
};
IRdecodeSamsung36 My_Decoder;

void setup()
{
  Serial.begin(9600);
  My_Receiver.enableIRIn(); // Start the receiver
}

void loop() {
  if (My_Receiver.GetResults(&My_Decoder)) {
    My_Decoder.decode();
    My_Decoder.DumpResults();
    My_Receiver.resume(); 
  }
}

