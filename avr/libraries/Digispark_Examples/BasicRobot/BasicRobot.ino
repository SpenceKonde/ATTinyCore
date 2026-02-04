void setup() {
  // put your setup code here, to run once:
  botInit();
}
 
 
void loop() {
  // put your main code here, to run repeatedly:
  botForward(255); //speed can be any value from 0 (stopped) to 255 (full)
  delay(5000);
  botReverse(255);
  delay(5000);
  botRight(255);
  delay(5000);
  botHardRight(255);
  delay(5000);
  botLeft(255);
  delay(5000);
  botHardLeft(255);
  delay(5000);
  botStop();
  delay(5000);
}
 
void botForward(int botSpeed){
 digitalWrite(2, HIGH); 
 digitalWrite(5, HIGH);
 analogWrite(0, 255 - botSpeed);
 analogWrite(1, 255 - botSpeed);
}
 
void botReverse(int botSpeed){
 digitalWrite(2, LOW); 
 digitalWrite(5, LOW);
 analogWrite(0, botSpeed);
 analogWrite(1, botSpeed);
}
 
void botRight(int botSpeed){
 digitalWrite(2, LOW); 
 digitalWrite(5, HIGH);
 analogWrite(0, 0);
 analogWrite(1, 255 - botSpeed);
}
 
void botHardRight(int botSpeed){
 digitalWrite(2, LOW); 
 digitalWrite(5, HIGH);
 analogWrite(0, botSpeed);
 analogWrite(1, 255 - botSpeed);
}
 
void botLeft(int botSpeed){
 digitalWrite(2, HIGH); 
 digitalWrite(5, LOW);
 analogWrite(0, 255 - botSpeed);
 analogWrite(1, 0);
}
 
void botHardLeft(int botSpeed){
 digitalWrite(2, HIGH); 
 digitalWrite(5, LOW);
 analogWrite(0, 255 - botSpeed);
 analogWrite(1, botSpeed);
}
 
void botStop(){
 digitalWrite(2,LOW); 
 digitalWrite(5,LOW);
 analogWrite(0,0);
 analogWrite(1,0);
}
 
void botInit(){
 pinMode(0,OUTPUT);
 pinMode(1,OUTPUT);
 pinMode(2,OUTPUT);
 pinMode(5,OUTPUT);
}