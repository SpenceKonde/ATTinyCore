void setup() {                
  // initialize the digital pin as an output.
 

}

// the loop routine runs over and over again forever:
void loop() {
  LEDon(0, 1);
  delay(1000);
  LEDon(0, 2);
  delay(1000);
  LEDon(0, 3);
  delay(1000);
  LEDon(0, 4);
  delay(1000);
  LEDon(1, 0);
  delay(1000);
  LEDon(1, 2);
  delay(1000);
  LEDon(1, 3);
  delay(1000);
  LEDon(1, 4);
  delay(1000);
  LEDon(2, 0);
  delay(1000);
  LEDon(2, 1);
  delay(1000);
  LEDon(2, 3);
  delay(1000);
  LEDon(2, 4);
  delay(1000);
  LEDon(3, 0);
  delay(1000);
  LEDon(3, 1);
  delay(1000);
  LEDon(3, 2);
  delay(1000);
  LEDon(3, 4);
  delay(1000);
  LEDon(4, 0);
  delay(1000);
  LEDon(4, 1);
  delay(1000);
  LEDon(4, 2);
  delay(1000);
  LEDon(4, 3);
  delay(1000);  
}

void LEDon(int vin, int gnd) {
  pinMode(0, INPUT); 
  pinMode(1, INPUT); 
  pinMode(2, INPUT); 
  pinMode(3, INPUT); 
  pinMode(4, INPUT); 
  pinMode(5, INPUT); 

  pinMode(vin, OUTPUT);   
  pinMode(gnd, OUTPUT); 
  digitalWrite(vin, HIGH);
  digitalWrite(gnd, LOW); 
}
