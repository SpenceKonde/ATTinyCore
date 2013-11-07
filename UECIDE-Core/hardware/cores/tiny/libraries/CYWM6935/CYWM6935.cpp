
#include "CYWM6935.h"
#include <Arduino.h>
#include <avr/interrupt.h>

//======= Interrupt driven ring buffers.

SoftRingBuffer radio_rx_buffer  =  { { 0 }, 0, 0 };

CYWM6935 radio = CYWM6935(&radio_rx_buffer);

inline void storeChar(unsigned char c, SoftRingBuffer *buffer)
{
  byte i = (buffer->head + 1) & (BUFFER_SIZE-1); //buffer size is always a power of 2, so much more efficient than Modulo!

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

#if !defined(INT0_vect) && defined(EXT_INT0_vect)
#define INT0_vect EXT_INT0_vect
#elif !defined(INT0_vect)
#error Interrupt 0 vector not found!
#endif
ISR (INT0_vect) {
    if (radio.isTransmitting()) {
        //CYWM6935RegisterData flags = radio.readRegister(CYWM6935CommandMake(REG_TX_INT_STAT));
        //Serial.println((byte)flags.__private_data);
        
        CYWM6935Command command = CYWM6935CommandMake(REG_TX_DATA); //read bit is 1.
        // There is more data in the output buffer. Send the next byte
        //unsigned char c = radio_tx_buffer.buffer[radio_tx_buffer.tail];
        //radio_tx_buffer.tail = (radio_tx_buffer.tail + 1) & (BUFFER_SIZE-1);
        CYWM6935RegisterData data;
        data.REG_TX_DATA = radio._tx_buffer;
        radio.writeRegister(command,data); //Write to the transmit data register with next byte
        
        //if (radio_tx_buffer.head == radio_tx_buffer.tail) {
            /*CYWM6935Command*/ command = CYWM6935CommandMake(REG_TX_INT_EN); //read bit is 0.
            // Buffer empty, so disable interrupts
            //CYWM6935RegisterData data;
            data.clear();
            data.REG_TX_INT_EN.empty = Disabled;
            radio.writeRegister(command,data); //Disable TX empty interrupt.
            radio.setTransmitting(false);
        //}
        
    } else if (radio.isReceiving()) {
        CYWM6935Command command = CYWM6935CommandMake(REG_RX_INT_STAT);
        //assume it is a recieving interrupt.
        CYWM6935RegisterData flags = radio.readRegister(command);
        CYWM6935Command reg;
        if (flags.REG_RX_INT_STAT.fullA) {
            reg = CYWM6935CommandMake(REG_RX_DATA_A);
        } else if (flags.REG_RX_INT_STAT.fullB) {
            reg = CYWM6935CommandMake(REG_RX_DATA_B);
        } else {
            return; //dont care about this interrupt.
        }
        
        //Read in the data buffer and validity buffer.
        CYWM6935RegisterData data[2];
        radio.readRegister(reg,data,2);
        byte in = data[0].REG_RX_DATA;// & data[1].REG_RX_VALID; //exclude invalid bits.
        storeChar(in,&radio_rx_buffer);
    }
}

//======= De/Constructors

CYWM6935::CYWM6935(SoftRingBuffer* __rx_buffer) {
    _rx_buffer = __rx_buffer;
}

CYWM6935::~CYWM6935()
{
    SPI.end();
}

//======= Begin and End functions.

void CYWM6935::begin(byte channel, byte SCK_, byte MOSI_, byte MISO_, byte SS_) {
    _SS = SS_;
    _SCLK = SCK_;
    _MISO = MISO_;
    _MOSI = MOSI_;
    _begin(channel);
}

#if defined(MOSI) && defined(MISO) && defined(SCK)
void CYWM6935::begin(byte channel,byte SS_) {
    _SS = SS_;
    _SCLK = SCK;
    _MISO = MISO;
    _MOSI = MOSI;
    _begin(channel);
}

#endif
#if defined(SS) && defined(MOSI) && defined(MISO) && defined(SCK)
void CYWM6935::begin(byte channel) {
    _SS = SS;
    _SCLK = SCK;
    _MISO = MISO;
    _MOSI = MOSI;
    _begin(channel);
}
#endif
    
void CYWM6935::_begin(byte channel) {
        
    digitalWrite(_SS,HIGH);
    pinMode(_SS,OUTPUT);
    digitalWrite(_SS,HIGH);
    delay(1);
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV16);
    SPI.begin(_SCLK,_MOSI,_MISO,_SS);
    
    CYWM6935Command command = CYWM6935CommandMake(REG_ANALOG_CTL);
    CYWM6935RegisterData data;
    data.REG_ANALOG_CTL.reset = Enabled;
    writeRegister(command,data); //Reset registers.
    
    delayMicroseconds(50);
    
    data.clear();
    command = CYWM6935CommandMake(REG_CONTROL);
    writeRegister(command,data);
    
    data.clear();
    data.REG_DATA_RATE.sampleRate = Oversampling6x;
    data.REG_DATA_RATE.dataRate = NormalDataRate;
    data.REG_DATA_RATE.codeWidth = CodeWidth64bit;
    command = CYWM6935CommandMake(REG_DATA_RATE);
    writeRegister(command,data);   
    
    data.clear();
    data.REG_CONFIG.pinSelect = OpenDrain;
    command = CYWM6935CommandMake(REG_CONFIG);
    writeRegister(command,data);   
    
    data.clear();
    data.REG_THRESHOLD_L.low = 12;
    command = CYWM6935CommandMake(REG_THRESHOLD_L);
    writeRegister(command,data);
    
    data.clear();
    data.REG_THRESHOLD_H.high = 52;
    command = CYWM6935CommandMake(REG_THRESHOLD_H);
    writeRegister(command,data);
    
    data.clear();
    data.REG_CRYSTAL_ADJ.clockOut = Enabled;
    command = CYWM6935CommandMake(REG_CRYSTAL_ADJ);
    writeRegister(command,data);
    
    data.clear();
    data.REG_PA.transmitPower = 5;
    command = CYWM6935CommandMake(REG_PA);
    writeRegister(command,data); 
    
    data.clear();
    data.REG_SERDES_CTL.SERDES = Enabled;
    data.REG_SERDES_CTL.EOFLength = 3;
    command = CYWM6935CommandMake(REG_SERDES_CTL);
    writeRegister(command,data); 
    
    data.REG_TX_VALID = 0xFF;
    command = CYWM6935CommandMake(REG_TX_VALID);
    writeRegister(command,data);  
    
    data.clear();
    data.REG_ANALOG_CTL.PAOutput = Enabled;
    data.REG_ANALOG_CTL.PWRWriteAccess = Enabled;
    command = CYWM6935CommandMake(REG_ANALOG_CTL);
    writeRegister(command,data);
    
    data.clear();
    data.REG_PWR_CTL.powerSaveMode = Enabled;
    command = CYWM6935CommandMake(REG_PWR_CTL);
    writeRegister(command,data);
    
    data.REG_SYN_LOCK_CNT = 150;
    command = CYWM6935CommandMake(REG_SYN_LOCK_CNT);
    writeRegister(command,data);  
    
    data.REG_CLOCK_MANUAL = 0x41;
    command = CYWM6935CommandMake(REG_CLOCK_MANUAL);
    writeRegister(command,data);
    //data.REG_CLOCK_ENABLE = 0x41;
    command = CYWM6935CommandMake(REG_CLOCK_ENABLE);
    writeRegister(command,data);
    
    
    
    
    data.clear();
    data.REG_VCO_CAL.VCOSlope = MaxAdjust;
    command = CYWM6935CommandMake(REG_VCO_CAL);
    writeRegister(command,data);
    
    setChannel(channel);
       
    resume();
}

