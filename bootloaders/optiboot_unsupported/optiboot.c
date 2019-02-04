/**********************************************************/
/* Optiboot bootloader for Arduino                        */
/*                                                        */
/* http://optiboot.googlecode.com                         */
/*                                                        */
/* Arduino-maintained version : See README.TXT            */
/* http://code.google.com/p/arduino/                      */
/*                                                        */
/* Heavily optimised bootloader that is faster and        */
/* smaller than the Arduino standard bootloader           */
/*                                                        */
/* Enhancements:                                          */
/*   Fits in 512 bytes, saving 1.5K of code space         */
/*   Background page erasing speeds up programming        */
/*   Higher baud rate speeds up programming               */
/*   Written almost entirely in C                         */
/*   Customisable timeout with accurate timeconstant      */
/*   Optional virtual UART. No hardware UART required.    */
/*   Optional virtual boot partition for devices without. */
/*                                                        */
/* What you lose:                                         */
/*   Implements a skeleton STK500 protocol which is       */
/*     missing several features including EEPROM          */
/*     programming and non-page-aligned writes            */
/*   High baud rate breaks compatibility with standard    */
/*     Arduino flash settings                             */
/*                                                        */
/* Fully supported:                                       */
/*   ATmega168 based devices  (Diecimila etc)             */
/*   ATmega328P based devices (Duemilanove etc)           */
/*   ATtiny84 based devices (Luminet)                     */
/*   ATtiny85 based devices                               */
/*                                                        */
/* Alpha test                                             */
/*   ATmega1280 based devices (Arduino Mega)              */
/*                                                        */
/* Work in progress:                                      */
/*   ATmega644P based devices (Sanguino)                  */
/*                                                        */
/* Does not support:                                      */
/*   USB based devices (eg. Teensy)                       */
/*                                                        */
/* Assumptions:                                           */
/*   The code makes several assumptions that reduce the   */
/*   code size. They are all true after a hardware reset, */
/*   but may not be true if the bootloader is called by   */
/*   other means or on other hardware.                    */
/*     No interrupts can occur                            */
/*     UART and Timer 1 are set to their reset state      */
/*     SP points to RAMEND                                */
/*                                                        */
/* Code builds on code, libraries and optimisations from: */
/*   stk500boot.c          by Jason P. Kyle               */
/*   Arduino bootloader    http://arduino.cc              */
/*   Spiff's 1K bootloader http://spiffie.org/know/arduino_1k_bootloader/bootloader.shtml */
/*   avr-libc project      http://nongnu.org/avr-libc     */
/*   Adaboot               http://www.ladyada.net/library/arduino/bootloader.html */
/*   AVR305                Atmel Application Note         */
/*                                                        */
/* This program is free software; you can redistribute it */
/* and/or modify it under the terms of the GNU General    */
/* Public License as published by the Free Software       */
/* Foundation; either version 2 of the License, or        */
/* (at your option) any later version.                    */
/*                                                        */
/* This program is distributed in the hope that it will   */
/* be useful, but WITHOUT ANY WARRANTY; without even the  */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A   */
/* PARTICULAR PURPOSE.  See the GNU General Public        */
/* License for more details.                              */
/*                                                        */
/* You should have received a copy of the GNU General     */
/* Public License along with this program; if not, write  */
/* to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA */
/*                                                        */
/* Licence can be viewed at                               */
/* http://www.fsf.org/licenses/gpl.txt                    */
/*                                                        */
/**********************************************************/


/**********************************************************/
/*                                                        */
/* Optional defines:                                      */
/*                                                        */
/**********************************************************/
/*                                                        */
/* BIG_BOOT:                                              */
/* Build a 1k bootloader, not 512 bytes. This turns on    */
/* extra functionality.                                   */
/*                                                        */
/* BAUD_RATE:                                             */
/* Set bootloader baud rate.                              */
/*                                                        */
/* LUDICROUS_SPEED:                                       */
/* 230400 baud :-)                                        */
/*                                                        */
/* SOFT_UART:                                             */
/* Use AVR305 soft-UART instead of hardware UART.         */
/*                                                        */
/* LED_START_FLASHES:                                     */
/* Number of LED flashes on bootup.                       */
/*                                                        */
/* LED_DATA_FLASH:                                        */
/* Flash LED when transferring data. For boards without   */
/* TX or RX LEDs, or for people who like blinky lights.   */
/*                                                        */
/* SUPPORT_EEPROM:                                        */
/* Support reading and writing from EEPROM. This is not   */
/* used by Arduino, so off by default.                    */
/*                                                        */
/* TIMEOUT_MS:                                            */
/* Bootloader timeout period, in milliseconds.            */
/* 500,1000,2000,4000,8000 supported.                     */
/*                                                        */
/**********************************************************/

