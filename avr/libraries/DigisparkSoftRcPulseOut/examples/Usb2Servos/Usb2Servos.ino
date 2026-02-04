// This sketch demonstrates how to command 2 servos through the USB of the Digispark.
// It uses:
// - <SoftRcPulseOut> library to easily generates the RC pulses for the servos.
// - <DigiUSB> library to communicate with the PC
// By RC Navy (http://p.loussouarn.free.fr)

// The command (issued in the DigiUSB Monitor or the digiterm) is:
// S=P with:
// S=A for ServoA and S=B for ServoB
// P=Position number x 20° (Possible positions are from 0 to 9 which correspond to from 0° to 180°)
// Ex:
// A=7 sets Servo1 at 7 x 20 =140°
// B=3 sets Servo2 at 3 x 20 =60°
// Once the servo selected, just type the value between 0 and 9
// Please, note this sketch is derived from the SerialServo example of <SoftwareServo> library.

#include <DigiUSB.h>
#include <SoftRcPulseOut.h>

#define LED_PIN       1 /* Builtin Led on Rev2 ModelA Digispark */
#define SERVO_A_PIN   2
/* /!\ Do not use Pin 3 (used by USB) /!\ */
/* /!\ Do not use Pin 4 (used by USB) /!\ */
#define SERVO_B_PIN   5

SoftRcPulseOut ServoA;
SoftRcPulseOut ServoB;


void setup()
{
  pinMode(LED_PIN,OUTPUT);
  ServoA.attach(SERVO_A_PIN);
  ServoB.attach(SERVO_B_PIN);
  DigiUSB.begin();
  DigiUSB.println(" Ready");
}

void loop()
{
  static int value = 0;
  static char CurrentServo = 0;

  if ( DigiUSB.available()) {
    char ch = DigiUSB.read();
    switch(ch) {
      case 'A':
        CurrentServo='A';
        digitalWrite(LED_PIN,LOW);
        break;
      case 'B':
        CurrentServo='B';
        digitalWrite(LED_PIN,HIGH);
        break;
      case '0' ... '9':
        value=(ch-'0')*20;
        if (CurrentServo=='A')
        {
          ServoA.write(value);
        }
        else if (CurrentServo=='B')
        {
          ServoB.write(value);
        }
        break;
    }
  }
  DigiUSB.refresh();
  SoftRcPulseOut::refresh();
  /*
    Put here your non-blocking code
  */
}
