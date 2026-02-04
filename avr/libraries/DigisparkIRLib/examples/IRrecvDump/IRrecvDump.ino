/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
/*
 * IRLib: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 */

#include <IRLib.h>

int RECV_PIN = 11;

IRrecv My_Receiver(RECV_PIN);

IRdecode My_Decoder;
unsigned int Buffer[RAWBUF];

void setup()
{
  Serial.begin(9600);
  My_Receiver.enableIRIn(); // Start the receiver
  My_Decoder.UseExtnBuf(Buffer);
}

void loop() {
  if (My_Receiver.GetResults(&My_Decoder)) {
    //Restart the receiver so it can be capturing another code
    //while we are working on decoding this one.
    My_Receiver.resume(); 
    My_Decoder.decode();
    My_Decoder.DumpResults();
  }
}

