void setup() {
  Serial.begin(9600);
  Serial.println("raw text");
  Serial.println(F("F() macro"));
  Serial.printHex(EEDR);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    Serial.write(Serial.read());
  }
}
