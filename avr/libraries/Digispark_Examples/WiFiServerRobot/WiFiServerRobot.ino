void setup() {
  // put your setup code here, to run once:
  delay(10000);//wait for wifi to connect
  Serial.begin(9600); //open connection to wifi module
/*==========================================================
  =  NOTE: This assumes you have used the wifi module      =
  =  web interface to set it up as a server and 9600 baud  =
  =  See the wifi shield page for more info.               =
  ==========================================================
  
  Goto: http://[WIFI IP ADDRESS]:[WIFI SERVER PORT]/forward to make the bot go forward
  ie. http://192.168.0.123:8899/forward
  change the forward to back, right, left, or stop to do those things instead
*/  
  botInit();  //setup the pins for the bot
  
}


void loop() {
  
  if(serverRequest()){
    //new request
    //find the path requested
    String path = getRequestPath();
    
    //route based on path
    
    ///wrap responses and other strings in F() to save ram
    
    if(path == F("/forward")){
      botForward(255);
      sendOK();
    }
    else if(path == F("/back")){
      botReverse(255);
      sendOK();
    }
    else if(path == F("/right")){
      botRight(255);
      sendOK();
    }
    else if(path == F("/left")){
      botLeft(255);
      sendOK();
    }
    else if(path == F("/stop")){
      botStop();
      sendOK();
    }
    else{
      //it doesn't like sending big responses as one chunk - so you can use these functions to break it up
      sendResponseStart();
      sendResponseChunk(F("<B>BOTLY - your RESTful Bot!</B>"));
      sendResponseChunk(F("<BR><A HREF=\"/forward\">Forward</A>"));
      sendResponseChunk(F("<BR><A HREF=\"/back\">Back</A>"));
      sendResponseChunk(F("<BR><A HREF=\"/right\">Right</A>"));
      sendResponseChunk(F("<BR><A HREF=\"/left\">Left</A>"));
      sendResponseChunk(F("<BR><A HREF=\"/stop\">Stop</A>"));
      sendResponseEnd();
    }
  }
  
  
}

void sendOK(){
 sendResponse(F("OK")); 
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


bool serverRequest(){
  if(Serial.available()>4){
    return Serial.find("GET ");
  }
  return false;
}

String getRequestPath(){
    String path = Serial.readStringUntil(' ');
    while(Serial.read() != -1); //clear read buffer
    return path;
}

void sendResponse(String response){
    sendResponseStart();
    sendResponseChunk(response);
    sendResponseEnd();
}


void sendResponseStart(){
    //sends a chunked response
    Serial.println(F("HTTP/1.1 200 OK"));
    Serial.println(F("Content-Type: text/html"));
    Serial.println(F("Connection: close"));
    Serial.println(F("Transfer-Encoding: chunked"));
    Serial.println();
}
void sendResponseChunk(String response){

    Serial.println(response.length()+2,HEX);
    Serial.println(response);
    Serial.println();

}
void sendResponseEnd(){
    Serial.println(F("0"));
    Serial.println();
}
  
