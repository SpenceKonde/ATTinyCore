/*
 * pin_defs.h
 * optiboot helper defining the default pin assignments (LED, SOFT_UART)
 * for the various chips that are supported.  This also has some ugly macros
 * for selecting among various UARTs and LED possibilities using command-line
 * defines like "UART=2 LED=B5"
 *
 * Copyright 2013-2015 by Bill Westfield.
 * Copyright 2010 by Peter Knight.
 * This software is licensed under version 2 of the Gnu Public Licence.
 * See optiboot.c for details.
 */

/*------------------------------------------------------------------------ */
#if    defined(__AVR_ATmega168__)	\
    || defined(__AVR_ATmega168P__)	\
    || defined(__AVR_ATmega328__)	\
    || defined(__AVR_ATmega328P__)	\
    || defined(__AVR_ATmega88)		\
    || defined(__AVR_ATmega8__)		\
    || defined(__AVR_ATmega88__)
/*------------------------------------------------------------------------ */

/* Onboard LED is connected to pin PB5 in Arduino NG, Diecimila, and Duemilanove
 */ 
#if !defined(LED)
#define LED B5
#endif

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*
 * Handle devices with up to 4 uarts (eg m1280.)  Rather inelegantly.
 * Note that mega8/m32 still needs special handling, because ubrr is handled
 * differently.
 */
#ifndef SOFT_UART
#if UART == 0
#if defined(UDR0)
# define UART_SRA UCSR0A
# define UART_SRB UCSR0B
# define UART_SRC UCSR0C
# define UART_SRL UBRR0L
# define UART_UDR UDR0
#elif defined(UDR)
# define UART_SRA UCSRA
# define UART_SRB UCSRB
# define UART_SRC UCSRC
# define UART_SRL UBRRL
# define UART_UDR UDR
#elif defined(LINDAT)
# define LIN_UART 1
# define UART_SRA UCSRA
# define UART_SRB UCSRB
# define UART_SRC UCSRC
# define UART_SRL UBRRL
# define UART_UDR LINDAT
#else
# error UART == 0, but no UART0 on device
#endif
#elif UART == 1
#if !defined(UDR1)
#error UART == 1, but no UART1 on device
#endif
# define UART_SRA UCSR1A
# define UART_SRB UCSR1B
# define UART_SRC UCSR1C
# define UART_SRL UBRR1L
# define UART_UDR UDR1
#elif UART == 2
#if !defined(UDR2)
#error UART == 2, but no UART2 on device
#endif
# define UART_SRA UCSR2A
# define UART_SRB UCSR2B
# define UART_SRC UCSR2C
# define UART_SRL UBRR2L
# define UART_UDR UDR2
#elif UART == 3
#if !defined(UDR3)
#error UART == 3, but no UART3 on device
#endif
# define UART_SRA UCSR3A
# define UART_SRB UCSR3B
# define UART_SRC UCSR3C
# define UART_SRL UBRR3L
# define UART_UDR UDR3
#endif
#endif //end #ifndef SOFT_UART

#if    defined(__AVR_ATmega8__)		\
    || defined (__AVR_ATmega32__)	\
    || defined (__AVR_ATmega16__)
  //Name conversion R.Wiersma
  #define UCSR0A	UCSRA
  #define UDR0 		UDR
  #define UDRE0 	UDRE
  #define RXC0		RXC
  #define FE0           FE
  #define TIFR1 	TIFR
  #define WDTCSR	WDTCR
#endif

#if    defined (__AVR_ATmega32__)	\
    || defined (__AVR_ATmega16__)
  #define WDCE		WDTOE
#endif


/*------------------------------------------------------------------------ */
/* Sanguino support (and other 40pin DIP cpus) */
/*------------------------------------------------------------------------ */
/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__) 
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B0
#endif

/* Fix register names */
#if defined(__AVR_ATmega8515__)
#define UCSR0A  UCSRA
#define UDR0    UDR
#define UDRE0   UDRE
#define RXC0    RXC
#define FE0     FE
#endif
#define TIFR1   TIFR
#define WDTCSR  WDTCR

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

#if    defined(__AVR_ATmega8535__)		\
    || defined(__AVR_ATmega16__)		\
    || defined(__AVR_ATmega32__)		\
    || defined(__AVR_ATmega164A__)		\
    || defined(__AVR_ATmega164P__)		\
    || defined(__AVR_ATmega324A__)		\
    || defined(__AVR_ATmega324P__)		\
    || defined(__AVR_ATmega324PA__)		\
    || defined(__AVR_ATmega644__)		\
    || defined(__AVR_ATmega644P__)		\
    || defined(__AVR_ATmega1284__)		\
    || defined(__AVR_ATmega1284P__)
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B0
#endif