void CYWM6935::resume(){
    MCUCR |= (1<<ISC01); 
    MCUCR &= ~(0<<ISC00);
    GIFR |= (1 << INTF0);
    GIMSK |= (1<<INT0);
    
    //restore Interrupts
    writeRegister(CYWM6935CommandMake(REG_RX_INT_EN),_intRX);
    writeRegister(CYWM6935CommandMake(REG_TX_INT_EN),_intTX);
}

void CYWM6935::end() {
    GIMSK &= ~(1<<INT0);
    
    //read current CYWM6935 interrupt enable bits
    CYWM6935Command rx = CYWM6935CommandMake(REG_RX_INT_EN);
    CYWM6935Command tx = CYWM6935CommandMake(REG_TX_INT_EN);
    _intRX = readRegister(rx);
    _intTX = readRegister(tx);
    //Disable interrupts
    CYWM6935RegisterData data; //constructor sets bits in data to 0.
    writeRegister(rx,data);
    writeRegister(tx,data);
}

boolean CYWM6935::isTransmitting(){
    return _transmitting;
}

void CYWM6935::setTransmitting(boolean transmitting){
    _transmitting = transmitting;
}

boolean CYWM6935::isReceiving(){
    return _receiving;
}

void CYWM6935::setReceiving(boolean receiving){
    _receiving = receiving;
}

