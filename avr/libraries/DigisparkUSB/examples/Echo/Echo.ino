#include <DigiUSB.h>

void setup() {
  DigiUSB.begin();
}

void get_input() {
  int lastRead;
  // when there are no characters to read, or the character isn't a newline
  while (true) { // loop forever
    if (DigiUSB.available()) {
      // something to read
      lastRead = DigiUSB.read();
      DigiUSB.write(lastRead);
      
      if (lastRead == '\n') {
        break; // when we get a newline, break out of loop
      }
    }
    
    // refresh the usb port for 10 milliseconds
    DigiUSB.delay(10);
  }
}

void loop() {
  // print output
  DigiUSB.println("Waiting for input...");
  // get input
  get_input();
}