// This sketch will send a RFM12b packet that is compatible with the Jeelib and can be picked up by a JeeNode running RFM12 demo sketch
//See http://jeelabs.org/2011/06/09/rf12-packet-format-and-design/
//for packet design

#define GROUP  212
#define HEADER  17
//433mhz = 1, 868mhz = 2, 915mhz = 3
#define RF12_FREQ     1


#define RF12_NOT_CS()    PORTB |= _BV(PB3)
#define RF12_CS()        PORTB &= ~_BV(PB3)
#define MOSI_LOW() 	 PORTB &= ~_BV(PB1)
#define MISO_LEVEL() 	 (PINB & _BV(PB0))
#define RF12_TRANSMIT    0xB8

union
{
    unsigned char byte;
    struct
    {
char ATS_RSSI:
        1;	//ATS=Antenna tuning circuit detected strong enough RF signal
        //RSSI=The strength of the incoming signal is above the pre-programmed limit
char FFEM:
        1;		//FIFO is empty
char LBD:
        1;			//Low battery detect, the power supply voltage is below the pre-programmed limit
char EXT:
        1;			//Logic level on interrupt pin (pin 16) changed to low (Cleared after Status Read Command)
char WKUP:
        1;		//Wake-up timer overflow (Cleared after Status Read Command )
char RGUR_FFOV:
        1;	//RGUR=TX register under run, register over write (Cleared after Status Read Command )
        //FFOV=RX FIFO overflow (Cleared after Status Read Command )
char POR:
        1;			//Power-on reset (Cleared after Status Read Command )
char RGIT_FFIT:
        1;	//RGIT=TX register is ready to receive the next byte
        //(Can be cleared by Transmitter Register Write Command)
        //FFIT=The number of data bits in the RX FIFO has reached the pre-programmed limit
        //(Can be cleared by any of the FIFO read methods)
    }bits;
} status_H;

union
{
    unsigned char byte;
    struct
    {
char OFFS:
        4;		//Offset value to be added to the value of the frequency control parameter (Four LSB bits)
char OFFS6:
        1;		//MSB of the measured frequency offset (sign of the offset value)
char ATGL:
        1;		//Toggling in each AFC cycle
char CRL:
        1;			//Clock recovery locked
char DQD:
        1;			//Data quality detector output
    }bits;
} status_L;


void setup(){
DDRB = _BV(PB1) | _BV(PB2) | _BV(PB3) | _BV(PB4);
// MOSI, SCK, SEL
PORTB = _BV(PB3); // deselect RFM12
rf12_init();
}

//Some extremely dummy packet payload, just to show it works..
uint8_t data[] = { "TEST" };

void loop(){

	rf12_cmd(0x82,0x38); //Enable transciever
	rf12_send((uint8_t *)&data, sizeof(data));
        //wait till the next-to-last byte is sent (the last is the dummy)
        while (!rf12_read_status_MSB()); 
	rf12_cmd(0x82,0x08); //Disable transciever
        delay(3000);   
}


static void spi_run_clock () {
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
  USICR = _BV(USIWM0) | _BV(USITC);
  USICR = _BV(USIWM0) | _BV(USITC) | _BV(USICLK);
}

void rf12_cmd(uint8_t highbyte, uint8_t lowbyte)
{
    RF12_CS();
    USIDR = highbyte;
    spi_run_clock();
    USIDR = lowbyte;
    spi_run_clock();
    RF12_NOT_CS();
}

void rf12_loop_until_FFIT_RGIT(void)
{
    do
    {
        rf12_read_status_MSB();
    }
    while (!status_H.bits.RGIT_FFIT);
}

/*	rf12_read_status_MSB
	RX Mode: FFIT = The number of data bits in the RX FIFO has reached the pre-programmed limit.
					Can be cleared by any of the FIFO read methods
	TX Mode: RGIT = TX register is ready to receive the next byte
					(Can be cleared by Transmitter Register Write Command)
*/
uint8_t rf12_read_status_MSB(void)
{
    RF12_CS();
    MOSI_LOW();
    asm volatile("nop");
    if (MISO_LEVEL())
        status_H.bits.RGIT_FFIT=1;
    else
        status_H.bits.RGIT_FFIT=0;
    RF12_NOT_CS();
    return status_H.bits.RGIT_FFIT;
}

void rf12_read_status(void)
{
    RF12_CS();
    USIDR = 0x00;	//Status Read Command
    spi_run_clock();
    status_H.byte = USIDR;
    USIDR = 0x00; 	//Status Read Command
    spi_run_clock();
    status_L.byte = USIDR;
    RF12_NOT_CS();
}

void rf12_TX(uint8_t aByte)
{
    //FFIT wird gepollt um zu erkennen ob das FIFO TX
    //Register bereit ist.
    //Alternativ ist es auch möglich(wenn verbunden)
    //den Interrupt Ausgang des RF12 zu pollen: while(INT1_LEVEL());
    while (!rf12_read_status_MSB());
    rf12_cmd(RF12_TRANSMIT,aByte);
}