#if     defined(__AVR_ATmega8535__)	\
    ||  defined(__AVR_ATmega16__)	\
    ||  defined(__AVR_ATmega32__)
/* Fix register names */
#define UCSR0A  UCSRA
#define UDR0    UDR
#define UDRE0   UDRE
#define RXC0    RXC
#define FE0     FE
#define TIFR1   TIFR
#define WDTCSR  WDTCR
#endif
#if defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__)
#define WDCE        WDTOE
#endif

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega324PB__)
/*------------------------------------------------------------------------ */

#if !defined(LED)
#define LED         B0
#endif

/* Fix missing register names */
#define MPCM0   MPCM
#define U2X0    U2X
#define UPE0    UPE
#define DOR0    DOR
#define FE0     FE
#define UDRE0   UDRE
#define TXC0    TXC
#define RXC0    RXC

#define TXB80   TXB8
#define RXB80   RXB8
#define UCSZ02  UCSZ2
#define TXEN0   TXEN
#define RXEN0   RXEN
#define UDRIE0  UDRIE
#define TXCIE0  TXCIE
#define RXCIE0  RXCIE

#define UCPOL0  UCPOL
#define UCSZ00  UCSZ0
#define UCSZ01  UCSZ1
#define USBS0   USBS
#define UPM00   UPM0
#define UPM01   UPM1
#define UMSEL00 UMSEL0
#define UMSEL01 UMSEL1

#define MPCM1   MPCM
#define U2X1    U2X
#define UPE1    UPE
#define DOR1    DOR
#define FE1     FE
#define UDRE1   UDRE
#define TXC1    TXC
#define RXC1    RXC

#define TXB81   TXB8
#define RXB81   RXB8
#define UCSZ12  UCSZ2
#define TXEN1   TXEN
#define RXEN1   RXEN
#define UDRIE1  UDRIE
#define TXCIE1  TXCIE
#define RXCIE1  RXCIE

#define UCPOL1  UCPOL
#define UCSZ10  UCSZ0
#define UCSZ11  UCSZ1
#define USBS1   USBS
#define UPM10   UPM0
#define UPM11   UPM1
#define UMSEL10 UMSEL0
#define UMSEL11 UMSEL1

#define MPCM2   MPCM
#define U2X2    U2X
#define UPE2    UPE
#define DOR2    DOR
#define FE2     FE
#define UDRE2   UDRE
#define TXC2    TXC
#define RXC2    RXC

#define TXB82   TXB8
#define RXB82   RXB8
#define UCSZ22  UCSZ2
#define TXEN2   TXEN
#define RXEN2   RXEN
#define UDRIE2  UDRIE
#define TXCIE2  TXCIE
#define RXCIE2  RXCIE

#define UCPOL2  UCPOL
#define UCSZ20  UCSZ0
#define UCSZ21  UCSZ1
#define USBS2   USBS
#define UPM20   UPM0
#define UPM21   UPM1
#define UMSEL20 UMSEL0
#define UMSEL21 UMSEL1

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

#define TIFR1 TIFR
#define WDTCSR WDTCR
//#define MCUSR MCUCSR
//#define SPMCR SPMCSR

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*------------------------------------------------------------------------ */
/* Mega support */
#if    defined(__AVR_ATmega640__)	\
    || defined(__AVR_ATmega1280__)	\
    || defined(__AVR_ATmega2560__)
/*------------------------------------------------------------------------ */
/* Onboard LED is connected to pin PB7 on Arduino Mega */ 
#if !defined(LED)
#define LED         B7
#endif

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega169__) 
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

/* Fix register names */
#define UCSR0A UCSRA
#define UCSR0B UCSRB
#define UCSR0C UCSRC
#define UBRR0L UBRRL
#define UDR0 UDR
#define UDRE0 UDRE
#define RXC0 RXC
#define FE0 FE
#define WDTCSR WDTCR
#define U2X0 U2X
#define RXEN0 RXEN
#define TXEN0 TXEN
#define UCSZ00 UCSZ0
#define UCSZ01 UCSZ1

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega169P__) || defined(__AVR_ATmega329__) || defined(__AVR_ATmega329P__) \
|| defined(__AVR_ATmega3290__) || defined(__AVR_ATmega3290P__) || defined(__AVR_ATmega649__) \
|| defined(__AVR_ATmega649P__) || defined(__AVR_ATmega6490__) || defined(__AVR_ATmega6490P__)
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

/* Fix register names */
#define WDTCSR WDTCR

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif
/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2561__)
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega169__) 
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

