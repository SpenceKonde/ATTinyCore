### ATtiny 2313/4313
![x4 pin mapping](Pinout_x313.jpg "Arduino Pin Mapping for ATtiny x313-family")

 Specifications | .
------------ | -------------
Flash (program memory) | 2048b/4096b
RAM | 128/256 bytes
EEPROM | 128/256 bytes
Bootloader | No
GPIO Pins | 17
ADC Channels | None
PWM Channels | 4
Interfaces | UART, USI
Clock options | Internal 0.5/1/4/8 MHz, external crystal or clock* up to 20 MHz

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

### Tone Support
Tone() uses Timer1. For best results, use pin 12, and 13, as this will use the hardware output compare to generate the square wave instead of using interrupts. In order to use Tone(), you must select Initialize Secondard Timers: Yes

### Initialize Secondard Timers option
When an Arduino sketch runs, prior to Setup() being called, a hidden init() function is called to set up the hardware. One of the things it does is configure the timers. Timer0 (used for millis) is always configured, but on the x313 series, due to the extremely limited flash, it is optional whether to initialize Timer1. If Timer1 is not initialized, Tone() and PWM on pins 12 and 13 will not work.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common).

### UART (Serial) Support
There is one full hardware Serial port, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. Be aware that due to the limited memory on these chips the buffers are quite small.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels
```
UCSRB &=~(1<<TXEN); // disable TX
UCSRB &=~(1<<RXEN); // disable RX
```

### There is no ADC

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x313-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
vect_num | Vector Address | Vector Name | Interrupt Definition
------------ | ------------- | ------------ | -------------
0 | 0x0000 | RESET_vect | Any reset (pin, WDT, power-on, BOD)
1 | 0x0001 | INT0_vect | External Interrupt Request 0
2 | 0x0002 | INT1_vect | External Interrupt Request 1
3 | 0x0003 | TIMER1_CAPT_vect | Timer/Counter1 Capture Event
4 | 0x0004 | TIMER1_COMPA_vect | Timer/Counter1 Compare Match A
5 | 0x0005 | TIMER1_OVF_vect | Timer/Counter1 Overflow
6 | 0x0006 | TIMER0_OVF_vect | Timer/Counter0 Overflow
7 | 0x0007 | USART0_RX_vect | USART0 Rx Complete
7 | 0x0007 | USART_RX_vect | Alias - provided by io.h
8 | 0x0008 | USART0_UDRE_vect | USART0 Data Register Empty
8 | 0x0008 | USART_UDRE_vect | Alias - provided by io.h
9 | 0x0009 | USART0_TX_vect | USART0 Tx Complete
9 | 0x0009 | USART_TX_vect | Alias - provided by io.h
10 | 0x000A | ANALOG_COMP_vect | Analog Comparator
11 | 0x000B | PCINT0_vect | Pin Change Interrupt 0 (PORT B)
19 | 0x000B | PCINT_B_vect | Alias - provided by io.h
12 | 0x000C | TIMER1_COMPB_vect | Timer/Counter1 Compare Match B
13 | 0x000D | TIMER0_COMPA_vect | Timer/Counter0 Compare Match A
14 | 0x000E | TIMER0_COMPB_vect | Timer/Counter0 Compare Match B
15 | 0x000F | USI_START_vect | USI Start Condition
16 | 0x0010 | USI_OVERFLOW_vect | USI Overflow
17 | 0x0011 | EE_READY_vect | EEPROM Ready
18 | 0x0012 | WDT_OVERFLOW_vect | Watchdog Time-out (interrupt mode)
19 | 0x0013 | PCINT1_vect | Pin Change Interrupt 1 (PORT A) *
19 | 0x0013 | PCINT_A_vect | Alias - provided by io.h
20 | 0x0014 | PCINT2_vect | Pin Change Interrupt 2 (PORT D) *
20 | 0x0014 | PCINT_D_vect | Alias - provided by io.h
*This core always compiles with ATtiny2313a, not ATtiny2313 as the target*
* Vector not available in ATtiny2313 (although it is defined in the io headers!)