/**********************************************************/
/* Version Numbers!                                       */
/*                                                        */
/* Arduino Optiboot now includes this Version number in   */
/* the source and object code.                            */
/*                                                        */
/* Version 3 was released as zip from the optiboot        */
/*  repository and was distributed with Arduino 0022.     */
/* Version 4 starts with the arduino repository commit    */
/*  that brought the arduino repository up-to-date with   */
/* the optiboot source tree changes since v3.             */
/*                                                        */
/**********************************************************/

/**********************************************************/
/* Edit History:                                          */
/*                                                        */
/* 4.5 TomC  : Fixed the software UART code so it actually*/
/*             works. Added support for Tiny84/85 (works).*/
/*             Added the ability to use RS484 with a chip.*/
/*             Converted some code to use Unions to reduce*/
/*             size considerably - tiny=576B, uno=480B.   */
/* 4.4 WestfW: add initialization of address to keep      */
/*             the compiler happy.  Change SC'ed targets. */
/*             Return the SW version via READ PARAM       */
/* 4.3 WestfW: catch framing errors in getch(), so that   */
/*             AVRISP works without HW kludges.           */
/*  http://code.google.com/p/arduino/issues/detail?id=368n*/
/* 4.2 WestfW: reduce code size, fix timeouts, change     */
/*             verifySpace to use WDT instead of appstart */
/* 4.1 WestfW: put version number in binary.          */
/**********************************************************/

#define OPTIBOOT_MAJVER 4
#define OPTIBOOT_MINVER 5

/*
#define MAKESTR(a) #a
#define MAKEVER(a, b) MAKESTR(a*256+b)
asm("  .section .version\n"
    "optiboot_version:  .word " MAKEVER(OPTIBOOT_MAJVER, OPTIBOOT_MINVER) "\n"
    "  .section .text\n");
*/
//It takes no extra memory (either flash or ram) to do it this way, but it means the address of the version number is known
#ifdef USE_TINY_TUNER
const unsigned char ver[4] __attribute__ ((section (".version"))) = {0xFF, 0xFF, OPTIBOOT_MINVER, OPTIBOOT_MAJVER};
#else
const unsigned char ver[2] __attribute__ ((section (".version"))) = {OPTIBOOT_MINVER, OPTIBOOT_MAJVER};
#endif
    
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// <avr/boot.h> uses sts instructions, but this version uses out instructions
// This saves cycles and program memory.
#include "boot.h"


#ifdef RS485_SUPPORT
#include <util/delay.h>
#endif

// We don't use <avr/wdt.h> as those routines have interrupt overhead we don't need.

#include "pin_defs.h"
#include "stk500.h"


#ifdef VIRTUAL_BOOT_PARTITION

	#ifndef VIRTUAL_BOOT_PARTITION_START
		#define VIRTUAL_BOOT_PARTITION_START 0x1dc0
	#endif
		//generate rjmp instruction for virtual boot partition
	#define RJUMP_COMMAND ((((VIRTUAL_BOOT_PARTITION_START/2)-FLASHEND-2) & 0xFFF) | 0xC000)
	#define RJUMP_COMMAND_LOW (RJUMP_COMMAND & 0xFF)
	#define RJUMP_COMMAND_HIGH ((RJUMP_COMMAND>>8) & 0xFF)

#endif

#ifndef LED_START_FLASHES
	#define LED_START_FLASHES 0
#endif

#ifdef LUDICROUS_SPEED
	#ifdef BAUD_RATE
	//Fix warnings about BAUD_RATE being redefined.
	#undef BAUD_RATE
	#endif
	#define BAUD_RATE 230400L
#endif

/* set the UART baud rate defaults */
#ifndef BAUD_RATE
	#if F_CPU >= 8000000L
		#define BAUD_RATE   115200L // Highest rate Avrdude win32 will support
	#elsif F_CPU >= 1000000L
		#define BAUD_RATE   9600L   // 19200 also supported, but with significant error
	#elsif F_CPU >= 128000L
		#define BAUD_RATE   4800L   // Good for 128kHz internal RC
	#else
		#define BAUD_RATE 1200L     // Good even at 32768Hz
	#endif
#endif

#if !defined(__AVR_ATtiny87__) && !defined(__AVR_ATtiny167__)
/* Switch in soft UART for hard baud rates */
#if (F_CPU/BAUD_RATE) > 280 // > 57600 for 16MHz
	#ifndef SOFT_UART
		#define SOFT_UART
	#endif
