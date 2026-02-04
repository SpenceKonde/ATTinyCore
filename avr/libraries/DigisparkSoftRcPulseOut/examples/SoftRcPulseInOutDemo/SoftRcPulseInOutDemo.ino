/*
This sketch demonstrates how to use <SoftRcPulseIn> library to get RC pulses from a receiver and to use <SoftRcPulseOut> library to drive 2 servos.
The first servo will follow the order, and the second one will have a reverted motion.
Please notice this sketch is fully asynchronous: no blocking functions such as delay() or pulseIn() are used.
Tested on arduino UNO, ATtiny84, ATtiny85 and Digispark rev2 (Model A).
RC Navy 2013
http://p.loussouarn.free.fr
*/

#include <SoftRcPulseIn.h>
#include <SoftRcPulseOut.h>
#include <TinyPinChange.h> /* Needed for <SoftRcPulseIn> library */

#define RX_CHANNEL_PIN          2

#define SERVO1_PIN              3
#define SERVO2_PIN              4

#define LED_PIN                 1//1 on Digispark rev2 (Model A), change to pin 0 for Digispark rev1 (Model B), change to 13 for UNO

#define LED_HALF_PERIOD_MS      250

#define PULSE_MAX_PERIOD_MS     30  /* To refresh the servo in case of pulse extinction */

#define NOW                     1

#define NEUTRAL_US              1500 /* Default position in case of no pulse at startup */

enum {NORMAL=0, INVERTED, SERVO_NB}; /* Trick: use an enumeration to declare the index of the servos AND the amount of servos */


SoftRcPulseIn  RxChannelPulse;       /* RxChannelPulse is an objet  of SoftRcPulseIn type */
SoftRcPulseOut ServoMotor[SERVO_NB]; /* Table Creation for 2 objets of SoftRcPulseOut type */

/* Possible values to compute a shifting average fin order to smooth the recieved pulse witdh */
#define AVG_WITH_1_VALUE        0
#define AVG_WITH_2_VALUES       1
#define AVG_WITH_4_VALUES       2
#define AVG_WITH_8_VALUES       3
#define AVG_WITH_16_VALUES      4

#define AVERAGE_LEVEL          AVG_WITH_4_VALUES  /* Choose here the average level among the above listed values */
                                                  /* Higher is the average level, more the system is stable (jitter suppression), but lesser is the reaction */

/* Macro for average */
#define AVERAGE(ValueToAverage,LastReceivedValue,AverageLevelInPowerOf2)  ValueToAverage=(((ValueToAverage)*((1<<(AverageLevelInPowerOf2))-1)+(LastReceivedValue))/(1<<(AverageLevelInPowerOf2)))

/* Variables */
uint32_t LedStartMs=millis();
uint32_t RxPulseStartMs=millis();
boolean  LedState=HIGH;

void setup()
{
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny167__)
  Serial.begin(9600);
  Serial.print("SoftRcPulseIn library V");Serial.print(SoftRcPulseIn::LibTextVersionRevision());Serial.print(" demo"); /* For arduino UNO which has an hardware UART, display the library version in the console */
#endif
  RxChannelPulse.attach(RX_CHANNEL_PIN);
  ServoMotor[NORMAL].attach(SERVO1_PIN);   /* enumeration is used a index for the ServoMotor[] table */
  ServoMotor[INVERTED].attach(SERVO2_PIN); /* enumeration is used a index for the ServoMotor[]table */
  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
static uint16_t Width_us=NEUTRAL_US; /* Static to keep the value at the next loop */
  
  /* Receiver pulse acquisition and command of 2 servos, one in the direct direction, one in the inverted direction */
  if(RxChannelPulse.available())
  {
    AVERAGE(Width_us,RxChannelPulse.width_us(),AVERAGE_LEVEL);
    ServoMotor[NORMAL].write_us(Width_us);                  /* Direct Signal */
    ServoMotor[INVERTED].write_us((NEUTRAL_US*2)-Width_us); /* Inverted Signal */
    SoftRcPulseOut::refresh(NOW); /* NOW argument (=1) allows to synchronize outgoing pulses with incoming pulses */
    RxPulseStartMs=millis();      /* Restart the Chrono for Pulse */
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny167__)
    Serial.print("Pulse=");Serial.println(Width_us); /* For arduino UNO which has an hardware UART, display the library version in the console */
#endif
  }
  else
  {
    /* Check for pulse extinction */
    if(millis()-RxPulseStartMs>=PULSE_MAX_PERIOD_MS)
    {
      /* Refresh the servos with the last known position in order to avoid "flabby" servos */
      SoftRcPulseOut::refresh(NOW); /* Immediate refresh of outgoing pulses */
      RxPulseStartMs=millis(); /* Restart the Chrono for Pulse */
    }
  }
  
  /* Blink LED Management */
  if(millis()-LedStartMs>=LED_HALF_PERIOD_MS)
  {
    digitalWrite(LED_PIN, LedState);
    LedState=!LedState;  /* At the next loop, if the half period is elapsed, the LED state will be inverted */
    LedStartMs=millis(); /* Restart the Chrono for the LED */
  }

}
