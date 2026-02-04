// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// Adapted to SoftRcPulseOut library by RC Navy (http://p.loussouarn.free.fr)
// This sketch can work with ATtiny and Arduino UNO, MEGA, etc...
// This example code is in the public domain.

#include <SoftRcPulseOut.h> 
 
SoftRcPulseOut myservo;  // create servo object to control a servo 
                        // a maximum of eight servo objects can be created 
#define SERVO_PIN         3

#define REFRESH_PERIOD_MS 20

#define NOW               1

int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  myservo.attach(SERVO_PIN);  // attaches the servo on pin defined by SERVO_PIN to the servo object 
} 
 
 
void loop() 
{ 
  for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(REFRESH_PERIOD_MS);        // waits 20ms for refresh period 
    SoftRcPulseOut::refresh(NOW);    // generates the servo pulse Now
  } 
  for(pos = 180; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(REFRESH_PERIOD_MS);        // waits 20ms for for refresh period 
    SoftRcPulseOut::refresh(NOW);    // generates the servo pulse Now
  } 
} 
