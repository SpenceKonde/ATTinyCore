
void setup() {                
  //If prompted for a pairing code it is 1234, 12345, or 000000
  Serial.begin(9600); 
}

// the loop routine runs over and over again forever:
void loop() {
  if (Serial.available()) {
    Serial.write(Serial.read());
  }

}