/* Fix register names */
#define UCSR0A UCSRA
#define UCSR0B UCSRB
#define UCSR0C UCSRC
#define UBRR0L UBRRL
#define UDR0 UDR
#define UDRE0 UDRE
#define RXC0 RXC
#define FE0 FE
#define WDTCSR WDTCR
#define U2X0 U2X
#define RXEN0 RXEN
#define TXEN0 TXEN
#define UCSZ00 UCSZ0
#define UCSZ01 UCSZ1

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATmega169P__) || defined(__AVR_ATmega329__) || defined(__AVR_ATmega329P__) \
|| defined(__AVR_ATmega3290__) || defined(__AVR_ATmega3290P__) || defined(__AVR_ATmega649__) \
|| defined(__AVR_ATmega649P__) || defined(__AVR_ATmega6490__) || defined(__AVR_ATmega6490P__)
/*------------------------------------------------------------------------ */
#if !defined(LED)
#define LED         B5
#endif

/* Fix register names */
#define WDTCSR WDTCR

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny1634__)
/*------------------------------------------------------------------------ */
/* LED is on C0 */
#if !defined(LED)
#define LED 	C0
#endif
#define TIFR1 TIFR

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 2
#define UART_RX_BIT 3
#endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny841__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED 	B2
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 2
#define UART_RX_BIT 3
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny441__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B2
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 2
#define UART_RX_BIT 3
#endif
#endif
/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny84__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B2
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 1
#define UART_RX_BIT 2
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny44__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B2
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 1
#define UART_RX_BIT 2
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny85__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B2
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTB
#define UART_PIN    PINB
#define UART_DDR    DDRB
#define UART_TX_BIT 0
#define UART_RX_BIT 1
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny45__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B2
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTB
#define UART_PIN    PINB
#define UART_DDR    DDRB
#define UART_TX_BIT 0
#define UART_RX_BIT 1
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny861__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B3
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 6
#define UART_RX_BIT 7
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny461__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B3
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 6
#define UART_RX_BIT 7
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny88__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B5
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 6
#define UART_RX_BIT 7
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny48__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED   B5
#endif

#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 6
#define UART_RX_BIT 7
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny167__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED 	A3
#endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny87__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED 	A3
#endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny828__)
/*------------------------------------------------------------------------ */
/* LED is on B2 */
#if !defined(LED)
#define LED 	B0
#endif

#define UDRE0 UDRE 
#define FE0 FE 
#define RXC0 RXC
#define UCSZ00 UCSZ0
#define UCSZ01 UCSZ1 
#define TXEN0 TXEN
#define RXEN0 RXEN
#define U2X0 U2X


#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT 2
#define UART_RX_BIT 3
#endif
#endif

/*
 * ------------------------------------------------------------------------
 * A bunch of macros to enable the LED to be specifed as "B5" for bit 5 
 * of port B, and similar.
 * We define symbols for all the legal combination of port/bit on a chip,
 * and do pre-processor tests to see if there's a match.  This ends up
 * being very verbose, but it is pretty easy to generate semi-automatically.
 * (We wouldn't need this if the preprocessor could do string compares.)
 */

// Symbols for each PortA bit.
#define A0 0x100
#define A1 0x101
#define A2 0x102
#define A3 0x103
#define A4 0x104
#define A5 0x105
#define A6 0x106
#define A7 0x107
// If there is no PORTA on this chip, don't allow these to be used
//   (and indicate the error by redefining LED)
#if !defined(PORTA)
#if LED >= A0 && LED <= A7
#undef LED
#define LED -1
#endif
#endif

#define B0 0x200
#define B1 0x201
#define B2 0x202
#define B3 0x203
#define B4 0x204
#define B5 0x205
#define B6 0x206
#define B7 0x207
#if !defined(PORTB)
#if LED >= B0 && LED <= B7
#undef LED
#define LED -1
#endif
#endif

#define C0 0x300
#define C1 0x301
#define C2 0x302
#define C3 0x303
#define C4 0x304
#define C5 0x305
#define C6 0x306
#define C7 0x307
#if !(defined(PORTC))
#if LED >= C0 && LED <= C7
#undef LED
#define LED -1
#endif
#endif

#define D0 0x400
#define D1 0x401
#define D2 0x402
#define D3 0x403
#define D4 0x404
#define D5 0x405
#define D6 0x406
#define D7 0x407
#if !(defined(PORTD))
#if LED >= D0 && LED <= D7
#undef LED
#define LED -1
#endif
#endif

