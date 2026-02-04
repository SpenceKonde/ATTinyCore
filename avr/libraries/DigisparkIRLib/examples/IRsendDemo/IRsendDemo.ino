/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.0  January 2013
 * Copyright 2013 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
#include <IRLib.h>

IRsend My_Sender;

void setup()
{
  Serial.begin(9600);
}

void loop() {
  if (Serial.read() != -1) {
    //send a code  every time a character is received from the serial port
    //Sony DVD power A8BCA
    My_Sender.send(SONY,0xa8bca, 20);
  }
}

