/*-------------------------------------------------------------------------*
 * tinySPI.h - Arduino hardware SPI master library for ATtiny24/44/84,     *
 * ATtiny25/45/85, and Attiny2313/4313.                                    *
 *                                                                         *
 * Original version of tinyISP by Jack Christensen 24Oct2013               *
 *                                                                         *
 * Added support for Attiny24/25, and Attiny2313/4313                      *
 * by Leonardo Miliani 28Nov2014                                           *                          
 *                                                                         *
 * CC BY-SA-NC:                                                            *
 * This work is licensed under the Creative Commons Attribution-           *
 * ShareAlike- Not Commercial 4.0 Unported License. To view a copy of this *
 * license, visit                                                          *
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a                *
 * letter to Creative Commons, 171 Second Street, Suite 300,               *
 * San Francisco, California, 94105, USA.                                  *
 *-------------------------------------------------------------------------*/

#include "SPI.h"


#ifdef SPDR //Then we have hardware SPI, let's use it:


#include "SPI.h"

SPIClass SPI;

uint8_t SPIClass::initialized = 0;
uint8_t SPIClass::interruptMode = 0;
uint8_t SPIClass::interruptMask = 0;
uint8_t SPIClass::interruptSave = 0;
#ifdef SPI_TRANSACTION_MISMATCH_LED
uint8_t SPIClass::inTransactionFlag = 0;
#endif

void SPIClass::begin()
{
  uint8_t sreg = SREG;
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  if (!initialized) {
 
    #ifdef  REMAP
    uint8_t SS_pin=0;

    if (REMAP & 1<<SPIMAP) {
      pinMode(SCK_REMAP, OUTPUT);
      pinMode(MOSI_REMAP, OUTPUT);
      SS_pin=SS_REMAP;
    } else {
      pinMode(SCK, OUTPUT);
      pinMode(MOSI, OUTPUT);
      SS_pin=SS;
    }    
    uint8_t port = digitalPinToPort(SS_pin);
    uint8_t bit = digitalPinToBitMask(SS_pin);
    volatile uint8_t *reg = portModeRegister(port);

    // if the SS pin is not already configured as an output
    // then set it high (to enable the internal pull-up resistor)
    if(!(*reg & bit)){
      digitalWrite(SS_pin, HIGH);
    }
    pinMode(SS_pin, OUTPUT);
    #else 
    uint8_t port = digitalPinToPort(SS);
    uint8_t bit = digitalPinToBitMask(SS);
    volatile uint8_t *reg = portModeRegister(port);

    // if the SS pin is not already configured as an output
    // then set it high (to enable the internal pull-up resistor)
    if(!(*reg & bit)){
      digitalWrite(SS, HIGH);
    }
       // Set SS to high so a connected chip will be "deselected" by default

    

    // When the SS pin is set as OUTPUT, it can be used as
    // a general purpose output port (it doesn't influence
    // SPI operations).

    // Warning: if the SS pin ever becomes a LOW INPUT then SPI
    // automatically switches to Slave, so the data direction of
    // the SS pin MUST be kept as OUTPUT.

    // Set direction register for SCK and MOSI pin.
    // MISO pin automatically overrides to INPUT.
    // By doing this AFTER enabling SPI, we avoid accidentally
    // clocking in a single bit since the lines go directly
    // from "input" to SPI control.
    // http://code.google.com/p/arduino/issues/detail?id=888
    pinMode(SS, OUTPUT);
    pinMode(SCK, OUTPUT);
    pinMode(MOSI, OUTPUT);
    #endif
    SPCR |= _BV(MSTR);
    SPCR |= _BV(SPE);
  }
  initialized++; // reference count
  SREG = sreg;
}

void SPIClass::end() {
  uint8_t sreg = SREG;
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  // Decrease the reference counter
  if (initialized)
    initialized--;
  // If there are no more references disable SPI
  if (!initialized) {
    SPCR &= ~_BV(SPE);
    interruptMode = 0;
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    inTransactionFlag = 0;
    #endif
  }
  SREG = sreg;
}

// mapping of interrupt numbers to bits within SPI_AVR_EIMSK
#if defined(__AVR_ATmega32U4__)
  #define SPI_INT0_MASK  (1<<INT0)
  #define SPI_INT1_MASK  (1<<INT1)
  #define SPI_INT2_MASK  (1<<INT2)
  #define SPI_INT3_MASK  (1<<INT3)
  #define SPI_INT4_MASK  (1<<INT6)
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
  #define SPI_INT0_MASK  (1<<INT0)
  #define SPI_INT1_MASK  (1<<INT1)
  #define SPI_INT2_MASK  (1<<INT2)
  #define SPI_INT3_MASK  (1<<INT3)
  #define SPI_INT4_MASK  (1<<INT4)
  #define SPI_INT5_MASK  (1<<INT5)
  #define SPI_INT6_MASK  (1<<INT6)
  #define SPI_INT7_MASK  (1<<INT7)
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
  #define SPI_INT0_MASK  (1<<INT4)
  #define SPI_INT1_MASK  (1<<INT5)
  #define SPI_INT2_MASK  (1<<INT0)
  #define SPI_INT3_MASK  (1<<INT1)
  #define SPI_INT4_MASK  (1<<INT2)
  #define SPI_INT5_MASK  (1<<INT3)
  #define SPI_INT6_MASK  (1<<INT6)
  #define SPI_INT7_MASK  (1<<INT7)
