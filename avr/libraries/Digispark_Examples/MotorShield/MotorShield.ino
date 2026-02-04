/*

  This example code is in the public domain.
 */

int MotorADir = 2;
int MotorASpeed = 0;
int MotorBDir = 5;
int MotorBSpeed = 1;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the outputs.
  pinMode(MotorADir, OUTPUT);   
  pinMode(MotorASpeed, OUTPUT);
  pinMode(MotorBDir, OUTPUT);
  pinMode(MotorBSpeed, OUTPUT);  
}

// the loop routine runs over and over again forever:
void loop() {
  //both motors forward full speed
  digitalWrite(MotorADir, HIGH);   //forward
  digitalWrite(MotorBDir, HIGH);
  analogWrite(MotorASpeed, 255); //full speed
  analogWrite(MotorBSpeed, 255);
  delay(5000);               // wait for 5 seconds
  //turn in place (if using a skid steer configuration) 
  digitalWrite(MotorADir, HIGH);   //forward
  digitalWrite(MotorBDir, HIGH);
  analogWrite(MotorASpeed, 255);
  analogWrite(MotorBSpeed, 0); //off
  delay(5000);               // wait for 5 seconds
  //turn gradually - the other direction (if using a skid steer configuration) 
  digitalWrite(MotorADir, HIGH);   //forward
  digitalWrite(MotorBDir, HIGH);
  analogWrite(MotorASpeed, 100);
  analogWrite(MotorBSpeed, 255); 
  delay(5000);               // wait for 5 seconds
  //stop 
  digitalWrite(MotorADir, HIGH);   //forward
  digitalWrite(MotorBDir, HIGH);
  analogWrite(MotorASpeed, 0);
  analogWrite(MotorBSpeed, 0); //off  
  //reverse slowly
  digitalWrite(MotorADir, LOW);   //reverse
  digitalWrite(MotorBDir, LOW);
  analogWrite(MotorASpeed, 100);
  analogWrite(MotorBSpeed, 100); 
}
