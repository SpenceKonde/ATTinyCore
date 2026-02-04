/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
/*
 * JVC sends repeat codes that are identical to the regular JVC codes
 * however they have no header. Therefore there is an additional parameter
 * that tells you whether or not to send as an original code or as a repeat.
 *
 * The only device I had to test this protocol was an old JVC VCR. It would only work if at least
 * 2 frames are sent separated by 45us of "space". All JVC is the same bit length so we use
 * the third parameter as a to tell it whether or not to send the header.
 * Once with  the third parameter "1" then delay about 50 microseconds and send again
 * with the third parameter "0".
 */

#include <IRLib.h>

IRsend My_Sender;

void setup()
{
  Serial.begin(9600);
}

//send a code every time a character is received from the serial port
void loop() {
  if (Serial.read() != -1) {
    My_Sender.send(JVC,0xc2d0,1);   delayMicroseconds (50);
    My_Sender.send(JVC,0xc2d0,0);   delayMicroseconds (50);
  }
}

