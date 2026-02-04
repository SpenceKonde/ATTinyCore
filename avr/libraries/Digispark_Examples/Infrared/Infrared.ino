int irPin=2;
 
void setup()
{
 pinMode(irPin,INPUT);
 pinMode(0,OUTPUT);
 //Serial.begin(9600);
 digitalWrite(0,HIGH);
     //Serial.println("You pressed a button");
     delay(1000);
     digitalWrite(0,LOW);
}
 
void loop()
{
 
  if(pulseIn(irPin,LOW))
  {
     //button pressed 
     delay(100);
     digitalWrite(0,HIGH);
     //Serial.println("You pressed a button");
     delay(1000);
     digitalWrite(0,LOW);
  }
  
}
