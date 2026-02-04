void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //open connection to bt/ble module
  botInit();  //setup the pins for the bot
  
}


void loop() {
  
  if(Serial.available()){

    char input = Serial.read();
    
    //route based on input
    
    if(input == 'f'){
      botForward(255);
    }
    else if(input == 'b'){
      botReverse(255);
    }
    else if(input == 'r'){
      botRight(255);
    }
    else if(input == 'l'){
      botLeft(255);
    }
    else if(input == 's'){
      botStop();
    }
  }
  
  
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


