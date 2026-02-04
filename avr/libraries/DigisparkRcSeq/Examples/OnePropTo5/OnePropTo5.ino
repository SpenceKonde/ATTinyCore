#include <RcSeq.h>
#include <TinyPinChange.h>
#include <SoftRcPulseIn.h>

/*
IMPORTANT:
For this sketch to compile, RC_SEQ_WITH_SOFT_RC_PULSE_IN_SUPPORT and RC_SEQ_WITH_SHORT_ACTION_SUPPORT shall be defined
in PathOfTheLibraries/(Digispark)RcSeq/RcSeq.h and RC_SEQ_WITH_SOFT_RC_PULSE_OUT_SUPPORT shall be commented.

This sketch demonstrates how to easily transform a proportionnal RC channel into 5 digital commands with an ATtiny85.
RC Navy (2013)
http://p.loussouarn.free.fr

COMMMAND OF 5 digital outputs from 5 push button replacing a potentiometer in the RC transmitter:
================================================================================================
   Output pins: #1, #2, #3, #4, #5 of an ATtiny85 or a Digispark
   The receiver output channel is connected to pin#0 of an ATtiny85 or a Digispark
   A furtive pressure on the push button on the transmitter toggles the corresponding output on the ATtiny85 or a Digispark
   connected to the receiver output channel.
   Version with RcSeq library inspired by: http://bateaux.trucs.free.fr/huit_sorties.html

Modification at RC Transmitter side:
===================================
                                                Custom keyboard with push buttons
                                                =================================
  Stick Potentiometer                       1K        1K        1K        1K        1K        1K
  ===================                   .--###---+---###---+---###---+---###---+---###---+---###---.
      .-.  .--.                     .-. |      _.|       _.|       _.|       _.|       _.|         |
      |O|--'  |                     |O|-' PB1 |_|   PB2 |_|   PB3 |_|   PB4 |_|   PB5 |_|          |  PB# = Push Button #
      | |     #     Replaced with   | |         '|        '|        '|        '|        '|         |
      |O|---->#     ============>   |O|----------+---------+---------+---------+---------+---###---+
      | |     #                     | |                                                     100K   |
      |O|--   |                     |O|------------------------------------------------------------'
      '-'  '--'                     '-'


At RC Receiver side: (The following sketch is related to this ATtiny85 or Digispark)
===================

                        .---------------.
                        |               |       
                        |        ,------+------.
                        |        |     VDD     |1
                        |        |             +-- LED, Relay, etc...
                        |        |             |
                        |        |             |2
                        |        |             +-- LED, Relay, etc...
                        |        |             |
                        |        |   ATtiny85  |3
                        |        |      or     +-- LED, Relay, etc...
     .------------.     |        |  Digispark  |
     |            |-----'       0|             |4
     |   Channel#1|--------------+             +-- LED, Relay, etc...
     |            |-----.        |             |
     |     RC     |     |        |             |5
     |  RECEIVER  |     |        |             +-- LED, Relay, etc...
     |            |     |        |     GND     |
     |            |-    |        '------+------'
     |   Channel#2|-    |               | 
     |            |-    '---------------' 
     '------------'

Note:
====
- Decoupling capacitors are not drawn.
- This sketch can easily be extended to 8 outputs by using an ATtiny84 which has more pins.
- This sketch cannot work if you are using DigiUSB library as this one monopolizes the "pin change interrupt vector" (which is very time sensitive).
- On the other side, its possible to communicate with exterior world by using <SoftSerial>, a library mainly derived from <SoftwareSerial>, but which
  allow to share the pin change interrupt vector through the <TinyPinChange> library.

================================================================================================*/

/* Channel Declaration */
enum {RC_CHANNEL, RC_CHANNEL_NB}; /* Here, as there is a single channel, we could  used a simple "#define RC_CHANNEL 0" rather an enumeration */

//==============================================================================================
/* Channel Signal of the Receiver */
#define RX_CHANNEL_SIGNAL_PIN  0

