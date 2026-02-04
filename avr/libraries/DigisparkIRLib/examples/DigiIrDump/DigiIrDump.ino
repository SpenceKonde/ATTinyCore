#include <IRLib.h>   // In {path_of_installation}/Digispark-Arduino-1.0.x/libraries/DigisparkIRLib/IRLib.h, set MY_PROTOCOL to NEC, SONY, RC5 to find the one used by your own IR Remote Control
#include <DigiUSB.h> // In {path_of_installation}/Digispark-Arduino-1.0.x/libraries/DigisparkUSB/DigiUSB.h, RING_BUFFER_SIZE shall be set to 32
/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013

                http://p.loussouarn.free.fr

         *************************************************
         *      Optimized <IRLib> library Dump Demo      *
         *************************************************

This sketch allows you to discover the protocol used by your own IR Remote Control and the code of each key when pressed.
You will see the decoded key codes in the DigiUSB console.

IMPORTANT:
=========
- In {path_of_installation}/Digispark-Arduino-1.0.x/libraries/DigisparkIRLib/IRLib.h, set MY_PROTOCOL to NEC, SONY, RC5 to find the protocol used by your own IR Remote Control
- In {path_of_installation}/Digispark-Arduino-1.0.x/libraries/DigisparkUSB/DigiUSB.h, RING_BUFFER_SIZE shall be set to 32

 Sensor wiring: (Warning: the wiring may vary depending of the model of IR sensor)
 =============
        .-------.
        |  ___  |
        | /   \ | InfraRed
        | \___/ |  Sensor
        |       |
        '+--+--+'
         |  |  |      100
 P5 <----'  |  '--+---###--- +5V
            |     |
            |   '==='4.7uF
            |     |
            '--+--'
               |
              GND

*/

#define LED_PIN    1
#define IR_RX_PIN  5

#define BUTTON_OFF           0xF740BF //Set here the OFF code for the built-in LED when determined
#define BUTTON_ON            0xF7C03F //Set here the ON  code for the built-in LED when determined

IRrecv   My_Receiver(IR_RX_PIN);//Receive on pin IR_RX_PIN
IRdecode My_Decoder; 

void setup() 
{ 
  My_Receiver.enableIRIn(); // Start the receiver
  pinMode(LED_PIN, OUTPUT);
  DigiUSB.begin();
} 

void loop() 
{ 
  if(My_Receiver.GetResults(&My_Decoder))
  {
    My_Decoder.decode();
    switch(My_Decoder.value)
    {
    case BUTTON_OFF:
      digitalWrite(LED_PIN, LOW);
      break;
    case BUTTON_ON:
      digitalWrite(LED_PIN, HIGH);
      break;
    default:break;
    }
    My_Receiver.resume();
    DigiUSB.println(My_Decoder.value, HEX);
  }
  if(DigiUSB.available())
  {
    DigiUSB.read();
  }
  DigiUSB.refresh();
}

