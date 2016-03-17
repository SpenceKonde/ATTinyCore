#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
/* Onboard LED is connected to pin PB5 in Arduino NG, Diecimila, and Duemilanove */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB5

#ifdef RS485_SUPPORT
#define DE_DDR      DDRD
#define DE_PORT     PORTD
#define DE_PIN      PIND
#define DE_BIT      PIND5
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

#if defined(__AVR_ATmega162__) || defined(__AVR_ATmega162)
#define LED_DDR  DDRB
#define LED_PORT PORTB
#define LED_PIN  PINB
#define LED      PINB7
	
/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif
	
#if defined(__AVR_ATmega8__)
  //Name conversion R.Wiersma
  #define UCSR0A	UCSRA
  #define UDR0 		UDR
  #define UDRE0 	UDRE
  #define RXC0		RXC
  #define FE0		FE
  #define TIFR1 	TIFR
  #define WDTCSR	WDTCR
#endif

#if defined( __AVR_ATtiny167__) || defined( __AVR_ATtiny87__)
/* Red LED is connected to pin PA2 */ 
#define LED_DDR     DDRA
#define LED_PORT    PORTA
#define LED_PIN     PINA
#define LED         PINA2
/* Ports for soft UART - left port only for now. TX/RX on PA1/PA0 */
#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT PINA6 //6
#define UART_RX_BIT PINA7 //7
#endif
#endif

/* Tiny24 support */
#ifdef __AVR_ATtiny24__
/* Bootloader entry pin pin is PB1 */ 
#define BOOTENTRY_PIN     PINB
#define BOOTENTRY         PINB1
/* Red LED is connected to pin PB2 */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB2
/* Ports for soft UART - left port only for now. TX/RX on PB1/PB0 */
#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT PINA1
#define UART_RX_BIT PINA2
#endif
#endif
/* Tiny84 support */
#ifdef __AVR_ATtiny84__
#ifdef ALTERNATE_LAYOUT
/* Red LED is connected to pin PB1 */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB1
/* Ports for soft UART - left port only for now. TX/RX on PB1/PB0 */
#ifdef SOFT_UART
#define UART_RX_PIN    PINA
#define UART_PORT   PORTB
#define UART_PIN    PINB
#define UART_DDR    DDRB
#define UART_TX_BIT    PINB2
#define UART_RX_BIT    PINA7
#endif
#else
/* Red LED is connected to pin PB2 */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB2
/* Ports for soft UART - left port only for now. TX/RX on PB1/PB0 */
#ifdef SOFT_UART
#define UART_PORT   PORTA
#define UART_PIN    PINA
#define UART_DDR    DDRA
#define UART_TX_BIT    PINA1
#define UART_RX_BIT    PINA2
#endif
#endif
#endif

/* Tiny85 support */
#ifdef __AVR_ATtiny85__
/* Red LED is connected to pin PB2 */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB2
/* Ports for soft UART - left port only for now. TX/RX on PB1/PB0 */
#ifdef SOFT_UART
#define UART_PORT   PORTB
#define UART_PIN    PINB
#define UART_DDR    DDRB
#define UART_TX_BIT 0
#define UART_RX_BIT 1
#endif
#endif

/* Sanguino support */
#if defined(__AVR_ATmega644P__)
/* Onboard LED is connected to pin PB0 on Sanguino */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB0

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTD
#define UART_PIN    PIND
#define UART_DDR    DDRD
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif

/* Mega support */
#if defined(__AVR_ATmega1280__)
/* Onboard LED is connected to pin PB7 on Arduino Mega */ 
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PINB
#define LED         PINB7

/* Ports for soft UART */
#ifdef SOFT_UART
#define UART_PORT   PORTE
#define UART_PIN    PINE
#define UART_DDR    DDRE
#define UART_TX_BIT 1
#define UART_RX_BIT 0
#endif
#endif