#endif

#endif

#if defined(SOFT_UART) && defined(RS485_SUPPORT)
	#undef SOFT_UART
#endif

/* Watchdog settings */
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))
#define WATCHDOG_32MS   (_BV(WDP0) | _BV(WDE))
#define WATCHDOG_64MS   (_BV(WDP1) | _BV(WDE))
#define WATCHDOG_125MS  (_BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_250MS  (_BV(WDP2) | _BV(WDE))
#define WATCHDOG_500MS  (_BV(WDP2) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_2S     (_BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDE))
#if !defined(__AVR_ATmega162__) && !defined(__AVR_ATmega8__)
	#define WATCHDOG_4S     (_BV(WDP3) | _BV(WDE))
	#define WATCHDOG_8S     (_BV(WDP3) | _BV(WDP0) | _BV(WDE))
#endif

#ifdef USE_TINY_TUNER

#define BOOTLOADER_SECTION __attribute__((section(".bootloader")))
#define BOOTLOADER_START_SECTION __attribute__((section(".bootloader.start")))
/*When using tiny tuner, stdlib is required, but we don't want that in the bootloader otherwise the size of the bootloader is
massively increased. To get around this, when in tinytuner mode, the bootloader is no longer in the main() function, but in a
new function called bootloader. This allows main() to be placed with the tinyTuner code along with stdlib. The main function
simply calls the bootloader function.

When not using tinyTuner, the main function is the bootloader and stdlib is not linked meaning this change has no effect of
non-tiny chips.
*/
int main(void) __attribute__ ((section (".init9"))) __attribute__ ((naked));
void bootloader(void) __attribute__ ((naked)) __attribute__ ((noreturn)) BOOTLOADER_START_SECTION;

