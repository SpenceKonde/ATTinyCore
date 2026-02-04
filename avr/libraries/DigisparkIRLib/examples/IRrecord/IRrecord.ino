/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
/*
 * IRrecord: record and play back IR signals 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * Unlike the original version of this demo sketch, you need not hook up a pushbutton
 * Simply send any character from the serial screen to send the recorded code.
 * Also demonstrates how to use toggle bits which must be controlled outside
 * the library routines.
 * The logic is:
 * If an IR code is received, record it.
 * If A serial character is received, send the IR code.
 */

#include <IRLib.h>

int RECV_PIN = 11;

IRrecv My_Receiver(RECV_PIN);
IRdecode My_Decoder;
IRsend My_Sender;
/*
 * Because this version of the library separated the receiver from the decoder,
 * technically you would not need to "store" the code outside the decoder object
 * for this overly simple example. All of the details would remain in the object.
 * However we are going to go ahead and store them just to show you how.
 */
// Storage for the recorded code
IRTYPES codeType; // The type of code
unsigned long codeValue;   // The data bits if type is not raw
int codeBits; // The length of the code in bits
// These values are only stored if it's an unknown type and we are going to use
// raw codes to resend the information.
unsigned int rawCodes[RAWBUF]; // The durations if raw
int rawCount; //The number of interval samples

bool GotOne, GotNew; 

void setup()
{
  GotOne=false; GotNew=false;
  codeType=UNKNOWN; 
  codeValue=0; 
  Serial.begin(9600);
  Serial.println(F("Send a code from your remote and we will record it."));
  Serial.println(F("Type any character and press enter. We will send the recorded code."));
  My_Receiver.enableIRIn(); // Start the receiver
}

// Stores the code for later playback
void storeCode(void) {
  GotNew=true;
  codeType = My_Decoder.decode_type;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    rawCount = My_Decoder.rawlen-1;
    for (int i = 1; i <=rawCount; i++) {
      rawCodes[i - 1] = My_Decoder.Interval_uSec(i);//Converts to microseconds and adjusts for Mark/space
    };
    My_Decoder.DumpResults();
    codeType=UNKNOWN;
  }
  else {
    Serial.print(F("Received "));
    Serial.print(Pnames(codeType));
    if (My_Decoder.value == REPEAT) {
      // Don't record a NEC repeat value as that's useless.
      Serial.println("repeat; ignoring.");
     }
     else {
       codeValue = My_Decoder.value;
       codeBits = My_Decoder.bits;
     }
     Serial.print(F(" Value:0x"));
     Serial.println(My_Decoder.value, HEX);
  }
}
void sendCode(int repeat) {
  if(codeType== UNKNOWN) {
    // Assume 38 KHz
    My_Sender.IRsendRaw::send(rawCodes,rawCount,38);
    Serial.println("Sent raw");
    return;
  }
  if( !GotNew ) {//We've already sent this so handle toggle bits
    if (codeType == RC5) {
      codeValue ^= 0x0800;
    }
    else if (codeType == RC6) {
      codeValue ^= 0x10000;
    }
  }
  GotNew=false;
  My_Sender.send(codeType,codeValue,codeBits);
  Serial.print(F("Sent "));
  Serial.print(Pnames(codeType));
  Serial.print(F(" Value:0x"));
  Serial.println(codeValue, HEX);
}

void loop() {
  if (Serial.read() != -1) {
    if(GotOne) {
      sendCode(0);
      My_Receiver.enableIRIn(); // Re-enable receiver
    }
  } 
  else if (My_Receiver.GetResults(&My_Decoder)) {
    //Restart the receiver so it can be capturing another code
    //while we are working on decoding this one.
    if(My_Decoder.decode()) {
      GotOne=true;
      storeCode();
    }
    delay(500);
    My_Receiver.resume(); 
  }
}

