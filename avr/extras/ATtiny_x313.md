# ATtiny 2313/4313
![x313 pin mapping](Pinout_x313.jpg "Arduino Pin Mapping for ATtiny x313-family")

Specification         |    ATtiny4313  |   ATtiny2313A  |    ATtiny2313  |
----------------------|----------------|----------------|----------------|
Bootloader (if any)   |           None |           None |           None |
Uploading uses        |   ISP/SPI pins |   ISP/SPI pins |   ISP/SPI pins |
Flash available user  |     4096 bytes |     2048 bytes |     2048 bytes |
RAM                   |      256 bytes |      128 bytes |      128 bytes |
EEPROM                |       64 bytes |       64 bytes |       64 bytes |
GPIO Pins             |     17 + RESET |     17 + RESET |     17 + RESET |
ADC Channels          |           None |           None |           None |
PWM Channels          |              4 |              4 |              4 |
Interfaces            |     USI, USART |     USI, USART |     USI, USART |
Clocking Options      |         in MHz |         in MHz |         in MHz |
Int. Oscillator       |     8, 4, 2, 1 |     8, 4, 2, 1 |     8, 4, 2, 1 |
Int. WDT Oscillator   |        128 kHz |        128 kHz |        128 kHz |
Internal, with tuning |    8, 12, 12.8 |    8, 12, 12.8 |    8, 12, 12.8 |
External Crystal      |   All Standard |   All Standard |   All Standard |
External Clock        |   All Standard |   All Standard |   All Standard |

Tiny flash, a poor featureset, and a relatively high price tag relegate the x313 to the realm of "desperation" processors. Or one would think it would - except they remain surprisingly popular despite having been obsolete for ages. We do not recommend use of these parts if you can avoid it. The 2313-not-A is a strictly worse version of the 2313, sold for a significant premium. Definitely avoid those.

## Programming
Any of these parts can be programmed by use of any ISP programmer. If using a version of Arduino prior to 1.8.13, be sure to choose a programmer with (ATTinyCore) after it's name (in 1.8.13 and later, only those will be shown), and connect the pins as normal for that ISP programmer.

### There is no bootloader
Not enough space for it to make sense. The x313-series is a pair of essentially obsolete and overpriced parts being kept in production to milk companies who don't have time to redesign (and haven't for the past decade). Their ongoing popularity is baffling

### Tone Support
Tone() uses Timer1. For best results, use pin PB3 (12) or PB4 (13), as this will use the hardware output compare to generate the square wave instead of using interrupts. In order to use Tone(), you must select Initialize Secondard Timers: Yes. tone() will disable PWM on PB3 and PB4.

### Servo Support
The standard Servo library is hardcoded to work on specific parts only, we include a builtin Servo library that supports the Tiny 4313 though getting everything to fit may be challenging. As always, while a software serial port (including the builtin one, Serial, on these ports, see below) is receiving or transmitting, the servo signal will glitch. See [the Servo/Servo_ATTinyCore library](../libraries/Servo/README.md) for more details. Like tone(), it takes PWM on PB3 and PB4. It is not compatible with Tone.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. This is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all. There is no need for libraries like TinyWire or USIWire or that kind of thing.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI. This should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common, and the only mode that the SPI library has ever supported).

### UART (Serial) Support
There is one full hardware Serial port, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. Be aware that due to the limited memory on these chips the buffers are quite small.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels
```t
UCSRB &= ~(1 << TXEN); // disable TX
UCSRB &= ~(1 << RXEN); // disable RX
```
### There is no ADC
analogRead() is not defined on these parts.

### 2313-not-a does not have PCINT on ports A and D
Not much more to say there. These parts are pretty primitive.


## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x313-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" (for example, if you misspell a vector name) - however, when that interrupt is triggered, the device will (at best) immediately reset (and not clearly - I refer to this as a "dirty reset") The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". The vector number is the number you are shown in the event of a duplicate vector error, as well as the interrupt priority (lower number = higher priority), if, for example, several interrupt flags are set while interrupts are disabled, the lowest numbered one would run first.

|  # | Address | Vector Name        | Interrupt Definition              |
|----|---------|--------------------| ----------------------------------|
|  0 |  0x0000 | RESET_vect         | Any reset                         |
|  1 |  0x0001 | INT0_vect          | External Interrupt Request 0      |
|  2 |  0x0002 | INT1_vect          | External Interrupt Request 1      |
|  3 |  0x0003 | TIMER1_CAPT_vect   | Timer/Counter1 Capture Event      |
|  4 |  0x0004 | TIMER1_COMPA_vect  | Timer/Counter1 Compare Match A    |
|  5 |  0x0005 | TIMER1_OVF_vect    | Timer/Counter1 Overflow           |
|  6 |  0x0006 | TIMER0_OVF_vect    | Timer/Counter0 Overflow           |
|  7 |  0x0007 | USART0_RX_vect     | USART0 Rx Complete                |
|  7 |  0x0007 | USART_RX_vect      | Alias - provided by io.h          |
|  8 |  0x0008 | USART0_UDRE_vect   | USART0 Data Register Empty        |
|  8 |  0x0008 | USART_UDRE_vect    | Alias - provided by io.h          |
|  9 |  0x0009 | USART0_TX_vect     | USART0 Tx Complete                |
|  9 |  0x0009 | USART_TX_vect      | Alias - provided by io.h          |
| 10 |  0x000A | ANALOG_COMP_vect   | Analog Comparator                 |
| 11 |  0x000B | PCINT0_vect        | Pin Change Interrupt 0 (PORT B)   |
| 11 |  0x000B | PCINT_B_vect       | Alias - provided by io.h          |
| 12 |  0x000C | TIMER1_COMPB_vect  | Timer/Counter1 Compare Match B    |
| 13 |  0x000D | TIMER0_COMPA_vect  | Timer/Counter0 Compare Match A    |
| 14 |  0x000E | TIMER0_COMPB_vect  | Timer/Counter0 Compare Match B    |
| 15 |  0x000F | USI_START_vect     | USI Start Condition               |
| 16 |  0x0010 | USI_OVERFLOW_vect  | USI Overflow                      |
| 17 |  0x0011 | EE_READY_vect      | EEPROM Ready                      |
| 18 |  0x0012 | WDT_OVERFLOW_vect  | Watchdog Time-out (interrupt mode)|
| 19 |  0x0013 | PCINT1_vect        | Pin Change Interrupt 1 (PORT A) * |
| 19 |  0x0013 | PCINT_A_vect       | Alias - provided by io.h *        |
| 20 |  0x0014 | PCINT2_vect        | Pin Change Interrupt 2 (PORT D) * |
| 20 |  0x0014 | PCINT_D_vect       | Alias - provided by io.h *        |

*This core always compiles with ATtiny2313a, not ATtiny2313 as the target*

* Vector not available in ATtiny2313 (although it is defined in the io headers!)
