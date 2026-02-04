/* USB LCD */

//#define DEBUG
#include <TinyWireM.h>                  // I2C Master lib for ATTinys which use USI - comment this out to use with standard arduinos
#include <LiquidCrystal_I2C.h>          // for LCD w/ GPIO MODIFIED for the ATtiny85
#include <DigiUSB.h>

#define GPIO_ADDR     0x27             // (PCA8574A A0-A2 @5V) typ. A0-A3 Gnd 0x20 / 0x38 for A - 0x27 is the address of the Digispark LCD modules.
int currentLine = 0;
boolean clearOnNext = 0;
boolean backlight = 1;

LiquidCrystal_I2C lcd(GPIO_ADDR,16,2);  // set address & 16 chars / 2 lines


void setup(){
  DigiUSB.begin();
  TinyWireM.begin();                    // initialize I2C lib - comment this out to use with standard arduinos
  lcd.init();                           // initialize the lcd 
  lcd.backlight();                      // Print a message to the LCD.
  lcd.setCursor(0, currentLine);
}


void get_input() {

  int lastRead;
  // when there are no characters to read, or the character isn't a newline
  while (1==1) {
    if(DigiUSB.available()){
      //something to read
      lastRead = DigiUSB.read();
      if(lastRead == '\n'){
        
        
        if(currentLine > 0)
          currentLine = 0;
        else
          currentLine = 1;
        
        clearOnNext = 1;
        
        lcd.setCursor(0, currentLine);
        
      }
      else if(lastRead == 172){ //not sign "¬" send it with the send program to toggle the backlight
        if(backlight){
          lcd.noBacklight();
          backlight = 0;
        }
        else{
          lcd.backlight(); 
          backlight = 1;
        }
        DigiUSB.read(); //read to nothing to get rid of newline that should come after it

      }
      else{
        if(clearOnNext){
          lcd.print("                "); //clear a single line
          lcd.setCursor(0, currentLine);
          clearOnNext=0;
        }
        lcd.print(char(lastRead));
      }
      
      
      
    }
    // refresh the usb port
    DigiUSB.refresh();
    delay(10);

  }

}


void loop(){
  get_input();
}