/* ATtiny85 as an I2C Master   Ex3        BroHogan                         1/22/11
 * I2C master reading DS1621 temperature sensor & DS1307 RTC. Display to I2C GPIO LED.
 * SETUP:
 * ATtiny Pin 1 = (RESET) N/U                      ATtiny Pin 2 = (D3) N/U
 * ATtiny Pin 3 = (D4) to LED1                     ATtiny Pin 4 = GND
 * ATtiny Pin 5 = SDA on all devices               ATtiny Pin 6 = (D1) to LED2
 * ATtiny Pin 7 = SCK on all devices               ATtiny Pin 8 = VCC (2.7-5.5V)
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 * DS1621 wired per data sheet. This ex assumes A0-A2 are set LOW for an addeess of 0x48
 * DS1307 wired per data sheet. This ex assumes A0-A2 are set LOW for an addeess of 0x68
 * PCA8574A GPIO was used wired per instructions in "info" folder in the LiquidCrystal_I2C lib.
 * This ex assumes A0-A2 are set HIGH for an addeess of 0x3F
 * LiquidCrystal_I2C lib was modified for ATtiny - on Playground with TinyWireM lib.
 * TinyWireM USAGE & CREDITS: - see TinyWireM.h
 */

//#define DEBUG
#include <TinyWireM.h>                  // I2C Master lib for ATTinys which use USI
#include <LiquidCrystal_I2C.h>          // for LCD w/ GPIO MODIFIED for the ATtiny85

#define GPIO_ADDR     0x3F              // (PCA8574A A0-A2 @5V) typ. A0-A3 Gnd 0x20 / 0x38 for A
#define DS1307_ADDR   0x68              // I2C real time clock
#define DS1621_ADDR   0x48              // 7 bit I2C address for DS1621 temperature sensor
#define LED1_PIN         4              // ATtiny Pin 3
#define LED2_PIN         1              // ATtiny Pin 6
//#define HR24         true

int tempC = 0;                          // holds temp in C
int tempF = 0;                          // holds temp in F
byte seconds,minutes,hours,day_of_week,days,months,years,PM,hour12,DST;
char timeString[10];                    // HH:MM 12 Hr. no AM/PM or 24 Hr (based on param)
char dateString[10];                    // MM/DD or DD/MM (based on param)- no year
bool HR24;                              // 12/24 Hr Time and date

LiquidCrystal_I2C lcd(GPIO_ADDR,16,2);  // set address & 16 chars / 2 lines


void setup(){
#ifdef DEBUG
  pinMode(LED1_PIN,OUTPUT);
  pinMode(LED2_PIN,OUTPUT);
  Blink(LED1_PIN,3);                    // show it's alive
#endif
  TinyWireM.begin();                    // initialize I2C lib
  Init_Temp();                          // Setup DS1621
  lcd.init();                           // initialize the lcd 
  lcd.backlight();                      // Print a message to the LCD.
  lcd.print("Hello, Temp!");
  delay (2000);
}


void loop(){
  Get_Temp();                           // read current temperature
  Get_Time();                           // read current time
  lcd.clear();                          // display it
  lcd.print("C");
  lcd.print((char)223);
  lcd.print(": ");
  lcd.print(tempC,DEC);
  lcd.setCursor(9,0);
  lcd.print("F");
  lcd.print((char)223);
  lcd.print(": ");
  lcd.print(tempF,DEC);
  lcd.setCursor(0,1);
  lcd.print(timeString);
  lcd.setCursor(9,1);
  lcd.print(dateString);
#ifdef DEBUG
  Blink(LED1_PIN,tempC/10);             // blink 10's of temperature on LED 1
  delay (1000);
  Blink(LED2_PIN,tempC%10);             // blink 1's of temperature on LED 2
#endif
  HR24 = ! HR24;                        // flip the format
  delay (4000);                         // wait a few sec before next reading
}


void Init_Temp(){ // Setup the DS1621 for one-shot mode
  TinyWireM.beginTransmission(DS1621_ADDR);
  TinyWireM.send(0xAC);                 // Access Command Register
  TinyWireM.send(B00000001);            // Using one-shot mode for battery savings
  //TinyWireM.send(B00000000);          // if setting continious mode for fast reads
  TinyWireM.endTransmission();          // Send to the slave
}