#else
  #ifdef INT0
  #define SPI_INT0_MASK  (1<<INT0)
  #endif
  #ifdef INT1
  #define SPI_INT1_MASK  (1<<INT1)
  #endif
  #ifdef INT2
  #define SPI_INT2_MASK  (1<<INT2)
  #endif
#endif

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
  uint8_t mask = 0;
  uint8_t sreg = SREG;
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  switch (interruptNumber) {
  #ifdef SPI_INT0_MASK
  case 0: mask = SPI_INT0_MASK; break;
  #endif
  #ifdef SPI_INT1_MASK
  case 1: mask = SPI_INT1_MASK; break;
  #endif
  #ifdef SPI_INT2_MASK
  case 2: mask = SPI_INT2_MASK; break;
  #endif
  #ifdef SPI_INT3_MASK
  case 3: mask = SPI_INT3_MASK; break;
  #endif
  #ifdef SPI_INT4_MASK
  case 4: mask = SPI_INT4_MASK; break;
  #endif
  #ifdef SPI_INT5_MASK
  case 5: mask = SPI_INT5_MASK; break;
  #endif
  #ifdef SPI_INT6_MASK
  case 6: mask = SPI_INT6_MASK; break;
  #endif
  #ifdef SPI_INT7_MASK
  case 7: mask = SPI_INT7_MASK; break;
  #endif
  default:
    interruptMode = 2;
    break;
  }
  interruptMask |= mask;
  if (!interruptMode)
    interruptMode = 1;
  SREG = sreg;
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber)
{
  // Once in mode 2 we can't go back to 0 without a proper reference count
  if (interruptMode == 2)
    return;
  uint8_t mask = 0;
  uint8_t sreg = SREG;
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  switch (interruptNumber) {
  #ifdef SPI_INT0_MASK
  case 0: mask = SPI_INT0_MASK; break;
  #endif
  #ifdef SPI_INT1_MASK
  case 1: mask = SPI_INT1_MASK; break;
  #endif
  #ifdef SPI_INT2_MASK
  case 2: mask = SPI_INT2_MASK; break;
  #endif
  #ifdef SPI_INT3_MASK
  case 3: mask = SPI_INT3_MASK; break;
  #endif
  #ifdef SPI_INT4_MASK
  case 4: mask = SPI_INT4_MASK; break;
  #endif
  #ifdef SPI_INT5_MASK
  case 5: mask = SPI_INT5_MASK; break;
  #endif
  #ifdef SPI_INT6_MASK
  case 6: mask = SPI_INT6_MASK; break;
  #endif
  #ifdef SPI_INT7_MASK
  case 7: mask = SPI_INT7_MASK; break;
  #endif
  default:
    break;
    // this case can't be reached
  }
  interruptMask &= ~mask;
  if (!interruptMask)
    interruptMode = 0;
  SREG = sreg;
}


#else 
#ifdef USICR

tinySPI::tinySPI() 
{
}

uint8_t tinySPI::reversebit=0;
uint8_t tinySPI::initialized=0;

uint8_t tinySPI::interruptMode = 0;
uint8_t tinySPI::interruptMask = 0;
uint8_t tinySPI::interruptSave = 0;
void tinySPI::begin(void)
{
    USICR &= ~(_BV(USISIE) | _BV(USIOIE) | _BV(USIWM1));
    USICR |= _BV(USIWM0) | _BV(USICS1) | _BV(USICLK);
    USI_SCK_PORT |= _BV(USCK_DD_PIN);   //set the USCK pin as output
    USI_DDR_PORT |= _BV(DO_DD_PIN);     //set the DO pin as output
    USI_DDR_PORT &= ~_BV(DI_DD_PIN);    //set the DI pin as input
    initialized++;
}

void tinySPI::setDataMode(uint8_t spiDataMode)
{
    if (spiDataMode == SPI_MODE1)
        USICR |= _BV(USICS0);
    else
        USICR &= ~_BV(USICS0);
}

void tinySPI::setClockDivider(uint8_t clockDiv)
{
  // CD - this routine currently does nothing as clock dividers for the
  //      software driven USI SPI bus are ignored for this first cut
  uint8_t dummy; // Nothing routine (needed so not optimised away)
}
  
