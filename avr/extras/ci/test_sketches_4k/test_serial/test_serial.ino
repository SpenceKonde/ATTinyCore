void setup() {
  Serial.begin(9600);
  Serial.println("raw text");
  Serial.println(F("F() macro"));
  Serial.printHex(GPIOR0);

}

void loop() {
  // put your main code here, to run repeatedly:

}