void Get_Temp(){  // Get the temperature from a DS1621
  TinyWireM.beginTransmission(DS1621_ADDR);
  TinyWireM.send(0xEE);                 // if one-shot, start conversions now
  TinyWireM.endTransmission();          // Send 1 byte to the slave
  delay(750);                           // if one-shot, must wait ~750 ms for conversion
  TinyWireM.beginTransmission(DS1621_ADDR);
  TinyWireM.send(0xAA);                 // read temperature (for either mode)
  TinyWireM.endTransmission();          // Send 1 byte to the slave
  TinyWireM.requestFrom(DS1621_ADDR,1); // Request 1 byte from slave
  tempC = TinyWireM.receive();          // get the temperature
  tempF = tempC * 9 / 5 + 32;           // convert to Fahrenheit 
}


void Get_Time(){         // get the time and date from the DS1307 chip
  byte wireRet = 0;
  memset(timeString,0,sizeof(timeString));  // initialize the strings
  memset(dateString,0,sizeof(dateString));

  TinyWireM.beginTransmission(DS1307_ADDR); // reset DS1307 register pointer 
  TinyWireM.send(0); 
  wireRet = TinyWireM.endTransmission(); 
  if (wireRet) {                            // report any send esrrors
    lcd.clear();
    lcd.print("SendError: ");
    lcd.print(wireRet,DEC);
    delay(1500);
  }
  wireRet = TinyWireM.requestFrom(DS1307_ADDR, 7);    // request 7 bytes from DS1307
  if (wireRet) {                                      // report any receive esrrors
    lcd.clear();
    lcd.print("RcveError: ");
    lcd.print(wireRet,DEC);
    delay(1500);
  }
#ifdef DEBUG
  lcd.clear();
  lcd.print("Before Reads: ");
  lcd.print(TinyWireM.available(),DEC);     // testing TinyWireM.available()
  delay(1500);
#endif    
  seconds = bcdToDec(TinyWireM.receive());  // handle the 7 bytes received
  minutes = bcdToDec(TinyWireM.receive());      
  hours = bcdToDec(TinyWireM.receive());        
  day_of_week = TinyWireM.receive();    
  days = bcdToDec(TinyWireM.receive());          
  months = bcdToDec(TinyWireM.receive());      
  years = bcdToDec(TinyWireM.receive());  
#ifdef DEBUG
  lcd.clear();
  lcd.print("After Reads: ");
  lcd.print(TinyWireM.available(),DEC);     // testing TinyWireM.available()
  delay(1500);
#endif    
  // deal with AM/PM global and 12 hour clock 
  if (hours >= 12) PM = true;
  else PM = false;
  if (hours > 12)hour12 = hours - 12;
  else hour12 = hours;
  if (hours == 0) hour12 = 12;

  // make time string
  if (HR24) AppendToString (hours,timeString);   // add 24 hour time to string
  else AppendToString (hour12,timeString);       // add 12 hour time to string
  strcat(timeString,":");
  if (minutes < 10) strcat(timeString,"0");
  AppendToString (minutes,timeString);           // add MINUTES to string
  if (!HR24){
    if (hours >= 12) strcat(timeString," PM");   // deal with AM/PM
    else strcat(timeString," AM");
  }
  // make date string
  if (HR24)AppendToString (days,dateString);     // add DAY to string
  else AppendToString (months,dateString);       // add MONTH to string
  strcat(dateString,"/");
  if (HR24)AppendToString (months,dateString);   // add MONTH to string
  else AppendToString (days,dateString);         // add DAY to string
  strcat(dateString,"/");
  if (years < 10) strcat(dateString,"0");
  AppendToString (years,dateString);             // add YEAR to string
}


void AppendToString (byte bValue, char *pString){ // appends a byte to string passed
  char tempStr[6];
  memset(tempStr,'\0',sizeof(tempStr));
  itoa(bValue,tempStr,10);
  strcat(pString,tempStr);
}


byte bcdToDec(byte val) {               // Convert binary coded decimal to normal decimal numbers
  return ((val / 16 * 10) + (val % 16));
}


#ifdef DEBUG
void Blink(byte led, byte times){ // poor man's GUI
  for (byte i=0; i< times; i++){
    digitalWrite(led,HIGH);
    delay (400);
    digitalWrite(led,LOW);
    delay (175);
  }
}
#endif





