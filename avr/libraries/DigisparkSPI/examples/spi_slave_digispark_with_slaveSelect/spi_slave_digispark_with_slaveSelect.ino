//Ian Walsh
//SPI slave using ATTiny85's USI.
//Working correctly up to 4MHz (recieving)

//pin 3 is pulled high for USB stuff, so pin 4 works better here
#define SS 3

uint8_t spiDataSend;
uint8_t spiDataRead;
bool newData = false;
volatile bool hadSpiInterrupt = false;
volatile unsigned long lastSpiTime = 0;
uint8_t slaveCounter = 0; //data to send back to master

uint8_t slaveSend, slaveReceive;

void setup() {                
  pinMode(0, INPUT);  //MOSI
  pinMode(1, OUTPUT); //MISO
  pinMode(2, INPUT);  //SCK
  pinMode(SS, INPUT);

  GIMSK |= (1 << PCIE);//enable global pin change interrupts
  PCMSK |= (1 << SS);//enable PCINT on pin 4

  //set USI to three wire mode, external clock.
  USICR = (1<<USIWM0) | (1<<USICS1); 
  //clear USI data register
  USIDR = 0;
}

void loop() {
  if (newData) {//equivalent later to spi.newData flag or something?
    spiDataSend = spiDataRead + 0x10;
    //set USIDR here, because it will be sent asap on next transfer
    USIDR = spiDataSend;

    newData = false;
  }
}

ISR(USI_OVF_vect) {
  //read SPI data from register
  spiDataRead = USIDR;//supposed to be able to use USIBR for a buffered read but
  // it shifts byte by 1 bit. Maybe supposed to wait longer before reading?

  //interactions with registers should occur ASAP in interrupt
  //clear overflow flag, reset counter
  USISR = (1<<USIOIF); 

  USIDR = spiDataRead + 0x10;
  newData = true;
}

ISR(PCINT0_vect) {
  if ( !(PINB & (1 << SS)) ) {//if pin 4 is LOW
    //set MISO (1) to output
    DDRB |= (1 << 1); //faster than pinMode(1, OUTPUT)

    //clear USI counter and overflow flag
    USISR = (1<<USIOIF);

    //enable USI overflow interrupt
    USICR |= (1<<USIOIE);

    //need this? data should already be there?
    //THIS BREAKS IT
    //USIDR = spiDataSend;//0xAA;
  } else {

    //disable overflow interrupt
    USICR &= ~(1<<USIOIE);

    //set MISO (1) as input (tri-state)
    DDRB &= ~(1 << 1); //faster than pinMode(1, INPUT)
  }
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