//======= Control Tx/Rx

void CYWM6935::setTransmitter(Enabled_t status){
    if ((status == Enabled) && (_recieverEn == Enabled)) {
        setReciever(Disabled); //don't allow both TX and RX to be enabled at the same time
    }
    setReceiving(false);
    CYWM6935Command command = CYWM6935CommandMake(REG_CONTROL);
    CYWM6935RegisterData data = readRegister(command);
    data.REG_CONTROL.transmitter = status;
    data.REG_CONTROL.synLockBypass = status;
    writeRegister(command,data);
    
    _transmitterEn = status;
}

void CYWM6935::setReciever(Enabled_t status){
    if ((status == Enabled) && (_transmitterEn == Enabled)) {
        setTransmitter(Disabled); //don't allow both TX and RX to be enabled at the same time
    }
    if(status == Enabled) {
        setReceiving(true);
        setTransmitting(false);
    } else {
        setReceiving(false);
    }
    CYWM6935Command command = CYWM6935CommandMake(REG_CONTROL);
    CYWM6935RegisterData data = readRegister(command);
    data.REG_CONTROL.reciever = status;
    data.REG_CONTROL.synLockBypass = status;
    writeRegister(command,data);
    
    _intRX.REG_RX_INT_EN.fullA = status;
    _intRX.REG_RX_INT_EN.fullB = status;
    writeRegister(CYWM6935CommandMake(REG_RX_INT_EN),_intRX);
    
    readRegister(CYWM6935CommandMake(REG_RX_INT_STAT)); //clear the flag if set
    readRegister(CYWM6935CommandMake(REG_RX_DATA_A)); //clear the flag if set
    
    _recieverEn = status;
}

Enabled_t CYWM6935::transmitterIs(){
  return _transmitterEn;
}
Enabled_t CYWM6935::recieverIs(){
  return _recieverEn;
}

//======= Adjust/Read current channel

void CYWM6935::setChannel(byte channel){
    _channel = channel;
    
    CYWM6935RegisterData data;
    data.REG_CHANNEL.channel = channel;
    CYWM6935Command command = CYWM6935CommandMake(REG_CHANNEL);
    writeRegister(command,data); //Enable RX interrupt.
}

byte CYWM6935::currentChannel(){
    return _channel;
}

//======= Register Read/Write

