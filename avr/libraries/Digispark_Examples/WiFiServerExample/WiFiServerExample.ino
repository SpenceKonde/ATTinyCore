void setup() {
  // put your setup code here, to run once:
  pinMode(1,OUTPUT); //use onboard LED as output
  delay(10000);//wait for wifi to connect
  Serial.begin(9600); //open connection to wifi module
/*==========================================================
  =  NOTE: This assumes you have used the wifi module      =
  =  web interface to set it up as a server and 9600 baud  =
  =  See the wifi shield page for more info.               =
  ==========================================================
  
  Goto: http://[WIFI IP ADDRESS]:[WIFI SERVER PORT]/ to see the response
  ie. http://192.168.0.123:8899/

*/  
}


void loop() {
  
  if(serverRequest()){
    //new request
    //find the path requested
    String path = getRequestPath();
    
    //route based on path
    
    ///wrap responses and other strings in F() to save ram
    
    if(path == F("/on")){
      digitalWrite(1,HIGH);
      sendResponse(F("LED ON<BR><A HREF=\"/off\">LED OFF</A>"));
    }
    else if(path == F("/off")){
      digitalWrite(1,LOW);
      sendResponse(F("LED OFF<BR><A HREF=\"/on\">LED ON</A>"));
    }
    else{
      sendResponse(F("<B>WELCOME</B><BR><A HREF=\"/on\">LED ON</A><BR><A HREF=\"/off\">LED OFF</A>"));
    }
  }
  
  
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