byte tinySPI::reverse (byte x){
 byte result;
 asm("mov __tmp_reg__, %[in] \n\t"
  "lsl __tmp_reg__  \n\t"   /* shift out high bit to carry */
  "ror %[out] \n\t"  /* rotate carry __tmp_reg__to low bit (eventually) */
  "lsl __tmp_reg__  \n\t"   /* 2 */
  "ror %[out] \n\t"
  "lsl __tmp_reg__  \n\t"   /* 3 */
  "ror %[out] \n\t"
  "lsl __tmp_reg__  \n\t"   /* 4 */
  "ror %[out] \n\t"
  "lsl __tmp_reg__  \n\t"   /* 5 */
  "ror %[out] \n\t"
  "lsl __tmp_reg__  \n\t"   /* 6 */
  "ror %[out] \n\t"
  "lsl __tmp_reg__  \n\t"   /* 7 */
  "ror %[out] \n\t"
  "lsl __tmp_reg__  \n\t"   /* 8 */
  "ror %[out] \n\t"
  : [out] "=r" (result) : [in] "r" (x));
  return(result);
}

uint8_t tinySPI::transfer(uint8_t spiData)
{
    USIDR = (reversebit?spiData:reverse(spiData));
    USISR = _BV(USIOIF);                //clear counter and counter overflow interrupt flag
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //ensure a consistent clock period
        while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC); // CD - need to add clockDiv based delays?
    }
    return USIDR;
}


uint16_t tinySPI::transfer16(uint16_t data) {
 union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
 in.val = data;
 USIDR = in.msb;
 USISR = _BV(USIOIF);                //clear counter and counter overflow interrupt flag
 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //ensure a consistent clock period
  while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC); // CD - need to add clockDiv based delays?
 }
 out.msb = USIDR; 
 USIDR = in.lsb;
 USISR = _BV(USIOIF);                //clear counter and counter overflow interrupt flag
 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //ensure a consistent clock period
  while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC); // CD - need to add clockDiv based delays?
 }
 out.lsb = USIDR;
 return out.val;
}

void tinySPI::transfer(void *buf, size_t count) {
 if (count == 0) return;
 uint8_t *p = (uint8_t *)buf;
 USIDR = *p;
 while (--count > 0) {
  uint8_t out = *(p + 1);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //ensure a consistent clock period
   while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC); // CD - need to add clockDiv based delays?
  }
  uint8_t in = USIDR;
  USIDR = out;
  *p++ = in;
 }
 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //ensure a consistent clock period
  while ( !(USISR & _BV(USIOIF)) ) USICR |= _BV(USITC); // CD - need to add clockDiv based delays?
 }
 *p = USIDR;
}

static void tinySPI::beginTransaction(SPISettings settings) {
    if (interruptMode > 0) {
      uint8_t sreg = SREG;
      noInterrupts();

      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        interruptSave = SPI_AVR_EIMSK;
        SPI_AVR_EIMSK &= ~interruptMask;
        SREG = sreg;
      } else
      #endif
      {
        interruptSave = sreg;
      }
    }
    USICR = settings.usicr;
    reversebit=settings.reverse;
  }

static void tinySPI::endTransaction(void) {
  if (interruptMode > 0) {
    #ifdef SPI_AVR_EIMSK
    uint8_t sreg = SREG;
    #endif
    noInterrupts();
    #ifdef SPI_AVR_EIMSK
    if (interruptMode == 1) {
      SPI_AVR_EIMSK = interruptSave;
      SREG = sreg;
    } else
    #endif
    {
      SREG = interruptSave;
    }
  }
}
#ifdef INT0
#define SPI_INT0_MASK  (1<<INT0)
#endif
#ifdef INT1
#define SPI_INT1_MASK  (1<<INT1)
#endif
#ifdef INT2
#define SPI_INT2_MASK  (1<<INT2)
#endif

void tinySPI::usingInterrupt(uint8_t interruptNumber)
{
  uint8_t mask = 0;
  uint8_t sreg = SREG;
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  switch (interruptNumber) {
  #ifdef SPI_INT0_MASK
  case 0: mask = SPI_INT0_MASK; break;
  #endif
  #ifdef SPI_INT1_MASK
  case 1: mask = SPI_INT1_MASK; break;
  #endif
  #ifdef SPI_INT2_MASK
  case 2: mask = SPI_INT2_MASK; break;
  #endif
  default:
    interruptMode = 2;
    break;
  }
  interruptMask |= mask;
  if (!interruptMode)
    interruptMode = 1;
  SREG = sreg;
}

void tinySPI::notUsingInterrupt(uint8_t interruptNumber)
{
  // Once in mode 2 we can't go back to 0 without a proper reference count
  if (interruptMode == 2)
    return;
  uint8_t mask = 0;
  uint8_t sreg = SREG;
  noInterrupts(); // Protect from a scheduler and prevent transactionBegin
  switch (interruptNumber) {
  #ifdef SPI_INT0_MASK
  case 0: mask = SPI_INT0_MASK; break;
  #endif
  #ifdef SPI_INT1_MASK
  case 1: mask = SPI_INT1_MASK; break;
  #endif
  #ifdef SPI_INT2_MASK
  case 2: mask = SPI_INT2_MASK; break;
  #endif
  default:
    break;
    // this case can't be reached
  }
  interruptMask &= ~mask;
  if (!interruptMask)
    interruptMode = 0;
  SREG = sreg;
}
void tinySPI::end(void)
{
    USICR &= ~(_BV(USIWM1) | _BV(USIWM0));
}

tinySPI SPI = tinySPI();                //instantiate a tinySPI objec


#endif
#endif