#define E0 0x500
#define E1 0x501
#define E2 0x502
#define E3 0x503
#define E4 0x504
#define E5 0x505
#define E6 0x506
#define E7 0x507
#if !(defined(PORTE))
#if LED >= E0 && LED <= E7
#undef LED
#define LED -1
#endif
#endif

#define F0 0x600
#define F1 0x601
#define F2 0x602
#define F3 0x603
#define F4 0x604
#define F5 0x605
#define F6 0x606
#define F7 0x607
#if !(defined(PORTF))
#if LED >= F0 && LED <= F7
#undef LED
#define LED -1
#endif
#endif

#define G0 0x700
#define G1 0x701
#define G2 0x702
#define G3 0x703
#define G4 0x704
#define G5 0x705
#define G6 0x706
#define G7 0x707
#if !defined(PORTG)
#if LED >= G0 && LED <= G7
#undef LED
#define LED -1
#endif
#endif

#define H0 0x800
#define H1 0x801
#define H2 0x802
#define H3 0x803
#define H4 0x804
#define H5 0x805
#define H6 0x806
#define H7 0x807
#if !(defined(PORTH))
#if LED >= H0 && LED <= H7
#undef LED
#define LED -1
#endif
#endif

#define J0 0xA00
#define J1 0xA01
#define J2 0xA02
#define J3 0xA03
#define J4 0xA04
#define J5 0xA05
#define J6 0xA06
#define J7 0xA07
#if !(defined(PORTJ))
#if LED >= J0 && LED <= J7
#undef LED
#define LED -1
#endif
#endif

#define K0 0xB00
#define K1 0xB01
#define K2 0xB02
#define K3 0xB03
#define K4 0xB04
#define K5 0xB05
#define K6 0xB06
#define K7 0xB07
#if !(defined(PORTK))
#if LED >= K0 && LED <= K7
#undef LED
#define LED -1
#endif
#endif

#define L0 0xC00
#define L1 0xC01
#define L2 0xC02
#define L3 0xC03
#define L4 0xC04
#define L5 0xC05
#define L6 0xC06
#define L7 0xC07
#if !(defined(PORTL))
#if LED >= L0 && LED <= L7
#undef LED
#define LED -1
#endif
#endif


/*
 * A statement like "#if LED == B0" will evaluation (in the preprocessor)
 * to #if C0 == B0, and then to #if 0x301 == 0x201
 */
#if LED == B0
#define LED_NAME "B0"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB0
#elif LED == B1
#define LED_NAME "B1"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB1
#elif LED == B2
#define LED_NAME "B2"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB2
#elif LED == B3
#define LED_NAME "B3"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB3
#elif LED == B4
#define LED_NAME "B4"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB4
#elif LED == B5
#define LED_NAME "B5"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB5
#elif LED == B6
#define LED_NAME "B6"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB6
#elif LED == B7
#define LED_NAME "B7"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED	    PINB7

#elif LED == C0
#define LED_NAME "C0"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC0
#elif LED == C1
#define LED_NAME "C1"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC1
#elif LED == C2
#define LED_NAME "C2"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC2
#elif LED == C3
#define LED_NAME "C3"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC3
#elif LED == C4
#define LED_NAME "C4"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC4
#elif LED == C5
#define LED_NAME "C5"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC5
#elif LED == C6
#define LED_NAME "C6"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC6
#elif LED == C7
#define LED_NAME "C7"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED	    PINC7

#elif LED == D0
#define LED_NAME "D0"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND0
#elif LED == D1
#define LED_NAME "D1"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND1
#elif LED == D2
#define LED_NAME "D2"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND2
#elif LED == D3
#define LED_NAME "D3"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND3
#elif LED == D4
#define LED_NAME "D4"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND4
#elif LED == D5
#define LED_NAME "D5"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND5
#elif LED == D6
#define LED_NAME "D6"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND6
#elif LED == D7
#define LED_NAME "D7"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED	    PIND7

#elif LED == E0
#define LED_NAME "E0"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE0
#elif LED == E1
#define LED_NAME "E1"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE1
#elif LED == E2
#define LED_NAME "E2"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE2
#elif LED == E3
#define LED_NAME "E3"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE3
#elif LED == E4
#define LED_NAME "E4"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE4
#elif LED == E5
#define LED_NAME "E5"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE5
#elif LED == E6
#define LED_NAME "E6"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE6
#elif LED == E7
#define LED_NAME "E7"
#undef LED
#define LED_DDR     DDRE
#define LED_PORT    PORTE
#define LED_PIN     PINE
#define LED	    PINE7

