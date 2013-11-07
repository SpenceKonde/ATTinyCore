/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | This sketch demonstrates how to access the key list and to
|| | find all the currently pressed (active) keys.
|| #
*/
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
}
  
void loop(){
    
  if (kpd.getKeys())
  {
    int keyPosition = kpd.findInList('#');
    Serial.print("# key is ");
    switch (kpd.key[keyPosition].kstate) {
        case IDLE:
            Serial.println("IDLE");
            Serial.println("");
            break;
        case PRESSED:
            Serial.println("PRESSED");
            Serial.println("");
            break;
        case HOLD:
            Serial.println("HOLD");
            Serial.println("");
            break;
        case RELEASED:
            Serial.println("RELEASED");
            Serial.println("");
            break;
    }
    

  }
}  // End loop




/*    for (int i=0; i<LIST_MAX; i++)   // Scan the entire list for any active keys.
    {
      if (kpd.key[i].kchar)    // Check for an active key.
      {
        switch (kpd.key[i].kstate) {
            case HOLD:
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.print(" is being HELD and the state ");
                if (!kpd.key[i].stateChanged)
                    Serial.println("has not changed.");
                else
                    Serial.println("has changed.");
                break;
            case PRESSED:
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.print(" is PRESSED and the state ");
                if (!kpd.key[i].stateChanged)
                    Serial.println("has not changed.");
                else
                    Serial.println("has changed.");
                break;
            case RELEASED:
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.print(" has been RELEASED and the state ");
                if (!kpd.key[i].stateChanged)
                    Serial.println("has not changed.");
                else
                    Serial.println("has changed.");
                break;
            default:
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.print(" is IDLE and the state ");
                if (!kpd.key[i].stateChanged)
                    Serial.println("has not changed.");
                else
                    Serial.println("has changed.");
        }
      }
    }
*/    

