/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013/2014

                http://p.loussouarn.free.fr

         *******************************************************
         *              <TinyPinChange> library Demo           *
         *           with debugging capabilities using         *
         * <SoftSerial> object as single wire serial interface *
         *******************************************************

This sketch demonstrates how to use <TinyPinChange> library.
It counts all the FALLING edges on 2 different pins.
/!\CAUTION/!\: as <TinyPinChange> library can be shared (and it is with SoftSerial in this sketch) , the user shall test if the changes are related to the declared pins.

Trick: By connecting Pin#1 to Pin#0 or to Pin#5 through a 1K resistor, you can generate transitions for testing purpose.
Output results are sent to a software serial.

And the great thing is: using a <SoftSerial> object as a bi-directionnal software serial port (half-duplex) on a single pin to communicate with the outside world!

To display the sketch results on a PC (in a Terminal):
1) Build the "Serial One Wire Debug Cable" and plug it to the regular RS232 port as depicted below,
2) Open your favorite Terminal at 38400,n,8,1: HyperTerminal, Teraterm (Windows) or Minicom, GtkTerm (Linux) and CoolTerm (MAC) does the trick.
3) You can also use the Serial Monitor of the arduino IDE: Tools->Serial Port and select your RS232 port (may be an USB virtual port), Rate=38400.
4) To enable the display, type 1, to disable, type 0 in the Terminal/Monitor.

                        SERIAL ONE WIRE
                        DEBUGGING CABLE
            _______________          ________________
           /               \___/\___/                \
                                              ____
 .--------.                                  |    \
 |    GND |--------------------------------+---o5  \
 |        |                           47K  | |   9o |
 |        |                        .--###--' | o4   |
 |  DEBUG |                  4.7K  |         |   8o |
 |  TX_RX |-------------------###--+--|<|------o3   |    ---> To regular RS232 SubD 9 pins Male of PC or Serial/USB adapter
 |   PIN  |        ^               | 1N4148  |   7o |
 |        |        |               '-----------o2   |
 '--------'        |                         |   6o |
  ATtiny85       Single                      | o1  /
 (Digispark)      I/O                        |____/
                                          SubD 9 pins
                                            Female
*/
#include <TinyPinChange.h>
#include <SoftSerial.h>

#define LED_PIN             1

#define DEBUG_TX_RX_PIN     2

#define FIRST_INPUT         0
#define SECOND_INPUT        5

volatile uint16_t FirstInputChangeCount = 0;  /* Volatile since the variable will be updated in interruption */
volatile uint16_t SecondInputChangeCount = 0; /* Volatile since the variable will be updated in interruption */

SoftSerial MySerial(DEBUG_TX_RX_PIN, DEBUG_TX_RX_PIN, true); /* Tx/Rx on a single Pin !!! (Pin#2) */

uint8_t VirtualPortNb;
uint8_t VirtualPortNb_;

void setup() 
{
  TinyPinChange_Init();

  MySerial.begin(57600); /* Trick: use a "high" data rate (less time wasted in ISR and for transmitting each character) */

  VirtualPortNb  = TinyPinChange_RegisterIsr(FIRST_INPUT,  InterruptFunctionToCall);
  VirtualPortNb_ = TinyPinChange_RegisterIsr(SECOND_INPUT, InterruptFunctionToCall);

  /* Enable Pin Change for each pin */
  TinyPinChange_EnablePin(FIRST_INPUT);
  TinyPinChange_EnablePin(SECOND_INPUT);

  MySerial.txMode();
  MySerial.println(F("\n*** Tiny PinChange Demo (Falling Edge) ***"));
  MySerial.print(F("Pin "));MySerial.print((int)FIRST_INPUT);
  MySerial.print(F(" is part of virtual port "));MySerial.println((int)VirtualPortNb);

  MySerial.print(F("Pin "));MySerial.print((int)SECOND_INPUT);
  MySerial.print(F(" is part of virtual port "));MySerial.println((int)VirtualPortNb_);

  MySerial.println(F("As you can see, virtual port is always port 0 for ATtiny85"));
  MySerial.println(F("Remember <TinyPinChange> is also designed for UNO, MEGA, ATtiny84 and ATtiny167 ;-)"));
  MySerial.println(F("Type 1 to start display, 0 to stop display"));
 
  pinMode(LED_PIN, OUTPUT);
  
  MySerial.rxMode(); /* Switch to Rx Mode */
}

/* Function called in interruption in case of change on pins */
void InterruptFunctionToCall(void)
{
    if(TinyPinChange_FallingEdge(VirtualPortNb, FIRST_INPUT)) /* Check for FIRST_INPUT Rising Edge */
    {
        FirstInputChangeCount++; /* Only Falling edges are counted */
    }
    
    if(TinyPinChange_FallingEdge(VirtualPortNb_, SECOND_INPUT)) /* Check for SECOND_INPUT Rising Edge */
    {
        SecondInputChangeCount++; /* Only Falling edges are counted */
    }
}

void loop()
{
static boolean State = HIGH, DisplayEnabled = true;
static uint32_t LedStartMs = millis(), DisplayStartMs = millis();
uint16_t LocalFirstInputChangeCount;
uint16_t LocalSecondInputChangeCount;

  /* Blink the built-in LED */
  if(millis() - LedStartMs >= 500UL)
  {
    LedStartMs = millis();
    digitalWrite(LED_PIN, State);
    State = !State; /* State will be inverted at the next digitalWrite() */
  }
  
  /* Get command from single wire SoftSerial */
  if(MySerial.available())
  {
    switch(MySerial.read())
    {
      case '0':
      DisplayEnabled = false;
      break;
      
      case '1':
      DisplayEnabled = true;
      break;
    }
  }

  /* Diplay Transition numbers every second */
  if((millis() - DisplayStartMs >= 1000UL) && DisplayEnabled)
  {
    DisplayStartMs = millis();
    noInterrupts(); /* Mandatory since counters are 16 bits */
    LocalFirstInputChangeCount = FirstInputChangeCount;
    LocalSecondInputChangeCount = SecondInputChangeCount;
    interrupts();
    MySerial.txMode();
    MySerial.print(F("FirstInputChangeCount="));MySerial.println(LocalFirstInputChangeCount);
    MySerial.print(F("SecondInputChangeCount="));MySerial.println(LocalSecondInputChangeCount);
    MySerial.rxMode();
  }
  
}