#elif LED == F0
#define LED_NAME "F0"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF0
#elif LED == F1
#define LED_NAME "F1"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF1
#elif LED == F2
#define LED_NAME "F2"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF2
#elif LED == F3
#define LED_NAME "F3"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF3
#elif LED == F4
#define LED_NAME "F4"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF4
#elif LED == F5
#define LED_NAME "F5"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF5
#elif LED == F6
#define LED_NAME "F6"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF6
#elif LED == F7
#define LED_NAME "F7"
#undef LED
#define LED_DDR     DDRF
#define LED_PORT    PORTF
#define LED_PIN     PINF
#define LED	    PINF7

#elif LED == G0
#define LED_NAME "G0"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING0
#elif LED == G1
#define LED_NAME "G1"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING1
#elif LED == G2
#define LED_NAME "G2"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING2
#elif LED == G3
#define LED_NAME "G3"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING3
#elif LED == G4
#define LED_NAME "G4"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING4
#elif LED == G5
#define LED_NAME "G5"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING5
#elif LED == G6
#define LED_NAME "G6"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING6
#elif LED == G7
#define LED_NAME "G7"
#undef LED
#define LED_DDR     DDRG
#define LED_PORT    PORTG
#define LED_PIN     PING
#define LED	    PING7

#elif LED == H0
#define LED_NAME "H0"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH0
#elif LED == H1
#define LED_NAME "H1"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH1
#elif LED == H2
#define LED_NAME "H2"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH2
#elif LED == H3
#define LED_NAME "H3"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH3
#elif LED == H4
#define LED_NAME "H4"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH4
#elif LED == H5
#define LED_NAME "H5"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH5
#elif LED == H6
#define LED_NAME "H6"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH6
#elif LED == H7
#define LED_NAME "H7"
#undef LED
#define LED_DDR     DDRH
#define LED_PORT    PORTH
#define LED_PIN     PINH
#define LED	    PINH7

#elif LED == J0
#define LED_NAME "J0"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ0
#elif LED == J1
#define LED_NAME "J1"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ1
#elif LED == J2
#define LED_NAME "J2"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ2
#elif LED == J3
#define LED_NAME "J3"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ3
#elif LED == J4
#define LED_NAME "J4"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ4
#elif LED == J5
#define LED_NAME "J5"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ5
#elif LED == J6
#define LED_NAME "J6"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ6
#elif LED == J7
#define LED_NAME "J7"
#undef LED
#define LED_DDR     DDRJ
#define LED_PORT    PORTJ
#define LED_PIN     PINJ
#define LED	    PINJ7

#elif LED == K0
#define LED_NAME "K0"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK0
#elif LED == K1
#define LED_NAME "K1"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK1
#elif LED == K2
#define LED_NAME "K2"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK2
#elif LED == K3
#define LED_NAME "K3"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK3
#elif LED == K4
#define LED_NAME "K4"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK4
#elif LED == K5
#define LED_NAME "K5"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK5
#elif LED == K6
#define LED_NAME "K6"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK6
#elif LED == K7
#define LED_NAME "K7"
#undef LED
#define LED_DDR     DDRK
#define LED_PORT    PORTK
#define LED_PIN     PINK
#define LED	    PINK7

#elif LED == L0
#define LED_NAME "L0"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL0
#elif LED == L1
#define LED_NAME "L1"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL1
#elif LED == L2
#define LED_NAME "L2"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL2
#elif LED == L3
#define LED_NAME "L3"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL3
#elif LED == L4
#define LED_NAME "L4"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL4
#elif LED == L5
#define LED_NAME "L5"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL5
#elif LED == L6
#define LED_NAME "L6"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL6
#elif LED == L7
#define LED_NAME "L7"
#undef LED
#define LED_DDR     DDRL
#define LED_PORT    PORTL
#define LED_PIN     PINL
#define LED	    PINL7

#elif LED == A0
#define LED_NAME "A0"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA0
#elif LED == A1
#define LED_NAME "A1"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA1
#elif LED == A2
#define LED_NAME "A2"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA2
#elif LED == A3
#define LED_NAME "A3"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA3
#elif LED == A4
#define LED_NAME "A4"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA4
#elif LED == A5
#define LED_NAME "A5"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA5
#elif LED == A6
#define LED_NAME "A6"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA6
#elif LED == A7
#define LED_NAME "A7"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED	    PINA7

#else
// Stop compilation right away, so we don't get more errors.
#if LED == -1
#error Unrecognized LED name.  Should be like "B5"
// Stop compilation right away, so we don't get more errors.
#pragma GCC diagnostic warning "-Wfatal-errors"
#error Nonexistent LED PORT.  Check datasheet.
#endif
#pragma GCC diagnostic warning "-Wfatal-errors"
#error Unrecognized LED name.  Should be like "B5"
#endif