void CYWM6935::writeRegister(CYWM6935Command command, CYWM6935RegisterData data){
    byte oldSREG = SREG;
    cli();
    command.reg.increment = Disabled;
    command.reg.direction = Write;
    //byte array[1];
    //array[0] = data.__private_data;
    __private_writeRegister(command.__private_data,&data.__private_data,1);
    SREG = oldSREG;
}

CYWM6935RegisterData CYWM6935::readRegister(CYWM6935Command command){
    byte oldSREG = SREG;
    cli();
    CYWM6935RegisterData data;
    command.reg.direction = Read;
    command.reg.increment = Disabled;
    //byte array[1];
    __private_readRegister(command.__private_data,&data.__private_data,1);
    //data.__private_data = array[0];
    SREG = oldSREG;
    return data;
}

//======= Extended Register Read/Write

void CYWM6935::writeExtendedRegister(CYWM6935Command command, CYWM6935ExtendedRegisterData data){
    byte oldSREG = SREG;
    cli();
    command.extReg.increment = Enabled;
    command.extReg.direction = Write;
    if (command.extReg.address == REG_PN_CODE) {
        __private_writeRegister(command.__private_data,data.REG_PN_CODE.bytes,sizeof(data.REG_PN_CODE.bytes));
    } else {
        __private_writeRegister(command.__private_data,data.REG_MID.bytes,sizeof(data.REG_MID.bytes));
    }
    SREG = oldSREG;
}

CYWM6935ExtendedRegisterData CYWM6935::readExtendedRegister(CYWM6935Command command){
    byte oldSREG = SREG;
    cli();
    command.extReg.increment = Enabled;
    command.extReg.direction = Read;
    CYWM6935ExtendedRegisterData data;
    if (command.extReg.address == REG_PN_CODE) {
        __private_readRegister(command.__private_data,data.REG_PN_CODE.bytes,sizeof(data.REG_PN_CODE.bytes));
    } else {
        __private_readRegister(command.__private_data,data.REG_MID.bytes,sizeof(data.REG_MID.bytes));
    }
    SREG = oldSREG;
    return data;
}

//======= Multiple Register Read/Write

void CYWM6935::writeRegister(CYWM6935Command command, CYWM6935RegisterData* data, byte count){
    byte oldSREG = SREG;
    cli();
    command.reg.direction = Write;
    command.reg.increment = Enabled;
    byte* ptr = (byte*)(data);
    __private_writeRegister(command.__private_data,ptr,count);
    SREG = oldSREG;
}

void CYWM6935::readRegister(CYWM6935Command command, CYWM6935RegisterData* data, byte count){
    byte oldSREG = SREG;
    cli();
    command.reg.direction = Read;
    command.reg.increment = Enabled;
    byte* ptr = (byte*)(data);
    __private_readRegister(command.__private_data,ptr,count);
    SREG = oldSREG;
}

//======= SPI interface helpers

void CYWM6935::__private_writeRegister(byte command, byte* data, byte count){
    SPI.writeSS(LOW);
    SPI.transfer(command);
    byte i = 0;
    while(count--) {
        SPI.transfer(data[i++]);
    }
    SPI.writeSS(HIGH);
}

void CYWM6935::__private_readRegister(byte command, byte* data, byte count){
    SPI.writeSS(LOW);
    SPI.transfer(command);
    byte i = 0;
    while(count--) {
        data[i++] = SPI.transfer(0x00);
    }
    SPI.writeSS(HIGH);
}/*
void CYWM6935::__private_writeRegister(byte command, byte* data, byte count){
    Serial.print("\rWrite:");
    Serial.write(command);
    byte i = 0;
    while(count--) {
        Serial.write(data[i++]);
    }
}

void CYWM6935::__private_readRegister(byte command, byte* data, byte count){
    Serial.print("\rRead:");
    Serial.write(command);
    byte i = 0;
    while(count--) {
        while(!Serial.available());
        data[i++] = Serial.read();
    }
}*/

//TX Data write function (from Print)

