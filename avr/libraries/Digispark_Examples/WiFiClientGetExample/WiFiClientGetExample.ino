#include <DigiKeyboard.h>
void setup() {
  // put your setup code here, to run once:
   wifiDelay(10000); //wait a good amount of time for moduel to connect to wifi - this script assumes it can connect and doesn't check
   Serial.begin(9600);
  
  /*==========================================================
  =  NOTE: This assumes you have used the wifi module      =
  =  web interface to set it up as a client and 9600 baud  =
  =  See the wifi shield page for more info.               =
  =                                                        =
  =  NOTE YOU MAY NEED TO CHANGE THE begin STATEMENT AND   =
  =  THE MODULED TO 4800 baud IF YOU EXPERIENCE SCRAMBLED  =
  =  OUTPUT                                                =
  =                                                        =
  =  If not useing DigiKeyboard change wifiDelay as noted  =
  ==========================================================*/  
   
   if(wifiConnect(F("digistump.com"))){ //host wrapped in F() to save ram

     if(wifiSendGet(F("digistump.com"),F("/test.txt"))){ //host and path wrapped in F() to save ram
       
       //uncomment one of the three test methods below to try one out, only one will work at a time
       
       //YOU CAN READ IT OUT ONE CHAR AT A TIME AND PRINT IT
       while(!Serial.available()){wifiDelay(10);} //wait for some data
       while(Serial.available()){ //as long as there is data read it and do something with it
         DigiKeyboard.write(Serial.read()); //in this case we just type it out
       }
       
       
       /*
       //OR YOU CAN FIND SOMETHING AND THEN READ AFTER THAT
       //try this with ip.jsontest.com
       //See also: Serial.findUntil, Serial.readBytesUntil, Serial.readStringUntil, Serial.find, etc
       Serial.find(": \""); //find the start of where the ip is shown
       String ip = Serial.readStringUntil('"'); //read until the end of the ip which is the "
       DigiKeyboard.print(ip); //type it out for the demo
       */
   
       /*
       //OR YOU CAN GRAB IT ALL AT ONCE
       //ASSUMING THE BODY ISN'T LARGER THAN AVAILABLE RAME
       String body = Serial.readString(); //read it all to a string
       DigiKeyboard.print(body.trim()); //type it out for the demo - but first trim the whitespace off the start and end
       */
     }
     else{
      //we didn't get a response to our GET - something failed 
      DigiKeyboard.write('1');  
     }
   
   }
   else{
     //we couldn't talk to the wifi module properly - see wifiConnect for possible break points
     DigiKeyboard.write('2');  
   }
}





void loop() {
  // put your main code here, to run repeatedly:

}

void wifiDelay(long time){
  /*
   If you are not using DigiKeyboard in you sketch
   change this by commenting out the DigiKeyboard delay and 
   uncommenting the regular delay
   Also remove the DigiKeyboard.h include at the top
   Note: This sketch will not run with DigiKeyboard in use
   unless connected to a computer
  */
  DigiKeyboard.delay(time); 
  //delay(time);
}


bool wifiConnect(String host){
    wifiDelay(50); 
    Serial.write("+++");
    if (!Serial.find("a")) return false; //error in entering AT mode - try restoring module and then setting settings again
    Serial.write("a");
    if (!wifiWaitForOK()) return false; //error in entering AT mode
    Serial.print(F("AT+NETP=TCP,CLIENT,80,"));
    Serial.print(host);
    Serial.print(F("\r"));
    if (!wifiWaitForOK()) return false; //error in setting host
    Serial.print(F("AT+ENTM\r"));
    if (!wifiWaitForOK()) return false; //error in going back to transparent mode
    wifiDelay(1000); //instead of calling TCPLK to check for a link we just wait a good amount of time for a link, because TCPLK can be buggy
    while(Serial.read()!=-1); //empty the read buffer so we are ready for a clean GET OR POST    
    return true;
}

bool wifiWaitForOK(){
 return Serial.find("k");
}

bool wifiSendGet(String host, String path){
    Serial.print(F("GET "));
    Serial.print(path);
    Serial.println(F(" HTTP/1.1"));
    Serial.print(F("Host: "));
    Serial.println(host);
    Serial.println(F("Cache-Control: no-cache"));
    Serial.println();
    Serial.setTimeout(5000);
    if(!Serial.find("\r\n\r")) //skip the header - if this fail then the GET probably did too
        return false;
    Serial.setTimeout(1000);
    return true;
}

