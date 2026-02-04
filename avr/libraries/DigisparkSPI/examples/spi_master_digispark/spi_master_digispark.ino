//Ian Walsh
//SPI master using ATTiny85's USI.
//Clock speed must be 16.5MHz for using USB

//Not enough pins to use digispark as SPI master with slave select (SS). 
//If using an ATTiny with USI and more pins available,
//see how SS is implemented in spi_master_atmega328p, and modify this code.

#include <DigiCDC.h>
#include <util/delay.h>

//using SS incompatible with SerialUSB: it uses pin 3
#define SS 3

uint8_t slowdown = 10;//us delay in spiTransfer() because it runs as fast as system clock allows

uint8_t numValue = 0;

void setup() {
  SerialUSB.begin();
  SerialUSB.println("Hello");

  pinMode(0, INPUT);  //MISO
  pinMode(1, OUTPUT); //MOSI
  pinMode(2, OUTPUT); //SCK

  //clear data register
  USIDR = 0;

  USICR = (1<<USIWM0);  //three wire mode (SPI)
                        //defaults to USICS[1:0] = 0 which == no clock/software strobe 
  SerialUSB.delay(1000);//delay prevents SerialUSB from locking up
}

void loop() {
  SerialUSB.refresh();//needed or it locks up?

  uint8_t masterSend, masterReceive;
  numValue++;
  masterSend = numValue;

  //digitalWrite(SS, LOW);//if you need to drive a SS
  masterReceive=spiTransfer(masterSend);
  //digitalWrite(SS, HIGH);//if you need to drive a SS

  //strange serial print order here is to reflect the correlation of received byte to sent byte
  SerialUSB.print("Recieved: 0x");
  SerialUSB.println(masterReceive, HEX);
  SerialUSB.print("\nSent: 0x");
  SerialUSB.println(masterSend, HEX); 

  //SerialUSB.println("spiTransfer called");
  SerialUSB.delay(1000);
}

uint8_t spiTransfer(uint8_t data) {
  //write data to USI data register
  USIDR = data;

  //clear counter and overflow flag
  USISR = (1 << USIOIF);

  //triggers as fast as clock cycles allow. how to slow down?
  //while not yet reached 16 on counter (aka 8 bits not yet transferred)
  while (!(USISR & (1 << USIOIF))) {

    /*
    if (slowdown) {
      for (uint8_t x = 0; x <= slowdown; x++) {
        asm volatile (
          "nop"
        );
      }
    }*/

    if (slowdown > 0) {
      _delay_us(slowdown);
      //set slowdown as const to save ram, its not changing
    }

    //USIWM0: maintain three wire mode
    //USICS1: select external, positive edge clock source (clock pin triggered by USITC)
    //USICLK: software slock strobe (when set constantly to 1, triggering USITC toggles the clock; reading this clock pin shifts the registers)
    //USITC:  toggle clock port pin (generates the physical clock edge). always a zero, triggered when set.
    USICR = (1 << USIWM0) | (1 << USICS1) | (1 << USICLK) | (1 << USITC);
  }

  // Return the data received from the Slave (now sitting in USIDR)
  return USIDR;
}