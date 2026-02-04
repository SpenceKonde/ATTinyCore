//Master Arduino Code:
//SPI MASTER (ARDUINO)
//SPI COMMUNICATION BETWEEN TWO ARDUINO 
//CIRCUIT DIGEST

//edited by Ian Walsh for testing with ATTiny85 SPI (via USI)
//This code should work on any standard Arduinos, and really any
//board that has a hardware SPI interface, that uses the <SPI.h> library


#include<SPI.h>                             //Library for SPI 
#define LED 13           
#define ipbutton 2
int buttonvalue;
int x;
void setup (void)
{
  Serial.begin(115200);                   //Starts Serial Communication at Baud Rate 115200 
  //pinMode(ipbutton,INPUT);                //Sets pin 2 as input 
  pinMode(LED,OUTPUT);                    //Sets pin 7 as Output
  pinMode(SS, OUTPUT);
  SPI.begin();                            //Begins the SPI commnuication
  SPI.setClockDivider(SPI_CLOCK_DIV4);    //Sets clock for SPI communication at 8 (16/8=2Mhz)
  digitalWrite(SS,HIGH);                  // Setting SlaveSelect as HIGH (So master doesnt connnect with slave)
}
void loop(void)
{
  byte Mastersend,Mastereceive;          
  buttonvalue++;   //Reads the status of the pin 2

  //write SS low earlier so slave can get set up?
  digitalWrite(SS, LOW);//Starts communication with Slave connected to master
  delayMicroseconds(5);//time for slave to get ready after interrupt
  //delay(100);

  Mastersend = buttonvalue;                        
  Mastereceive=SPI.transfer(Mastersend); //Send the mastersend value to slave also receives value from slave
  digitalWrite(SS, HIGH);

  //strange serial print order here is to reflect the correlation of received byte to sent byte
  Serial.print("Recieved: 0x");
  Serial.println(Mastereceive, HEX);
  Serial.print("\nSent: 0x");
  Serial.println(Mastersend, HEX); 

  delay(1000);
}