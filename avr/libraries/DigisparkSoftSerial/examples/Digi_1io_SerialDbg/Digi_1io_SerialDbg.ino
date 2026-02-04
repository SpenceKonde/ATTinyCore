/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  |_|  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013

                http://p.loussouarn.free.fr

         ****************************************
         *   Digispark Debug Demo with 1 I/O    *
         ****************************************

 This sketch demonstrates how to debug a Digispark using a bi-directional serial port using a single I/O.
 This approach allows to use the built-in Serial Console of the arduino IDE.
 Please, note this solution requires a native RS232 port (rare today) or a RS232/USB adapter on the development PC.
 
 Hardware Wiring:
 ===============
                        SERIAL SINGLE I/O
                         DEBUGGING CABLE
            ___________________/\__________________
           /                                       \
                                              ____
 .--------.                                  |    \
 |    GND |--------------------------------+---o5  \
 |        |                           47K  | |   9o |
 |        |                        .--###--' | o4   |
 |  DEBUG |                  4.7K  |         |   8o |
 |  TX_RX |-------------------###--+--|<|------o3   |    ---> To regular RS232 SubD 9 pins Male of PC
 |   PIN  |        ^               | 1N4148  |   7o |         or to RS232/USB adapter
 |        |        |               '-----------o2   |
 '--------'        |                         |   6o |
  ATtiny85       Single                      | o1  /
 (Digispark)      I/O                        |____/
                                          SubD 9 pins
                                            Female
*/
#include <TinyPinChange.h>
#include <SoftSerial.h>
 
#define DEBUG_TX_RX_PIN         2 //Adjust here your Tx/Rx debug pin
 
SoftSerial MyDbgSerial(DEBUG_TX_RX_PIN, DEBUG_TX_RX_PIN, true); //true allows to connect to a regular RS232 without RS232 line driver

#define SERIAL_BAUD_RATE        57600 //Adjust here the serial rate (57600 is the higher supported rate)

void setup()
{
  MyDbgSerial.begin(SERIAL_BAUD_RATE); //After MyDbgSerial.begin(), the serial port is in rxMode by default
  MyDbgSerial.txMode(); //Before sending a message, switch to txMode
  MyDbgSerial.println(F("\nDebug enabled"));
  MyDbgSerial.rxMode(); //switch to rxMode to be ready to receive some commands
}
 
void loop()
{
  if(MyDbgSerial.available())
  {
    MyDbgSerial.txMode();
    MyDbgSerial.print(F("\nReceived: "));MyDbgSerial.write(MyDbgSerial.read());MyDbgSerial.print(F("\n"));
    MyDbgSerial.rxMode();
  }
}