#else
#define BOOTLOADER_SECTION
/* Function Prototypes */
/* The main function is in init9, which removes the interrupt vector table */
/* we don't need. It is also 'naked', which means the compiler does not    */
/* generate any entry or exit code itself. */
int main(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
#endif
void putch(char) BOOTLOADER_SECTION;
uint8_t getch(void) BOOTLOADER_SECTION;
static inline void getNch(uint8_t) BOOTLOADER_SECTION; /* "static inline" is a compiler hint to reduce code size */
void verifySpace() BOOTLOADER_SECTION;
static inline void flash_led(uint8_t) BOOTLOADER_SECTION;
uint8_t getLen() BOOTLOADER_SECTION;
static inline void watchdogReset() BOOTLOADER_SECTION;
void watchdogConfig(uint8_t x) BOOTLOADER_SECTION;
#ifdef SOFT_UART
void uartDelay() __attribute__ ((naked)) BOOTLOADER_SECTION;
#endif
void appStart() __attribute__ ((naked)) BOOTLOADER_SECTION;
#ifdef USE_TINY_TUNER
void tinyTuner() __attribute__ ((naked)) __attribute__ ((noreturn));
#endif

#if defined(__AVR_ATmega168__)
#define RAMSTART (0x100)
#define NRWWSTART (0x3800)
#elif defined(__AVR_ATmega328P__)
#define RAMSTART (0x100)
#define NRWWSTART (0x7000)
#elif defined(__AVR_ATmega162__)
#define RAMSTART (0x100)
#define NRWWSTART (0x3800)
#elif defined (__AVR_ATmega644P__)
#define RAMSTART (0x100)
#define NRWWSTART (0xE000)
#elif defined(__AVR_ATtiny24__)
#define RAMSTART (0x060)
#define NRWWSTART (0xE000)
#elif defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATtiny87__)
#define RAMSTART (0x100)
#define NRWWSTART (0x0000)
#elif defined(__AVR_ATmega1280__)
#define RAMSTART (0x200)
#define NRWWSTART (0xE000)
#elif defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
#define RAMSTART (0x100)
#define NRWWSTART (0x1800)
#endif

/* C zero initialises all global variables. However, that requires */
/* These definitions are NOT zero initialised, but that doesn't matter */
/* This allows us to drop the zero init code, saving us memory */


typedef union {
    uint16_t integer;
    uint8_t array[2];
}twoByte;

typedef union {
    uint32_t integer;
    uint8_t array[4];
}fourByte;

#define buff    ((uint8_t*)(RAMSTART))
#ifdef VIRTUAL_BOOT_PARTITION

#ifndef __AVR_ATtiny167__
#define rstVect (*(twoByte*)(RAMSTART+SPM_PAGESIZE*2+4))
#define wdtVect (*(twoByte*)(RAMSTART+SPM_PAGESIZE*2+6))
#define VECTOR_WORDS 1
#else
#define rstVect (*(fourByte*)(RAMSTART+SPM_PAGESIZE*2+4))
#define wdtVect (*(fourByte*)(RAMSTART+SPM_PAGESIZE*2+8))
#define VECTOR_WORDS 2
#endif

#if defined(__AVR_ATtiny167__)
#define WDT_VECT_START 20
#elif defined(__AVR_ATtiny87__)
#define WDT_VECT_START 10
#else
#define WDT_VECT_START 8
#endif

#endif

/* main program starts here */
#ifdef USE_TINY_TUNER
int main(void) {
  //newMain = bootloader;
  //(*newMain)();
  bootloader();
}

void bootloader(void) {
#else
int main(void) {
#endif
  uint8_t ch;

#ifdef USE_TINY_TUNER
  ch = pgm_read_byte_near(ver);
  if(ch == 255) tinyTuner();
  ch = pgm_read_byte_near(ver+1);
  OSCCAL = ch;
#endif

  /*
   * Making these local and in registers prevents the need for initializing
   * them, and also saves space because code no longer stores to memory.
   * (initializing address keeps the compiler happy, but isn't really
   *  necessary, and uses 4 bytes of flash.)
   */
  register uint16_t address = 0;
  register uint8_t  length;

  // After the zero init loop, this is the first code to run.
  //
  // This code makes the following assumptions:
  //  No interrupts will execute
  //  SP points to RAMEND
  //  r1 contains zero
  //
  // If not, uncomment the following instructions:
  //cli();

  asm volatile ("clr __zero_reg__");
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega162__)
  SP=RAMEND;  // This is done by hardware reset, except on these ones
#endif

  // Adaboot no-wait mod
#ifdef __AVR_ATmega162__
  ch = MCUCSR;
  ch &= 0x0F;
  MCUCSR &= 0xF0;
#else
  ch = MCUSR;
  MCUSR = 0;
#endif

#ifdef BOOTENTRY
  if(!(BOOTENTRY_PIN & _BV(BOOTENTRY))) appStart(); //if there is a bootloader entry pin, skip the bootloader if this is low.
#endif

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)// || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  uint8_t mask = (_BV(PORF) | _BV(EXTRF));
  if (!(ch & mask)) appStart(); //Power on reset loads bootloader as well - allows bootloader even if reset pin is disabled
#else
  if (!(ch & _BV(EXTRF))) appStart();
#endif
  
#if LED_START_FLASHES > 0
  // Set up Timer 1 for timeout counter
#if defined(__AVR_ATtiny85__)
  TCCR1 = _BV(CS13) | _BV(CS12); // div 2048 as it is only 8bit, so having to half TCNT1 value
#else
  TCCR1B = _BV(CS12) | _BV(CS10); // div 1024
#endif
#endif

#ifdef RS485_SUPPORT
  DE_DDR |= _BV(DE_BIT);
  DE_PORT &= ~_BV(DE_BIT);
#endif

#ifndef SOFT_UART
#ifdef __AVR_ATmega8__
  UCSRA = _BV(U2X); //Double speed mode USART
  UCSRB = _BV(RXEN) | _BV(TXEN);  // enable Rx & Tx
  UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);  // config USART; 8N1
  UBRRL = (uint8_t)( (F_CPU + BAUD_RATE * 4L) / (BAUD_RATE * 8L) - 1 );
#elif defined(__AVR_ATmega162__)
  UCSR0A = _BV(U2X0); //Double speed mode USART0
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(URSEL0) | _BV(UCSZ00) | _BV(UCSZ01);
  UBRR0L = (uint8_t)( (F_CPU + BAUD_RATE * 4L) / (BAUD_RATE * 8L) - 1 );
#elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
#define PRESCALE            16L
  LINCR = (1 << LSWRES); 
  LINBTR = _BV(LDISR) | (PRESCALE << LBT0);
  LINBRR = (((F_CPU * 10L / PRESCALE / BAUD_RATE) + 5L) / 10L) - 1;// ((((2 * F_CPU) + (BAUD_RATE * PRESCALE)) / (BAUD_RATE * 2L * PRESCALE)) - 1);
  LINERR = _BV(LFERR);
  LINCR = _BV(LENA) | _BV(LCMD2) | _BV(LCMD1) | _BV(LCMD0);
#else
  UCSR0A = _BV(U2X0); //Double speed mode USART0
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
  UBRR0L = (uint8_t)( (F_CPU + BAUD_RATE * 4L) / (BAUD_RATE * 8L) - 1 );
  
