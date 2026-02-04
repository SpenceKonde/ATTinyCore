//Ian Walsh
//SPI slave using ATTiny85's USI.
//Working correctly up to 4MHz (recieving)
//Clock speed must be 16.5MHz for using USB

#include <DigiCDC.h>

//using SS incompatible with SerialUSB: it uses pin 3
//This program doesn't use the SS pin; it acts as though SS is pulled LOW all the time.
//It will respond to any activity on the SPI bus, so the setup cana only be:
//One master -> this single slave.

uint8_t spiDataSend;
volatile byte spiDataRead;
bool newData = false;
volatile bool hadSpiInterrupt = false;
volatile unsigned long lastSpiTime = 0;
byte slaveCounter = 0; //data to send back to master

uint8_t slaveSend, slaveReceive;

void setup() {                
  SerialUSB.begin(); 
  spiSlaveInit();
  SerialUSB.delay(1000);//delay prevents SerialUSB from locking up
}

void loop() {
  //slaveSend = slaveReceive + 0x10;
  //slaveReceive = spiTransfer(slaveSend);
  //spiTransferFast();

  if (newData) {//equivalent later to spi.newData flag or something?
    SerialUSB.print("\nReceived: 0x");
    SerialUSB.println(spiDataRead, HEX);
    SerialUSB.println("Sent update in interrput");
    //SerialUSB.println(spiDataSend, HEX);
    newData = false;
  }

  //reset register since no ss interrupt to clear it
  if (millis() - lastSpiTime > 100) {
      USISR = (1<<USIOIF); 
      lastSpiTime = millis(); 
  }
  
  SerialUSB.refresh(); 
}

ISR(USI_OVF_vect) {
  //read SPI data from register
  spiDataRead = USIDR;//supposed to be able to use USIBR for a buffered read but
  // it shifts byte by 1 bit. Maybe supposed to wait longer before reading?

  //register should be written directly after being read
  USIDR = spiDataRead + 0x10;

  //interactions with registers should occur ASAP in interrupt
  //clear overflow flag, reset counter
  USISR = (1<<USIOIF); 


  hadSpiInterrupt = true;
  newData = true;
}

void spiSlaveInit() {
  pinMode(1, OUTPUT); // MISO 
  pinMode(0, INPUT);  // MOSI
  pinMode(2, INPUT);  // SCK 
  
  //clear data register
  USIDR = 0;

 
  USICR = (1<<USIWM0)  //three wire mode (SPI)
         |(1<<USIOIE)  //enable interrupt
         |(1<<USICS1); //external clock (positive edge)
         
  USISR = (1<<USIOIF); //clear flags/counter
}

uint8_t spiTransfer(uint8_t sendByte) {
  if (hadSpiInterrupt) {
    hadSpiInterrupt = false;
    lastSpiTime = millis();

    spiDataSend = sendByte;
    newData = true;
    return spiDataRead;
  }
}

void spiTransferFast(void) {//not as fast as modifying in interrupt
  if (hadSpiInterrupt) {
    hadSpiInterrupt = false;
    lastSpiTime = millis();

    spiDataSend = spiDataRead + 0x10;
    newData = true;
  }
}