#if RF12_RECEIVE_CODE
uint8_t rf12_RX(void)
{
    rf12_loop_until_FFIT_RGIT();
    RF12_CS();
    USIDR = 0xB0;
    spi_run_clock();
    USIDR = 0x00;
    spi_run_clock();
    RF12_NOT_CS();
    return USIDR;
}
#endif

static __inline__ uint16_t _crc16_update(uint16_t __crc, uint8_t __data)
{
	uint8_t __tmp;
	uint16_t __ret;

	__asm__ __volatile__ (
		"eor %A0,%2" "\n\t"
		"mov %1,%A0" "\n\t"
		"swap %1" "\n\t"
		"eor %1,%A0" "\n\t"
		"mov __tmp_reg__,%1" "\n\t"
		"lsr %1" "\n\t"
		"lsr %1" "\n\t"
		"eor %1,__tmp_reg__" "\n\t"
		"mov __tmp_reg__,%1" "\n\t"
		"lsr %1" "\n\t"
		"eor %1,__tmp_reg__" "\n\t"
		"andi %1,0x07" "\n\t"
		"mov __tmp_reg__,%A0" "\n\t"
		"mov %A0,%B0" "\n\t"
		"lsr %1" "\n\t"
		"ror __tmp_reg__" "\n\t"
		"ror %1" "\n\t"
		"mov %B0,__tmp_reg__" "\n\t"
		"eor %A0,%1" "\n\t"
		"lsr __tmp_reg__" "\n\t"
		"ror %1" "\n\t"
		"eor %B0,__tmp_reg__" "\n\t"
		"eor %A0,%1"
		: "=r" (__ret), "=d" (__tmp)
		: "r" (__data), "0" (__crc)
		: "r0"
	);
	return __ret;
}


void rf12_send(const uint8_t* buf, uint8_t cnt)
{
    if (!cnt) return;
    uint16_t chksum=~0;	
	
//See http://jeelabs.org/2011/06/09/rf12-packet-format-and-design/
//http://jeelabs.org/2010/12/07/binary-packet-decoding/

    rf12_TX(0xAA);  //PREAMBLE
    rf12_TX(0xAA);  //PREAMBLE
    rf12_TX(0xAA);  //PREAMBLE		
    rf12_TX(0x2D);  //SYNC HI BYTE
rf12_TX(GROUP); //SYNC LOW BYTE (group 210)
chksum = _crc16_update(chksum, GROUP);		
    rf12_TX(HEADER);			// Header byte
	chksum = _crc16_update(chksum, HEADER);			
    rf12_TX(cnt);
	chksum = _crc16_update(chksum, cnt);			
    while (cnt--)
    {
        rf12_TX(*buf);
	    chksum = _crc16_update(chksum,*buf++);
    }
    rf12_TX(chksum);
	rf12_TX(chksum>>8);
    rf12_TX(0xAA);	//dummy byte
}

#if RF12_RECEIVE_CODE
//returns 0 if no data is available
//returns -1(255) if there is a CRC Error
//else, returns number of received byte
uint8_t rf12_read(uint8_t* buf, const uint8_t max)
{
    uint16_t checksum=~0; 
    uint16_t received_checksum;

    uint8_t hdr;
    hdr=rf12_RX();
    checksum = _crc16_update(checksum,hdr);
    
    uint8_t len;
    len=rf12_RX();
    checksum = _crc16_update(checksum,len);
    
    uint8_t i=len;
    while (i--)
    {
        *buf=rf12_RX();
        checksum = _crc16_update(checksum,*buf++);        
    }
    received_checksum=rf12_RX();
    received_checksum=received_checksum<<8;
    received_checksum |= rf12_RX();    

    if (received_checksum==checksum)
        return len;
    else
        return -1;
}
#endif

 	
void rf12_init(void)
{
    USICR = _BV(USIWM0); // 3-wire, software clock strobe
    rf12_cmd(0x80, 0xC7 | (RF12_868MHZ << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF 
    rf12_cmd(0xA6,0x40); // 868MHz 
    rf12_cmd(0xC6,0x06); // approx 49.2 Kbps, i.e. 10000/29/(1+6) Kbps
    rf12_cmd(0x94,0xA2); // VDI,FAST,134kHz,0dBm,-91dBm 
    rf12_cmd(0xC2,0xAC); // AL,!ml,DIG,DQD4 
    rf12_cmd(0xCA,0x83); // FIFO8,2-SYNC,!ff,DR 
    rf12_cmd(0xCE,0x00 | GROUP); // SYNC=2DXX； 
    rf12_cmd(0xC4,0x83); // @PWR,NO RSTRIC,!st,!fi,OE,EN 
    rf12_cmd(0x98,0x50); // !mp,90kHz,MAX OUT 
    rf12_cmd(0xCC,0x77); // OB1，OB0, LPX,！ddy，DDIT，BW0 
    rf12_cmd(0xE0,0x00); // NOT USE 
    rf12_cmd(0xC8,0x00); // NOT USE 
    rf12_cmd(0xC0,0x40); // 1.66MHz,2.2V 

}
