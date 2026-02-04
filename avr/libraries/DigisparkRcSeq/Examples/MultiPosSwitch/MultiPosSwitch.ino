#include <RcSeq.h>
#include <TinyPinChange.h>
#include <SoftRcPulseIn.h>

/*
IMPORTANT:
For this sketch to compile, RC_SEQ_WITH_SOFT_RC_PULSE_IN_SUPPORT and RC_SEQ_WITH_SHORT_ACTION_SUPPORT shall be defined
in PathOfTheLibraries/(Digispark)RcSeq/RcSeq.h and RC_SEQ_WITH_SOFT_RC_PULSE_OUT_SUPPORT shall be commented.

  This sketch demonstrates how to easily use a 3 positions switch on a channel of a RC Transmitter with <RcSeq> library.
  1) If the switch is at the MIDDLE position, a LED will be OFF
  2) If the switch is at the DOWN   position, a LED will be ON
  2) If the switch is at the UP     position, a LED will blink

  This sketch can be extended to a rotactor (up to 8 positions)
  
  RC Navy (2013)
  http://p.loussouarn.free.fr


                   WIRING A TRANSMITTER SIDE                                     WIRING AT RECEIVER SIDE
                   
                            .------+-------------> +
                            |      |
              /             |      #                                               .-------------------------.
              |          UP o      # 4.7K                                          |                         |
              |              \     #                                               |     ARDUINO or ATTINY   |
 3 positions  |               \    |                                               |                         |     R
   switch    <    MIDDLE o     o---+-------------> To Tx Channel       Rx Channel->|RC_CHANNEL_PIN   LED_PIN >----###---|>|----|GND
              |                C   |                                               |                         |          LED
              |                    #                                               |          Sketch         |
              |        DOWN o      # 4.7K                                          '-------------------------'    External or
              \             |      #                                                                              Built-in LED
                            |      |
                            '------+-------------> -
 
     .------------.------------------------.--------------.
     | Switch Pos | Pulse Width Range (us) |    Action    |
     +------------+------------------------+--------------+
     |     UP     |      1000 -> 1270      | LED Blinking |
     +------------+------------------------+--------------+
     |   MIDDLE   |      1360 -> 1630      | LED OFF      |
     +------------+------------------------+--------------+
     |    DOWN    |      1720 -> 1990      | LED ON       |
     '------------'------------------------'--------------'
     
     Note:
     ====
     <RcSeq> computes automatically the valid pulse width range for each position of the switch.
*/


/* Channel Declaration */
enum {RC_CHANNEL=0, RC_CHANNEL_NB}; /* Here, as there is a single channel, we could  used a simple "#define RC_CHANNEL 0" rather an enumeration */

#define RC_CHANNEL_PIN     0 // Choose here the pin
#define LED_PIN            1 // Choose here the pin

enum {SW_POS_DOWN=0, SW_POS_MIDDLE, SW_POS_UP, SW_POS_NB}; /* Switch has 3 positions: Down, Middle and Up (For a rotactor with more positions, add positions here) */

boolean BlinkCmd=false;
boolean LedState=false;


void setup()
{
  RcSeq_Init();
  
  RcSeq_DeclareSignal(RC_CHANNEL, RC_CHANNEL_PIN); /* RC_CHANNEL Channel is assigned to RC_CHANNEL_PIN pin */
  
  RcSeq_DeclareMultiPosSwitch(RC_CHANNEL, 1000, 2000, SW_POS_NB); /* Tells to <RcSeq> that the RC_CHANNEL channel has SW_POS_NB positions distributed between 1000 and 2000 us */
  
  RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, SW_POS_DOWN,   ActionSwPosDown);   /* Action assigned to DOWN   position */
  RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, SW_POS_MIDDLE, ActionSwPosMiddle); /* Action assigned to MIDDLE position */
  RcSeq_DeclareCommandAndShortAction(RC_CHANNEL, SW_POS_UP,     ActionSwPosUp);     /* Action assigned to UP     position */

  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
static uint32_t StartMs=millis();

  /* Refresh RcSeq (mandatory) */
  RcSeq_Refresh();
  
  /* Blink Management */
  if( (BlinkCmd==true) && (millis() - StartMs >= 250UL) )
  {
    StartMs=millis();
    LedState=!LedState;
    digitalWrite(LED_PIN, LedState);
  }
  
}

void ActionSwPosUp() /* This function will be called when the switch is in UP position */
{
  BlinkCmd=true;
}

void ActionSwPosMiddle() /* This function will be called when the switch is in MIDDLE position */
{
  BlinkCmd=false;
  LedState=false;
  digitalWrite(LED_PIN, LedState);
}

void ActionSwPosDown() /* This function will be called when the switch is in DOWN position */
{
  BlinkCmd=false;
  LedState=true;
  digitalWrite(LED_PIN, LedState);
}

