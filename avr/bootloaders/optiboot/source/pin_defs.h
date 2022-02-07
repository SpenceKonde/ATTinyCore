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
      #error UART == 0, but no UART0 on device
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
  #endif //end #ifndef SOFT_UART
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny1634__)
/*------------------------------------------------------------------------ */
  /* LED is on C0 */
  #if !defined(LED)
    #define LED     C0
  #endif

  #define TIFR1 TIFR

  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN) && defined(TXPIN) && defined(RXPIN))
      #error "Software Serial requested but pins not specified, and no default pins because has hardware serial."
    #endif
  #endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny841__)
/*------------------------------------------------------------------------ */
  /* LED is on B2 */
  #if !defined(LED)
    #if defined(REMAP_UART) && REMAP_UART == 1
      #define LED     A2
    #else
      #define LED     B2
    #endif
  #endif

  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN) && defined(TXPIN) && defined(RXPIN))
      #error "Software Serial requested but pins not specified, and no default pins because has hardware serial."
    #endif
  #endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny441__)
/*------------------------------------------------------------------------ */
  /* LED is on B2 */
  #if !defined(LED)
    #if defined(REMAP_UART) && REMAP_UART == 1
      #define LED     A2
    #else
      #define LED     B2
    #endif
  #endif

  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN) && defined(TXPIN) && defined(RXPIN))
      #error "Software Serial requested but pins not specified, and no default pins because has hardware serial."
    #endif
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
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN))
      #define UART_PORT   PORTA
      #define UART_PIN    PINA
      #define UART_DDR    DDRA
    #endif
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 1
      #define UART_RX_BIT 2
    #endif
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
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN))
      #define UART_PORT   PORTA
      #define UART_PIN    PINA
      #define UART_DDR    DDRA
    #endif
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 1
      #define UART_RX_BIT 2
    #endif
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
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 0
      #define UART_RX_BIT 1
    #endif
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
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 0
      #define UART_RX_BIT 1
    #endif
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
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN))
      #define UART_PORT   PORTA
      #define UART_PIN    PINA
      #define UART_DDR    DDRA
    #endif
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 6
      #define UART_RX_BIT 7
    #endif
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
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN))
      #define UART_PORT   PORTA
      #define UART_PIN    PINA
      #define UART_DDR    DDRA
    #endif
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 6
      #define UART_RX_BIT 7
    #endif
  #endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny88__)
/*------------------------------------------------------------------------ */
  /* LED is on PB5 (arduino pin 13, to match the 328p boards?) */
  #if !defined(LED)
    #define LED   B5
  #endif

  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN))
      #define UART_PORT   PORTD
      #define UART_PIN    PIND
      #define UART_DDR    DDRD
    #endif
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 6
      #define UART_RX_BIT 7
    #endif
  #endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny48__)
/*------------------------------------------------------------------------ */
/* LED is on B5 */
  #if !defined(LED)
    #define LED   B5
  #endif

  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN))
      #define UART_PORT   PORTD
      #define UART_PIN    PIND
      #define UART_DDR    DDRD
    #endif
    #if defined(TXPIN) && defined(RXPIN)
      #define UART_TX_BIT TXPIN
      #define UART_RX_BIT RXPIN
    #elif defined(TXPIN) || defined(RXPIN)
      #error "If custom pins are used for soft UART, both pins must be specified, not just one or the other."
    #else
      #define UART_TX_BIT 6
      #define UART_RX_BIT 7
    #endif
  #endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny167__)
/*------------------------------------------------------------------------ */
  /* LED is on B2 */
  #if !defined(LED)
    #define LED     A6
  #endif
  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN) && defined(TXPIN) && defined(RXPIN))
      #error "Software Serial requested but pins not specified, and no default pins because has hardware LIN serial."
    #endif
  #endif
#endif

/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny87__)
/*------------------------------------------------------------------------ */
  /* LED is on B2 */
  #if !defined(LED)
  #define LED     A6
  #endif

  #ifdef SOFT_UART
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN) && defined(TXPIN) && defined(RXPIN))
      #error "Software Serial requested but pins not specified, and no default pins because has hardware LIN serial."
    #endif
  #endif
