
// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(0, OUTPUT); //LED on Model B
  pinMode(1, OUTPUT); //LED on Model A  or Pro
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(1, HIGH);
  delay(1000);               // wait for a second
  digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(1, LOW); 
  delay(1000);               // wait for a second
}
