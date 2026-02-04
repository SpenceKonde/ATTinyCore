/*
  Software serial multiple serial test
 
 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.
 
 The circuit: 
 * RX is digital pin 2 (connect to TX of other device)
 * TX is digital pin 3 (connect to RX of other device)
 
 created back in the mists of time
 modified 9 Apr 2012
 by Tom Igoe
 based on Mikal Hart's example
 
 This example code is in the public domain.
 
 <SoftSerial> adapted from <SoftwareSerial> for <TinyPinChange> library which allows sharing the Pin Change Interrupt Vector.
 Single difference with <SoftwareSerial>: add #include <TinyPinChange.h>  at the top of your sketch.
 RC Navy (2012): http://p.loussouarn.free.fr
 
 */
#include <SoftSerial.h>     /* Allows Pin Change Interrupt Vector Sharing */
#include <TinyPinChange.h>  /* Ne pas oublier d'inclure la librairie <TinyPinChange> qui est utilisee par la librairie <RcSeq> */

SoftSerial mySerial(2, 3); // RX, TX

void setup()  
{
 // Open serial communications and wait for port to open:
  Serial.begin(57600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  
  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  mySerial.println("Hello, world?");
}

void loop() // run over and over
{
  if (mySerial.available())
    Serial.write(mySerial.read());
  if (Serial.available())
    mySerial.write(Serial.read());
}