#endif
#endif

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  // Set up watchdog to trigger after 2s to give software UART a better chance.
  watchdogConfig(WATCHDOG_2S);
#else
  // Set up watchdog to trigger after 1s
  watchdogConfig(WATCHDOG_1S);
#endif

  /* Set LED pin as output */
  LED_DDR |= _BV(LED);
  
#ifdef SOFT_UART
  /* Set TX pin as output */
  UART_DDR |= _BV(UART_TX_BIT);
  UART_PORT |= _BV(UART_TX_BIT); //set high!
#endif

#if LED_START_FLASHES > 0
  /* Flash onboard LED to signal entering of bootloader */
  flash_led(LED_START_FLASHES * 2);
#endif

  /* Forever loop */
  for (;;) {
    /* get character from UART */
    ch = getch();

    if(ch == STK_GET_PARAMETER) {
#ifndef USE_TINY_TUNER
      unsigned char which = getch();
      verifySpace();
    //for tiny chips with tuner code, dont bother sending optiboot version - saves code.
      if (which == 0x82) {
    /*
     * Send optiboot version as "minor SW version"
     */
        putch(OPTIBOOT_MINVER);
      } else if (which == 0x81) {
    /*
     * Send optiboot version as "major SW version"
     */
        putch(OPTIBOOT_MAJVER);
      } else 
#else
      getch();
      verifySpace();
      if(1)
#endif
      {
    /*
     * GET PARAMETER returns a generic 0x03 reply for
         * other parameters - enough to keep Avrdude happy
     */
        putch(0x03);
      }
    }
    else if(ch == STK_SET_DEVICE) {
      // SET DEVICE is ignored
      getNch(20);
    }
    else if(ch == STK_SET_DEVICE_EXT) {
      // SET DEVICE EXT is ignored
      getNch(5);
    }
    else if(ch == STK_LOAD_ADDRESS) {
      // LOAD ADDRESS
      twoByte newAddress; //By using a union which combines a uint16_t with an array of two uint8_t's, code is MUCH smaller.
      newAddress.array[0] = getch();
      newAddress.array[1] = getch();
      //
      //newAddress = getch();
      //newAddress |= /*(newAddress & 0xff) |*/ (getch() << 8);
#ifdef RAMPZ
      // Transfer top bit to RAMPZ
      RAMPZ = (newAddress.integer & 0x8000) ? 1 : 0;
#endif
      newAddress.integer += newAddress.integer; // Convert from word address to byte address
      address = newAddress.integer;
      //newAddress += newAddress;
      //address = newAddress;
      verifySpace();
    }
    else if(ch == STK_UNIVERSAL) {
      // UNIVERSAL command is ignored
      getNch(4);
      putch(0x00);
    }
    /* Write memory, length is big endian and is in bytes */
    else if(ch == STK_PROG_PAGE) {
      // PROGRAM PAGE - we support flash programming only, not EEPROM
      uint8_t *bufPtr;
      uint16_t addrPtr;

      getch();            /* getlen() */
      length = getch();
      getch();

#ifndef VIRTUAL_BOOT_PARTITION
      // If we are in RWW section, immediately start page erase
      if (address < NRWWSTART) __boot_page_erase_short((uint16_t)(void*)address);
      
      //For tiny chips, this is never possible
#endif

      // While that is going on, read in page contents
      bufPtr = buff;
      do *bufPtr++ = getch();
      while (--length);

#ifdef SOFT_UART
      // Read command terminator, start reply
      verifySpace(); //Have to do this before starting page erase otherwise we miss the last byte from avrdude as CPU is disabled for erase
#endif

      // If we are in NRWW section, page erase has to be delayed until now.
      // Todo: Take RAMPZ into account
#ifndef VIRTUAL_BOOT_PARTITION
      if (address >= NRWWSTART) __boot_page_erase_short((uint16_t)(void*)address);
#else
      //For tiny chips, this is always the case
      __boot_page_erase_short((uint16_t)(void*)address);
#endif
      
#ifndef SOFT_UART
      // Read command terminator, start reply
      verifySpace();
#endif

      // If only a partial page is to be programmed, the erase might not be complete.
      // So check that here
      boot_spm_busy_wait();

#ifdef VIRTUAL_BOOT_PARTITION
      if ((uint16_t)(void*)address == 0) {
        // This is the reset vector page. We need to live-patch the code so the
        // bootloader runs.
        //
        // Move RESET vector to WDT vector
        #if VECTOR_WORDS == 1
        twoByte vect;
        vect.array[0] = buff[0];
        vect.array[1] = buff[1];
        twoByte tempWdtVect;
        tempWdtVect.array[0] = buff[WDT_VECT_START];
        tempWdtVect.array[1] = buff[WDT_VECT_START+1];
        #else
        fourByte vect;
        vect.array[0] = buff[0];
        vect.array[1] = buff[1];
        vect.array[2] = buff[2];
        vect.array[3] = buff[3];
        fourByte tempWdtVect;
        tempWdtVect.array[0] = buff[WDT_VECT_START];
        tempWdtVect.array[1] = buff[WDT_VECT_START+1];
        tempWdtVect.array[2] = buff[WDT_VECT_START+2];
        tempWdtVect.array[3] = buff[WDT_VECT_START+3];
        #endif
        
        // Add jump to bootloader at RESET vector
        buff[0] = RJUMP_COMMAND_LOW; //df
        buff[1] = RJUMP_COMMAND_HIGH; //ce   rjmp instruction
        #if VECTOR_WORDS == 2
        buff[2] = 0x00; //nop
        buff[3] = 0x00; //nop
        #endif
        
        wdtVect.integer = tempWdtVect.integer;
        rstVect.integer = vect.integer;
        
        #if VECTOR_WORDS == 1
        vect.integer -= (WDT_VECT_START/2); // Instruction is a relative jump (rjmp), so recalculate.
        #else
        buff[WDT_VECT_START+2] = vect.array[2]; //Instruction is a direct jump (jmp), so no need to recalculate.
        buff[WDT_VECT_START+3] = vect.array[3];
        #endif
        buff[WDT_VECT_START] = vect.array[0];
        buff[WDT_VECT_START+1] = vect.array[1];
        
        //EDIT: Although the code above looks A LOT longer, it compiles to be 24 bytes LESS - go unions!.
        /*uint16_t vect = buff[0] | (buff[1]<<8);
        rstVect = vect;
        wdtVect = buff[8] | (buff[9]<<8);
        vect -= 4; // Instruction is a relative jump (rjmp), so recalculate.
        buff[8] = vect & 0xff;
        buff[9] = vect >> 8;

        // Add jump to bootloader at RESET vector
        buff[0] = 0xbf;
        buff[1] = 0xce; // rjmp 0x1d80 instruction*/
      }
#endif

      // Copy buffer into programming buffer
      bufPtr = buff;
      addrPtr = (uint16_t)(void*)address;
      ch = SPM_PAGESIZE / 2;
      do {
        //uint16_t a;
        //a = *bufPtr++;
        //a |= (*bufPtr++) << 8;
        //__boot_page_fill_short((uint16_t)(void*)addrPtr,a);
        twoByte a; //Again by using a union, code length is slashed, this time by 16 bytes.
        a.array[0] = *bufPtr++;
        a.array[1] = *bufPtr++;
        __boot_page_fill_short((uint16_t)(void*)addrPtr,a.integer);
        addrPtr += 2;
      } while (--ch);

      // Write from programming buffer
      __boot_page_write_short((uint16_t)(void*)address);
      boot_spm_busy_wait();

#if defined(RWWSRE)
      // Reenable read access to flash
      boot_rww_enable();
#endif

    }
    /* Read memory block mode, length is big endian.  */
    else if(ch == STK_READ_PAGE) {
      // READ PAGE - we only read flash
      getch();            /* getlen() */
      length = getch();
      getch();

      verifySpace();
#ifdef VIRTUAL_BOOT_PARTITION
      do {
        // Undo vector patch in bottom page so verify passes
        if (address == 0)       ch=rstVect.array[0];
        else if (address == 1)  ch=rstVect.array[1];
        #if VECTOR_WORDS == 2
        else if (address == 2)  ch=rstVect.array[2];
        else if (address == 3)  ch=rstVect.array[3];
        #endif
        else if (address == WDT_VECT_START)     ch=wdtVect.array[0];
        else if (address == (WDT_VECT_START+1)) ch=wdtVect.array[1];
        #if VECTOR_WORDS == 2
        else if (address == (WDT_VECT_START+2)) ch=wdtVect.array[2];
        else if (address == (WDT_VECT_START+3)) ch=wdtVect.array[3];
        #endif
        else ch = pgm_read_byte_near(address);
        address++;
        putch(ch);
      } while (--length);
#else
#ifdef __AVR_ATmega1280__
//      do putch(pgm_read_byte_near(address++));
//      while (--length);
      do {
        uint8_t result;
        __asm__ ("elpm %0,Z\n":"=r"(result):"z"(address));
        putch(result);
        address++;
      }
      while (--length);
#else
      do putch(pgm_read_byte_near(address++));
      while (--length);
#endif
#endif
    }

    /* Get device signature bytes  */
    else if(ch == STK_READ_SIGN) {
      // READ SIGN - return what Avrdude wants to hear
      verifySpace();
      putch(SIGNATURE_0);
      putch(SIGNATURE_1);
      putch(SIGNATURE_2);
    }
    else if (ch == STK_LEAVE_PROGMODE) {
      // Adaboot no-wait mod
      watchdogConfig(WATCHDOG_16MS);
      verifySpace();
    }
    else {
      // This covers the response to commands like STK_ENTER_PROGMODE
      verifySpace();
    }
    putch(STK_OK);
  }
}

