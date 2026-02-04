#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,12);
const int LED = 1;

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The role of the current running sketch
role_e role = role_pong_back;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  
  digitalWrite(LED, LOW);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(pipes[1]);
  digitalWrite(LED, HIGH);
}

void loop() {
  unsigned long value = 2000;
  bool ok = radio.write(&value, sizeof(unsigned long));
  
  if(ok) {
    digitalWrite(LED, LOW);
    delay(2000);
    digitalWrite(LED, HIGH);
  }
}