#include <DigisparkRGB.h>
/*
 Digispark RGB
 
 This example shows how to use soft PWM to fade 3 colors.
 Note: This is only necessary for PB2 (pin 2) - Blue, as Red (pin 0) and Green (pin 1) as well as pin 4 support the standard Arduino analogWrite() function.
 
 This example code is in the public domain.
 */
byte RED = 0;
byte BLUE = 2;
byte GREEN = 1;
byte COLORS[] = {RED, BLUE, GREEN};

// the setup routine runs once when you press reset:
void setup() { 
  DigisparkRGBBegin();
} 


void loop() {
  //direction: up = true, down = false
  boolean dir = true;
  int i = 0;
  
  while(1) {
    fade(COLORS[i%3], dir);
    i++;
    dir = !dir;
  }
}

void fade(byte Led, boolean dir) {
  int i;
  
  //if fading up
  if (dir) {
    for (i = 0; i < 256; i++) {
      DigisparkRGB(Led, i);
      DigisparkRGBDelay(25);//1);
    }
  } else {
    for (i = 255; i >= 0; i--)  {
      DigisparkRGB(Led, i);
      DigisparkRGBDelay(25);//1);
    }
  }
}