void putch(char ch) {
#ifndef SOFT_UART

#if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  while ((LINSIR & _BV(LBUSY)));
#else
  while (!(UCSR0A & _BV(UDRE0)));
#endif

#ifdef RS485_SUPPORT
  DE_PORT |= _BV(DE_BIT);
#if defined(__AVR_ATmega162__) || defined(__AVR_ATmega8__) 
  LED_PORT ^= _BV(LED);
#else
  LED_PIN |= _BV(LED);
#endif
  UCSR0A |= _BV(TXC0);
#endif

#if defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  LINDAT = ch;
#else
  UDR0 = ch;
#endif
  
#ifdef RS485_SUPPORT
  while(!(UCSR0A & _BV(TXC0)) );
  _delay_us(9);
  DE_PORT &= ~_BV(DE_BIT);
#if defined(__AVR_ATmega162__) || defined(__AVR_ATmega8__) 
  LED_PORT ^= _BV(LED);
#else
  LED_PIN |= _BV(LED);
#endif
#endif

#else
  __asm__ __volatile__ (
    "   com %[ch]\n" // ones complement, carry set
    "   sec\n"
    "1: brcc 2f\n"
    "   cbi %[uartPort],%[uartBit]\n"
    "   rjmp 3f\n"
    "2: sbi %[uartPort],%[uartBit]\n"
    "   nop\n"
    "3: rcall uartDelay\n"
    "   rcall uartDelay\n"
    "   rcall uartDelay\n"
    "   rcall uartDelay\n"
    "   lsr %[ch]\n"
    "   dec %[bitcnt]\n"
    "   brne 1b\n"
    :
    :
      [bitcnt] "d" ((uint8_t)10),
      [ch] "r" (ch),
      [uartPort] "I" (_SFR_IO_ADDR(UART_PORT)),
      [uartBit] "I" (UART_TX_BIT)
    :
      "r25"
  );
#endif
}

