/*
  Software serial multiple serial test

  Receives from the hardware serial, sends to software serial.
  Receives from software serial, sends to hardware serial.

  The circuit:
  * RX is digital pin 10 (connect to TX of other device)
  * TX is digital pin 11 (connect to RX of other device)

  Note: Only pins that support Pin Change interrupts (PCINTs) can be used.
  If you are using Software Serial, you may not use any other library that uses
  PCINTs, nor may you use them in your own code.

  (however, this is ATTinyCore - we provide a software serial named Serial that
  does not have that restriction (though it still has all the others, see the
  part-specific documentation for the part you are using for more details).
  You should use that fake hardware serial instead of these software ones
  when you can.

  Created back in prehistoric times.
  Modified in March 31 2023 to add the little blob of #if's at the top tp
  make sure it can find a pin that exists on this part, so that we can use
  this for CI testing. I also removed the "while !(Serial)" line, and added
  basically all the comments, reminding people of the limits of software serial.
  This was done by Spence Konde for ATTinyCore 2.0.0.
  The previous time someone has admitted to modifying it was 25 May 2012,
  and the modifying person was none other than Tom Igoe,
  who notes that it was "based on Mikal Hart's example"

  This example code is in the public domain.
 */
#include <SoftwareSerial.h>

#if defined(__AVR_ATtinyX5__)
  // the tiny x5 has only 6 I/O pins, only 5 of which are usable without
  // disabling reset. Nobody wants to do THAT. "Serial", the software-
  // serial-that-looks-like-hw-serial, is on pins 0 and 1, so let's use
  // 3 and 4 for the example on an x5. These are pins PB3 and PB4.
  SoftwareSerial mySerial(PIN_PB3, PIN_PB4);
  // this is the recommended way to refer to pins (PIN_Pxn)
#else
  // Everything else has pins with numbers 10 and 11.
  // We aren't using Pxn notation here because the port and number
  // within that port is going to vary between parts. 10 and 11
  // may be stupid choices for some parts, but I need something here that
  // compiles, because SoftwareSerial is part of the CI testing.
  SoftwareSerial mySerial(10, 11); // RX, TX
#endif

void setup() {
  // Open serial communications on either the "fake" hardware serial port
  // which is actually another implementation of software serial (if there
  // is no hardware serial) or the actual hardware serial port (if any)
  Serial.begin(9600);

  Serial.println("Goodnight moon!");

  // Now we set up the software serial port. This is a conservative speed, though
  // we're pretty sure that our software "Serial" is faster than "SoftwareSerial".
  // Hardware serial can go faster than either of them - though nowhere near as
  // fast in practice as the headline specs suggest. On modern (post-2016 release)
  // AVRs, the hardware serial ports have the same headline top speed, but can
  // actually achieve it.
  mySerial.begin(4800);
  mySerial.println("Hello, world?");
}

void loop() // run over and over
{
  if (mySerial.available())
    Serial.write(mySerial.read()); // Warning: Unless this part has hardware serial, during this write, any data that is received will be lost or corrupted.
  if (Serial.available())
    mySerial.write(Serial.read()); // Warning: Unless this part has hardware serial, during this write, any data that is received will be lost or corrupted.
  //                             Additionally, even with hardware serial, if more than 1 byte is received before this byte is done sending, it will be lost.
}