#if ARDUINO >= 100
size_t CYWM6935::write(const unsigned char character) {
#else
void CYWM6935::write(const unsigned char character) {
#endif
    if(_transmitterEn == Disabled){
        #if ARDUINO >= 100
        return 0;
        #else
        return;
        #endif
    }

    //byte i = (_tx_buffer->head + 1) & (BUFFER_SIZE-1);

    //if(_transmitting){
    //    while (i == _tx_buffer->tail); //wait for buffer to clear.
    //}
    //_tx_buffer->buffer[_tx_buffer->head] = character;
    //_tx_buffer->head = i;
    _tx_buffer = character;
    
    //if(!_transmitting){
    _transmitting = true;
    CYWM6935RegisterData data;
    CYWM6935Command command = CYWM6935CommandMake(REG_TX_INT_EN);
    data.REG_TX_INT_EN.empty = Enabled;
    writeRegister(command,data); //Enable TX empty interrupt.
    //}
    while(radio.isTransmitting()); //wait for packet to be started.
    
    command = CYWM6935CommandMake(REG_TX_INT_STAT);
    //assume it is a recieving interrupt.
    data = radio.readRegister(command);
    while(!data.REG_TX_INT_STAT.done) { //wait for transmission complete marker.
      data = radio.readRegister(command);
    }
#if ARDUINO >= 100
    return 1;
}
#else
}
#endif

//RX Data read functions (from Stream)

int CYWM6935::available(void) {
    if(_recieverEn == Disabled){
        return 0;
    }
    return (unsigned int)(BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) & (BUFFER_SIZE-1);
}

int CYWM6935::peek(void){
    if(_recieverEn == Disabled){
        return -1;
    }
    if (_rx_buffer->head == _rx_buffer->tail) {
        return -1;
    } else {
        return _rx_buffer->buffer[_rx_buffer->tail];
    }
}

int CYWM6935::read(void){
    if(_recieverEn == Disabled){
        return -1;
    }
  // if the head isn't ahead of the tail, we don't have any characters
    if (_rx_buffer->head == _rx_buffer->tail) {
        return -1;
    } else {
        unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
        _rx_buffer->tail = (_rx_buffer->tail + 1) & (BUFFER_SIZE-1);
        return c;
    }
} 

void CYWM6935::flush(void){
    _rx_buffer->head = _rx_buffer->tail;
}

#ifdef INCLUDE_RSSI
const uint8_t CYWM6935::RSSI_peak(const uint8_t channel, const uint8_t count)
{    
    setChannel(channel);
    
    CYWM6935RegisterData data;
    
    data.__private_data = 0x80;
    writeRegister(CYWM6935CommandMake(REG_CONTROL), data);          // Turn receiver on
    // Wait for receiver to start-up
    // SYNTH_SETTLE (200) + RECEIVER_READY (35) + RSSI_ADC_CONVERSION (50)
    delayMicroseconds(285);
    data.clear();
    writeRegister(CYWM6935CommandMake(REG_CARRIER_DETECT), data);  // clear override

    uint8_t value = 0;
    for (uint8_t i = 0; i < count; i++) {
        while (1) {
            CYWM6935RegisterData v = readRegister(CYWM6935CommandMake(REG_RSSI));  // Read RSSI
            if (!v.REG_RSSI.valid) {
                data.__private_data = 0x80;
                writeRegister(CYWM6935CommandMake(REG_CARRIER_DETECT), data);  // set override
                delayMicroseconds(50);   // RSSI_ADC_CONVERSION (50)
                v = readRegister(CYWM6935CommandMake(REG_RSSI));  // Read RSSI
                data.clear();
                writeRegister(CYWM6935CommandMake(REG_CARRIER_DETECT), data);  // clear override 
            } else {
                if (v.REG_RSSI.RSSI > value) value = v.REG_RSSI.RSSI;
                break;
            }
        }
    } 
    return value;
}
#endif