uint8_t getch(void) {
  uint8_t ch;

#ifdef LED_DATA_FLASH
#if defined(__AVR_ATmega162__) || defined(__AVR_ATmega8__) 
  LED_PORT ^= _BV(LED);
#else
  LED_PIN |= _BV(LED);
#endif
#endif

#ifdef SOFT_UART
  __asm__ __volatile__ (
    "1: sbic  %[uartPin],%[uartBit]\n"  // Wait for start edge
    "   rjmp  1b\n"
    "   rcall uartDelay\n"          // Get to 0.25 of start bit 
    "2: rcall uartDelay\n"              // Wait 0.25 bit period
    "   rcall uartDelay\n"              // Wait 0.25 bit period
    "   rcall uartDelay\n"              // Wait 0.25 bit period
    "   rcall uartDelay\n"              // Wait 0.25 bit period
    "   clc\n"
    "   sbic  %[uartPin],%[uartBit]\n"
    "   sec\n"      
    "   ror   %[ch]\n"                    
    "   dec   %[bitCnt]\n"
    "   breq  3f\n"
    "   rjmp  2b\n"
    "3:\n"
    "   wdr\n" //Someone forgot to reset the watchdog!
    "   rcall uartDelay\n"              // Wait 0.25 bit period
    "   rcall uartDelay\n"              // Wait 0.25 bit period
    "   rcall uartDelay\n"              // Wait 0.25 bit period
    :
      [ch] "=r" (ch)
    :
      "d" ((uint8_t)0),
      [bitCnt] "d" ((uint8_t)8),
      #ifdef UART_RX_PIN
      [uartPin] "I" (_SFR_IO_ADDR(UART_RX_PIN)), //rx is on a different port.
      #else
      [uartPin] "I" (_SFR_IO_ADDR(UART_PIN)),
      #endif
      [uartBit] "I" (UART_RX_BIT)
    :
      "r25"
  ); //Edit: no longer waiting through stop bit to try and give it the upper hand if there are lots of bits being sent.
#elif defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)
  while(!(LINSIR & _BV(LRXOK)))
    ;
  if (!(LINSIR & _BV(LERR))) {
      /*
       * A Framing Error indicates (probably) that something is talking
       * to us at the wrong bit rate.  Assume that this is because it
       * expects to be talking to the application, and DON'T reset the
       * watchdog.  This should cause the bootloader to abort and run
       * the application "soon", if it keeps happening.  (Note that we
       * don't care that an invalid char is returned...)
       */
    watchdogReset();
  }
  
  ch = LINDAT;
#else
  while(!(UCSR0A & _BV(RXC0)))
    ;
  if (!(UCSR0A & _BV(FE0))) {
      /*
       * A Framing Error indicates (probably) that something is talking
       * to us at the wrong bit rate.  Assume that this is because it
       * expects to be talking to the application, and DON'T reset the
       * watchdog.  This should cause the bootloader to abort and run
       * the application "soon", if it keeps happening.  (Note that we
       * don't care that an invalid char is returned...)
       */
    watchdogReset();
  }
  
  ch = UDR0;
#endif

#ifdef LED_DATA_FLASH
#if defined(__AVR_ATmega162__) || defined(__AVR_ATmega8__) 
  LED_PORT ^= _BV(LED);
#else
  LED_PIN |= _BV(LED);
#endif
#endif

  return ch;
}

