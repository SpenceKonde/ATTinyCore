/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  |_|  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013

                http://p.loussouarn.free.fr

         *******************************************
         *   Digispark RC Debug Demo with 1 I/O    *
         *******************************************

 This sketch demonstrates how to display received RC pulse width with a Digispark in the serial console of the arduino IDE.
 by using a bi-directional serial port using a single I/O.
 This approach allows to use the built-in Serial Console of the arduino IDE.
 Please, note this solution requires a native RS232 port (rare today) or a RS232/USB adapter on the development PC.
 
 To display properly the outputs in the IDE serial console, you have to:
 - in the arduino IDE, select "Tools->Board->Digispark 16.0mhz - NO USB (Tiny core)"
 - in the arduino IDE, select the right serial port in "Tools->Serial Port" and select the port where the debug cable is connected to.
 - in the serial console, set the right baud rate (38400 in this sketch)
 
 In this sketch, only Tx capabilty of SoftSerial is used.
 
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
#include <SoftRcPulseIn.h>
#include <SoftSerial.h>

#define RX_AUX_GEAR_PIN   0 //Choose here the pin for the RC signal
#define DEBUG_TX_RX_PIN   1 //Adjust here your Tx/Rx debug pin (Do NOT work on Digispark PIN 5: choose another PIN)

#define SERIAL_BAUD_RATE  57600 //Adjust rate here

SoftRcPulseIn RxAuxGear;    //Choose a name for your RC channel signal

SoftSerial    MyDbgSerial(DEBUG_TX_RX_PIN, DEBUG_TX_RX_PIN, true); //true allows to connect to a regular RS232 without RS232 line driver 

void setup()
{
  RxAuxGear.attach(RX_AUX_GEAR_PIN);
  
  MyDbgSerial.begin(SERIAL_BAUD_RATE); //Do NOT forget to setup your terminal at same baud rate (eg: arduino IDE serial monitor)
  MyDbgSerial.txMode();     //Before sending a message, switch to txMode
  MyDbgSerial.print(F("SoftRcPulseIn lib V"));MyDbgSerial.print(SoftRcPulseIn::LibTextVersionRevision());MyDbgSerial.println(F(" demo"));
}

void loop()
{
  if(RxAuxGear.available())
  {
    MyDbgSerial.print(F("Pulse="));MyDbgSerial.println(RxAuxGear.width_us()); // Display Rx Pulse Width (in us)
  }
}