#endif


/*------------------------------------------------------------------------ */
#if defined(__AVR_ATtiny828__)
/*------------------------------------------------------------------------ */
  /* LED is on B2 */
  #if !defined(LED)
    #define LED     B0
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
    #if !(defined(UART_PORT) && defined(UART_DDR) && defined(UART_PIN) && defined(TXPIN) && defined(RXPIN))
      #error "Software Serial requested but pins not specified, and no default pins because has hardware serial."
    #endif
  #endif
#endif

/*
 * ------------------------------------------------------------------------
 * A bunch of macros to enable the LED to be specified as "B5" for bit 5
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
#warning "LED set for PORTA, this part has no PORTA. Omitting LED"
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
#warning "LED set for PORTB, this part has no PORTB. Omitting LED"
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
#warning "LED set for PORTC, this part has no PORTC. Omitting LED"
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
#warning "LED set for PORTD, this part has no PORTD. Omitting LED"
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
#define LED     PINB0
#elif LED == B1
#define LED_NAME "B1"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB1
#elif LED == B2
#define LED_NAME "B2"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB2
#elif LED == B3
#define LED_NAME "B3"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB3
#elif LED == B4
#define LED_NAME "B4"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB4
#elif LED == B5
#define LED_NAME "B5"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB5
#elif LED == B6
#define LED_NAME "B6"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB6
#elif LED == B7
#define LED_NAME "B7"
#undef LED
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED     PINB7

#elif LED == C0
#define LED_NAME "C0"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC0
#elif LED == C1
#define LED_NAME "C1"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC1
#elif LED == C2
#define LED_NAME "C2"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC2
#elif LED == C3
#define LED_NAME "C3"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC3
#elif LED == C4
#define LED_NAME "C4"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC4
#elif LED == C5
#define LED_NAME "C5"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC5
#elif LED == C6
#define LED_NAME "C6"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC6
#elif LED == C7
#define LED_NAME "C7"
#undef LED
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED_PIN     PINC
#define LED     PINC7

#elif LED == D0
#define LED_NAME "D0"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND0
#elif LED == D1
#define LED_NAME "D1"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND1
#elif LED == D2
#define LED_NAME "D2"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND2
#elif LED == D3
#define LED_NAME "D3"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND3
#elif LED == D4
#define LED_NAME "D4"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND4
#elif LED == D5
#define LED_NAME "D5"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND5
#elif LED == D6
#define LED_NAME "D6"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND6
#elif LED == D7
#define LED_NAME "D7"
#undef LED
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED     PIND7

#elif LED == A0
#define LED_NAME "A0"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA0
#elif LED == A1
#define LED_NAME "A1"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA1
#elif LED == A2
#define LED_NAME "A2"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA2
#elif LED == A3
#define LED_NAME "A3"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA3
#elif LED == A4
#define LED_NAME "A4"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA4
#elif LED == A5
#define LED_NAME "A5"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA5
#elif LED == A6
#define LED_NAME "A6"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA6
#elif LED == A7
#define LED_NAME "A7"
#undef LED
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED     PINA7

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


#ifdef RS485

#define PIN_INVERTED 0x8000
#define A0_INV (PIN_INVERTED + A0)
#define A1_INV (PIN_INVERTED + A1)
#define A2_INV (PIN_INVERTED + A2)
#define A3_INV (PIN_INVERTED + A3)
#define A4_INV (PIN_INVERTED + A4)
#define A5_INV (PIN_INVERTED + A5)
#define A6_INV (PIN_INVERTED + A6)
#define A7_INV (PIN_INVERTED + A7)
#define B0_INV (PIN_INVERTED + B0)
#define B1_INV (PIN_INVERTED + B1)
#define B2_INV (PIN_INVERTED + B2)
#define B3_INV (PIN_INVERTED + B3)
#define B4_INV (PIN_INVERTED + B4)
#define B5_INV (PIN_INVERTED + B5)
#define B6_INV (PIN_INVERTED + B6)
#define B7_INV (PIN_INVERTED + B7)
#define C0_INV (PIN_INVERTED + C0)
#define C1_INV (PIN_INVERTED + C1)
#define C2_INV (PIN_INVERTED + C2)
#define C3_INV (PIN_INVERTED + C3)
#define C4_INV (PIN_INVERTED + C4)
#define C5_INV (PIN_INVERTED + C5)
#define C6_INV (PIN_INVERTED + C6)
#define C7_INV (PIN_INVERTED + C7)
#define D0_INV (PIN_INVERTED + D0)
#define D1_INV (PIN_INVERTED + D1)
#define D2_INV (PIN_INVERTED + D2)
#define D3_INV (PIN_INVERTED + D3)
#define D4_INV (PIN_INVERTED + D4)
#define D5_INV (PIN_INVERTED + D5)
#define D6_INV (PIN_INVERTED + D6)
#define D7_INV (PIN_INVERTED + D7)

#if RS485 == B0
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB0
#elif RS485 == B1
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB1
#elif RS485 == B2
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB2
#elif RS485 == B3
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB3
#elif RS485 == B4
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB4
#elif RS485 == B5
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB5
#elif RS485 == B6
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB6
#elif RS485 == B7
#undef RS485
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB7

#elif RS485 == C0
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC0
#elif RS485 == C1
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC1
#elif RS485 == C2
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC2
#elif RS485 == C3
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC3
#elif RS485 == C4
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC4
#elif RS485 == C5
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC5
#elif RS485 == C6
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC6
#elif RS485 == C7
#undef RS485
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC7

#elif RS485 == D0
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND0
#elif RS485 == D1
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND1
#elif RS485 == D2
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND2
#elif RS485 == D3
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND3
#elif RS485 == D4
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND4
#elif RS485 == D5
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND5
#elif RS485 == D6
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND6
#elif RS485 == D7
#undef RS485
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND7

#elif RS485 == A0
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA0
#elif RS485 == A1
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA1
#elif RS485 == A2
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA2
#elif RS485 == A3
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA3
#elif RS485 == A4
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA4
#elif RS485 == A5
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA5
#elif RS485 == A6
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA6
#elif RS485 == A7
#undef RS485
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA7
#elif RS485 == B0_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB0
#elif RS485 == B1_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB1
#elif RS485 == B2_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB2
#elif RS485 == B3_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB3
#elif RS485 == B4_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB4
#elif RS485 == B5_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB5
#elif RS485 == B6_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB6
#elif RS485 == B7_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    PORTB
#define RS485_PIN     PINB
#define RS485         PINB7

#elif RS485 == C0_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC0
#elif RS485 == C1_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC1
#elif RS485 == C2_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC2
#elif RS485 == C3_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC3
#elif RS485 == C4_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC4
#elif RS485 == C5_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC5
#elif RS485 == C6_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC6
#elif RS485 == C7_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    PORTC
#define RS485_PIN     PINC
#define RS485         PINC7

#elif RS485 == D0_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND0
#elif RS485 == D1_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND1
#elif RS485 == D2_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND2
#elif RS485 == D3_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND3
#elif RS485 == D4_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND4
#elif RS485 == D5_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND5
#elif RS485 == D6_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND6
#elif RS485 == D7_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    PORTD
#define RS485_PIN     PIND
#define RS485         PIND7

#elif RS485 == A0_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA0
#elif RS485 == A1_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA1
#elif RS485 == A2_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA2
#elif RS485 == A3_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA3
#elif RS485 == A4_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA4
#elif RS485 == A5_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA5
#elif RS485 == A6_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA6
#elif RS485 == A7_INV
#undef RS485
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    PORTA
#define RS485_PIN     PINA
#define RS485         PINA7
#else
#error -------------------------------------------
#error Unrecognized RS485 name.  Should be like "B5"
#error or "B5_INV" for active high
#error -------------------------------------------
#endif
#endif