#ifdef SOFT_UART
// AVR350 equation: #define UART_B_VALUE (((F_CPU/BAUD_RATE)-23)/6)
// //Adding 6 to numerator simulates nearest rounding for more accurate baud rates
//#define UART_B_VALUE (((F_CPU/BAUD_RATE)-23)/6)
#define UART_B_VALUE (((F_CPU/BAUD_RATE)-29)/12)
#if UART_B_VALUE > 255
#error Baud rate too slow for soft UART
#endif

void uartDelay() {
  __asm__ __volatile__ (
    "ldi r25,%[count]\n"
    "1:dec r25\n"
    "brne 1b\n"
    "ret\n"
    ::[count] "M" (UART_B_VALUE):"r25"
  );
}
#endif

void getNch(uint8_t count) {
  do getch(); while (--count);
  verifySpace();
}

void verifySpace() {
  if (getch() != CRC_EOP) {
    watchdogConfig(WATCHDOG_16MS);    // shorten WD timeout
    while (1)                  // and busy-loop so that WD causes
      ;                      //  a reset and app start.
  }
  putch(STK_INSYNC);
}

#if LED_START_FLASHES > 0
void flash_led(uint8_t count) {
  do {
#ifdef __AVR_ATmega162__
    TCNT1 = -(F_CPU/(1024*16));
    TIFR = _BV(TOV1) | (TIFR & 0x17);
    while(!(TIFR & _BV(TOV1)));
#elif defined(__AVR_ATtiny85__)
    TCNT1 = -(char)(F_CPU/(2048U*16U));
    while(!(TIFR & _BV(TOV1)));
    TIFR = _BV(TOV1); //For tiny85, sbi() doesn't reach this register, so it saves memory to use an assignment rather than a bitwise or.
#else
    TCNT1 = -(F_CPU/(1024*16));
    //TIFR1 = _BV(TOV1);
    TIFR1 |= _BV(TOV1); //at boot TIFR1 is 0, so can just set the TOV1 bit - save memory.
    while(!(TIFR1 & _BV(TOV1)));
#endif
#if defined(__AVR_ATmega162__) || defined(__AVR_ATmega8__) 
    LED_PORT ^= _BV(LED);
#else
    LED_PIN |= _BV(LED);
#endif
    watchdogReset();
  } while (--count);
}
#endif

// Watchdog functions. These are only safe with interrupts turned off.
void watchdogReset() {
  __asm__ __volatile__ (
    "wdr\n"
  );
}

#if !defined(WDTCSR) && defined(WDTCR)
  #define WDTCSR WDTCR
#endif

void watchdogConfig(uint8_t x) {
  WDTCSR = _BV(WDCE) | _BV(WDE);
  WDTCSR = x;
}

void appStart() {
  watchdogConfig(WATCHDOG_OFF);
  __asm__ __volatile__ (
#ifdef VIRTUAL_BOOT_PARTITION
    // Jump to WDT vector
    "ldi r30,%[wdr]\n"
    "clr r31\n"
#else
    // Jump to RST vector
    "clr r30\n"
    "clr r31\n"
#endif
    "ijmp\n"
#ifdef VIRTUAL_BOOT_PARTITION
    ::[wdr] "M" (WDT_VECT_START/2)
#endif
  );
}

//TinyTuner Code.

#ifdef USE_TINY_TUNER
#include "tinyTuner.c"
#endif


