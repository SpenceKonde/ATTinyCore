void setup() {
  // put your setup code here, to run once:
  digitalRead(3);
  digitalWrite(3,LOW);
  pinMode(3,INPUT);
  pinMode(3,INPUT_PULLUP);
  pinMode(3,OUTPUT);
  digitalWrite(LED_BUILTIN ,HIGH);
  digitalRead(LED_BUILTIN);
  pinMode(LED_BUILTIN ,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

}
