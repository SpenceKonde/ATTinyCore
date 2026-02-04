// SweepNoDelay
// by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html>)
// This sketch can work with ATtiny and Arduino UNO, MEGA, etc...
// This example code is in the public domain.

#include <SoftRcPulseOut.h> 
 
SoftRcPulseOut myservo;  // create servo object to control a servo 
                        // a maximum of eight servo objects can be created 
#define SERVO_PIN         0

#define UP_DIRECTION      +1
#define DOWN_DIRECTION    -1

#define POS_MIN           0
#define POS_MAX           180

int pos = POS_MIN;    // variable to store the servo position 
int step = UP_DIRECTION;

void setup() 
{ 
  myservo.attach(SERVO_PIN);  // attaches the servo on pin defined by SERVO_PIN to the servo object 
  myservo.write(pos);
} 
 
void loop() 
{
  if (SoftRcPulseOut::refresh())    // refresh() returns 1 every 20ms (after pulse update)
  {
    // We arrive here every 20ms
    pos += step;
    if(pos >= POS_MAX) step = DOWN_DIRECTION; //180 degrees reached -> Change direction
    if(pos <= POS_MIN) step = UP_DIRECTION;   //  0 degrees reached -> Change direction
    myservo.write(pos);
  }
}
