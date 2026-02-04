// This SoftwareServo library example sketch was initially delivered without any comments.
// Below my own comments for SoftRcPulseOut library: by RC Navy (http://p.loussouarn.free.fr)
// Controlling the position of 2 servos using the Arduino built-in hardware UART (Arduino Serial object).
// This sketch do NOT work with an ATtinyX4 and ATtinyX5 since they do not have a built-in harware UART (no Arduino Serial object).

// The command (issued in the Arduino Serial Console or in a Terminal) is:
// S=P with:
// S=A for Servo1 and S=B for Servo2
// P=Position number x 20° (Possible positions are from 0 to 9 which correspond to from 0° to 180°)
// Ex:
// A=7 sets Servo1 at 7 x 20 =140°
// B=3 sets Servo2 at 3 x 20 =60°

#include <SoftRcPulseOut.h>

SoftRcPulseOut servo1;
SoftRcPulseOut servo2;

void setup()
{
  pinMode(13,OUTPUT);
  servo1.attach(2);
  servo1.setMaximumPulse(2200);
  servo2.attach(4);
  servo2.setMaximumPulse(2200);
  Serial.begin(9600);
  Serial.print("Ready");
}

void loop()
{
  static int value = 0;
  static char CurrentServo = 0;

  if ( Serial.available()) {
    char ch = Serial.read();
    switch(ch) {
      case 'A':
        CurrentServo='A';
        digitalWrite(13,LOW);
        break;
      case 'B':
        CurrentServo='B';
        digitalWrite(13,HIGH);
        break;
      case '0' ... '9':
        value=(ch-'0')*20;
        if (CurrentServo=='A')
        {
          servo1.write(value);
        }
        else if (CurrentServo=='B')
        {
          servo2.write(value);
        }
        break;
    }
  }
  SoftRcPulseOut::refresh();
}