//==============================================================================================
/* Declaration of the custom keyboard": the pulse width of the push buttons do not need to be equidistant */
enum {PUSH_BUTTON1, PUSH_BUTTON2, PUSH_BUTTON3, PUSH_BUTTON4, PUSH_BUTTON5, PUSH_BUTTON_NBR};
#define TOLERANCE  40 /* Tolerance  +/- (in microseconds): CAUTION, no overlap allowed between 2 adjacent active areas . active area width = 2 x TOLERANCE (us) */
const KeyMap_t CustomKeyboard[] PROGMEM ={ {CENTER_VALUE_US(1100,TOLERANCE)}, /* PUSH_BUTTON1: +/-40 us */
                                           {CENTER_VALUE_US(1300,TOLERANCE)}, /* PUSH_BUTTON2: +/-40 us */
                                           {CENTER_VALUE_US(1500,TOLERANCE)}, /* PUSH_BUTTON3: +/-40 us */
                                           {CENTER_VALUE_US(1700,TOLERANCE)}, /* PUSH_BUTTON4: +/-40 us */
                                           {CENTER_VALUE_US(1900,TOLERANCE)}, /* PUSH_BUTTON5: +/-40 us */
                                  };

//==============================================================================================
/* Trick: a macro to write a single time the ToggleAction#() function */
#define DECLARE_TOGGLE_ACTION(Idx)                \
void ToggleAction##Idx(void)                      \
{                                                 \
static uint32_t StartMs=millis();                 \
static boolean Etat=HIGH;                         \
                                                  \
/* Since version 2.0 of the <RcSeq> library,   */ \
/* for reactivity reasons, inter-command delay */ \
/* shall be managed in the user sketch.        */ \
  if(millis() - StartMs >= 500UL)                 \
  {                                               \
    StartMs=millis();                             \
    digitalWrite(Idx, Etat);                      \
    Etat=!Etat;                                   \
  }                                               \
}

/* Declaration of the actions using the DECLARE_TOGGLE_ACTION(Idx) macro with Idx = The number of the action and the pin number (The ##Idx will be automatically replaced with the Idx value */
DECLARE_TOGGLE_ACTION(1)
DECLARE_TOGGLE_ACTION(2)
DECLARE_TOGGLE_ACTION(3)
DECLARE_TOGGLE_ACTION(4)
DECLARE_TOGGLE_ACTION(5)

//==============================================================================================
void setup()
{
    RcSeq_Init();
    RcSeq_DeclareSignal(RC_CHANNEL, RX_CHANNEL_SIGNAL_PIN); /* RC_CHANNEL Channel is assigned to RX_CHANNEL_SIGNAL_PIN pin */
    RcSeq_DeclareCustomKeyboard(RC_CHANNEL, RC_CUSTOM_KEYBOARD(CustomKeyboard)); /* The CustomKeyboard map is assigned to the RC_CHANNEL Channel */
    RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, PUSH_BUTTON1, ToggleAction1);pinMode(1,OUTPUT); /* The ToggleAction1 is assigned to the PUSH_BUTTON1 push button #1 */
    RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, PUSH_BUTTON2, ToggleAction2);pinMode(2,OUTPUT); /* The ToggleAction2 is assigned to the PUSH_BUTTON1 push button #2 */
    RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, PUSH_BUTTON3, ToggleAction3);pinMode(3,OUTPUT); /* The ToggleAction3 is assigned to the PUSH_BUTTON1 push button #3 */
    RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, PUSH_BUTTON4, ToggleAction4);pinMode(4,OUTPUT); /* The ToggleAction4 is assigned to the PUSH_BUTTON1 push button #4 */
    RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, PUSH_BUTTON5, ToggleAction5);pinMode(5,OUTPUT); /* The ToggleAction5 is assigned to the PUSH_BUTTON1 push button #5 */
}
//==============================================================================================
void loop()
{
    RcSeq_Refresh(); /* This function performs all the needed job asynchronously (non blocking) */
}
//============================ END OF SKETCH =================================================

