#include "DigiKeyboard.h"

char nmeaString[57];
bool gpsReady = 0;
bool gpsDateTimeReady = 0;

void setup() {
  
  DigiKeyboard.delay(3000);//When this sketch is used with DigiKeyboard as it is here, it tends to need a bit of a delay at the start
  Serial.begin(9600); //Connect to GPS
  DigiKeyboard.delay(1000);//We need a delay before we init the GPS module
  initGPS(); //Send init msgs to GPS
  
}


void loop() {
  
  if(updateGPS()){ //this will return true if it was able to get a valid string from the GPS
    DigiKeyboard.println(nmeaString); //the whole string from the GPS unit

    
    DigiKeyboard.println(getStatus()); //A means we are good to go, V means it is still getting a fix
    if(gpsDateTimeReady){ //date and time are oftenr eady before the rest of the data, so if we are just looking for that we can use it much sooner
     DigiKeyboard.println(getTime());
     DigiKeyboard.println(getDate());
    }
    if(gpsReady){
     DigiKeyboard.println(getLat());
     DigiKeyboard.println(getLon());
    }
  }
  DigiKeyboard.delay(1000);
 
 
}

void initGPS(){
  
  Serial.println(F("$PUBX,40,RMC,1,1,1,0*46")); //turn on RMC msgs
  Serial.println(F("$PUBX,40,GLL,0,0,0,0*5C")); //turn off the rest
  Serial.println(F("$PUBX,40,GGA,0,0,0,0*5A"));
  Serial.println(F("$PUBX,40,GSA,0,0,0,0*4E"));
  Serial.println(F("$PUBX,40,GSV,0,0,0,0*59"));
  Serial.println(F("$PUBX,40,VTG,0,0,0,0*5E"));
  
}




bool updateGPS() {
  
  
  while(Serial.read()!='$');
  while(Serial.read()!=',');
  unsigned int i = 0;
  while(i<56){nmeaString[i] = Serial.read();i++;}
  nmeaString[56] = '\0';
      if(getStatus() == 'A'){ //we're good to go
        gpsReady = 1;
        gpsDateTimeReady = 1;
      }
      else if (getStatus() == 'V'){ //string is valid but gps isn't ready - check if we have date and time at least
        gpsReady = 0;
        gpsDateTimeReady = dateTimeReady();
      }
      else
        return false; //we got an invalid string
        
  return true;
    
}

unsigned int charIndexOf(char* array, char searchChar){
  unsigned int i = 0;
  while(array[i] != '\0'){
   if(array[i] == searchChar)
     return i;
   i++; 
  }
  return false;
}

unsigned int charLastIndexOf(char* array, char searchChar){
  unsigned int i = 0;
  unsigned int l = '\0';
  while(array[i] != '\0'){
   if(array[i] == searchChar)
     l=i;
   i++; 
  }
  if(l=='\0')
    return false;
  else
    return l;
}



long getTime(){
  char time[7];
  memcpy(time,&nmeaString[0],6);
  time[6] = '\0';
  return atol(time);
}


int getHour(){
  char time[3];
  memcpy(time,&nmeaString[0],2);
  time[2] = '\0';
  return atoi(time);
}

int getMin(){
  char time[3];
  memcpy(time,&nmeaString[2],2);
  time[2] = '\0';
  return atoi(time);
}

int getSec(){
  char time[3];
  memcpy(time,&nmeaString[4],2);
  time[2] = '\0';
  return atoi(time);
}

char getStatus(){
  return nmeaString[charIndexOf(nmeaString,',')+1]; //this has to work even if the other data isn't present
}


bool dateTimeReady(){
  if(nmeaString[0] != ',' && nmeaString[18] != ','){
    return true;
  }
  else
    return false;
}

float getLat(){
  char temp[11];
  memcpy( temp, &nmeaString[12], 10 );
  temp[10] = '\0';
  return atof(temp);
}

int getLatDeg(){
  char temp[3];
  memcpy( temp, &nmeaString[12], 2 );
  temp[2] = '\0';
  return atoi(temp);
}

float getLatMin(){
  char temp[9];
  memcpy( temp, &nmeaString[14], 8 );
  temp[8] = '\0';
  return atof(temp);
}

float getLon(){
  char temp[12];
  memcpy( temp, &nmeaString[25], 11 );
  temp[11] = '\0';
  return atof(temp);
}

int getLonDeg(){
  char temp[4];
  memcpy( temp, &nmeaString[25], 3 );
  temp[3] = '\0';
  return atoi(temp);
}

float getLonMin(){
  char temp[9];
  memcpy( temp, &nmeaString[28], 8 );
  temp[8] = '\0';
  return atof(temp);
}

float getKnots(){
  char temp[6];
  memcpy( temp, &nmeaString[39], 5 );
  temp[5] = '\0';
  return atof(temp);
}

float getCourse(){
  char temp[7];
  memcpy( temp, &nmeaString[45], 6 );
  temp[6] = '\0';
  return atof(temp);
}
bool isCoursePresent(){
  if(nmeaString[12] == ',')
    return false;
  else if(charLastIndexOf(nmeaString,',')>53)
    return false;
  else
    return true;
}

long getDate(){
  char date[7];
  if(isCoursePresent()){
    memcpy( date, &nmeaString[52], 4 );
    date[4] = '\0';
    return atol(date)*100+14;
  }
  else{
    if(nmeaString[12] == ',')
      memcpy( date, &nmeaString[18], 6 );
    else
      memcpy( date, &nmeaString[46], 6 );
    date[6] = '\0';
    return atol(date);
  }
}


int getDay(){
  char date[3];
  if(isCoursePresent())
    memcpy( date, &nmeaString[52], 2 );
  else{
    if(nmeaString[12] == ',')
      memcpy( date, &nmeaString[18], 2 );
    else
      memcpy( date, &nmeaString[46], 2 );
  }
  date[2] = '\0';
  return atoi(date);
}
int getMonth(){
  char date[3];
  if(isCoursePresent())
    memcpy( date, &nmeaString[54], 2 );
  else{
    if(nmeaString[12] == ',')
      memcpy( date, &nmeaString[20], 2 );
    else
      memcpy( date, &nmeaString[48], 2 );
  }
  date[2] = '\0';
  return atoi(date);
}
int getYear(){
  if(isCoursePresent())
    return 14; //just assume the year hasn't changed - we could also save it when we have it, but that would take more ram
  else{
    char date[3];
    if(nmeaString[12] == ',')
      memcpy( date, &nmeaString[22], 2 );
    else
      memcpy( date, &nmeaString[50], 2 );
    date[2] = '\0';
    return atoi(date);
  }
  
}

int getFullYear(){
  if(isCoursePresent())
    return 2014; //just assume the year hasn't changed - we could also save it when we have it, but that would take more ram
  else{
    char date[3];
    if(nmeaString[12] == ',')
      memcpy( date, &nmeaString[22], 2 );
    else
      memcpy( date, &nmeaString[50], 2 );
    date[2] = '\0';
    return atoi(date)+2000;
  }
}


char getNS(){
  return nmeaString[23]; 
}

char getEW(){
  return nmeaString[37]; 
}


