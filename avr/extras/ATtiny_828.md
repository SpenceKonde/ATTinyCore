### ATtiny828(R)
![828 Pin Mapping](http://drazzy.com/e/img/PinoutT828.jpg "Arduino Pin Mapping for ATtiny828")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 8192b ( 7680b with Optiboot)
RAM  | 512 bytes
EEPROM | 512 bytes
Bootloader | Yes, Optiboot (serial)
GPIO Pins | 27
ADC Channels | 28 (including the one on reset), many differential channels
PWM Channels | 4
Interfaces | UART, SPI, slave I2C
Clock options | Internal 1/4/8 MHz, external clock (no crystal) up to 20 MHz

The ATtiny828R has a tighter factory calibration of it's internal oscillator. It is otherwise identical to the ATtiny828.

## Programming
The ATtiny828 can be programmed by use of any ISP programmer. If using a version of Arduino prior to 1.8.13, be sure to choose a programmer with (ATTinyCore) after it's name (in 1.8.13 and later, only those will be shown), and connect the pins as normal for that ISP programmer.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny1634, operating on the hardware UART0 (Serial) port at 115200 baud for 12 MHz or higher, or 57600 when running at lower speeds. The bootloader uses 640b of space, leaving 15744b available for user code. In order to work on these parts, which do not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application (versions of the core prior to 1.2.0 used WDT vector, so WDT cannot be used as an interrupt - we recommend burning bootloader with the new version if this is an issue). A version of the bootloader that operates on Serial1 is included as well (choose the desired UART when burning the booloader).

### Clock options
The ATtiny828, like the ATtiny88 does not provide support for using an external crystal as a clock source, only the internal oscillator (at 8, 4 1 MHz) or an external *clock* source.

### Using external CLOCK on 828
These parts do not support using an external crystal. External Clock, however, is supported - this requires an external clock source (not just a crystal) connected to the CLKI pin. **DANGER** if this clock source is not present, you must supply a clock source to CLKI pin before it can be reprogrammed, including to use a different clock source. Unlike the parts which support a crystal, where this "potentially dangerous" clock option is not available in the menus because there are menu options for the supported speeds with a crystal as a clock source, so the procedure below can be used. On the 48/88/828, these menu options will enable the external CLOCK option. **This is not the same as external crystal - do not use this option if you are unsure about the difference between external clock and external crystal!**

### Internal Oscillator voltage dependence
Prior to 1.4.0, many users had encountered issues due to the voltage dependence of the oscillator. While the calibration is very accurate between 2.7 and 4v, as the voltage rises above 4.5v, the speed increases significantly. Although the magnitude of this is larger than on many of the more common parts, the issue is not as severe as had long been thought - the impact had been magnified by the direction of baud rate error, and the fact that many US ports actually supply 5.2-5.3v. As of 1.4.0, a simple solution was implemented to enable the same bootloader to work across the 8 MHz (Internal, Vcc < 4.5v) and 8 MHz (Internal, Vcc > 4.5 MHz ) board definitions - it should generally work between 2.7v and 5.25v - though the extremes of that range may be dicey. We do still provide a >4.5v clock option in order to improve behavior of the running sketch - it will nudge the oscillator calibration down to move it closer to the nominal 8MHz clock speed; sketches uploaded with the higher voltage option. This is not perfect, but it is generally good enough to work with Serial on around 5v (including 5.25v often found on USB ports to facilitate chargeing powerhungry devices), and millis()/micros() will keep better time than in previous versions.

The internal oscillator is factory calibrated to +/- 10% or +/- 2% for the slightly more expensive 828R. +/- 2% is good enough for serial communication. However, this spec is only valid below 4v - above 4v, the oscillator runs significantly faster; enough so that serial communication does not work absent the above-described countermeasures.

### Tone Support
Tone() uses Timer1. For best results, use pin 21 or 22 (PIN_PC5, PIN_PC6), as this will use the hardware output compare to generate the square wave instead of using interrupts. Any use of tone() will disable PWM on pins 21 and 22.

### I2C Support
Slave I2C functionality is provided in hardware, but a software implementation must be used for master functionality. This is done automatically with the included version of the Wire.h library.

### SPI Support
There is full Hardware SPI support.

### UART (Serial) Support
There is one hardware serial port, Serial. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels
```
UCSRB &=~(1<<TXEN); // disable TX
UCSRB &=~(1<<RXEN); // disable RX
```

### ADC Reference options
Despite having 28 ADC input channels, the 828 only has the two basic reference options.

* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
### Weird I/O-pin related features
There are a few strange features relating to the GPIO pins on the ATtinyx41 family which are found only in a small number of other parts released around the same time.

#### Special "high sink" port
All pins on PORTC have unusually high sink capability - when sinking a given amount of current, the voltage on these pins is about half that of typical pins. Using the `PHDE` register, these can be set to sink even more aggressively.

```
PHDE=(1<<PHDEC);
```

This is no great shakes - the Absolute Maximum current rating of 40mA still applies and all... but it does pull closer to ground with a a "large" 10-20mA load. A very strange feature of these parts. The PWM outputs of the timers can be remapped to this port as well, making it of obvious utility for driving LEDs and similar. This also means that, if you are attempting to generate an analog voltage with a PWM pin and an RC filter, your result may be lower than expected, as the pin drivers are not symmetric.

#### Separate pullup-enable register
Like the ATtinyx41 and ATtiny1634, these have a fourth register for each port, PUEx, which controls the pullups (rather than PORTx when DDRx has pin set as input).

### Purchasing ATtiny828 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny828 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATtiny828 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-05-18T04:57:39.963Z-AZB-8_V2_Asy.png.855x570_q85_pad_rcrop.png)
###[Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/)
###[Bare Boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/)

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny828, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
vect_num | Vector Address | Vector Name | Interrupt Definition
------------ | ------------- | ------------ | -------------
0 | 0x0000 | RESET_vect | Any reset (pin, WDT, power-on, BOD)
1 | 0x0001 | INT0_vect | External Interrupt Request 0
2 | 0x0002 | INT1_vect | External Interrupt Request 1
3 | 0x0003 | PCINT0_vect | Pin Change Interrupt 0 (PORT A)
4 | 0x0004 | PCINT1_vect | Pin Change Interrupt 1 (PORT B)
5 | 0x0005 | PCINT2_vect | Pin Change Interrupt 2 (PORT C)
6 | 0x0006 | PCINT3_vect | Pin Change Interrupt 3 (PORT D)
7 | 0x0007 | WDT_vect | Watchdog Time-out (Interrupt Mode)
8 | 0x0008 | TIM1_CAPT_vect | Timer/Counter1 Input Capture
8 | 0x0008 | TIMER1_CAPT_vect | Alias - provided by ATTinyCore
9 | 0x0009 | TIM1_COMPA_vect | Timer/Counter1 Compare Match A
9 | 0x0009 | TIMER1_COMPA_vect | Alias - provided by ATTinyCore
10 | 0x000A | TIM1_COMPB_vect | Timer/Counter1 Compare Match B
10 | 0x000A | TIMER1_COMPB_vect | Alias - provided by ATTinyCore
11 | 0x000B | TIM1_OVF_vect | Timer/Counter1 Overflow
11 | 0x000B | TIMER1_OVF_vect | Alias - provided by ATTinyCore
12 | 0x000C | TIM0_COMPA_vect | Timer/Counter0 Compare Match A
12 | 0x000C | TIMER0_COMPA_vect | Alias - provided by ATTinyCore
13 | 0x000D | TIM0_COMPB_vect | Timer/Counter0 Compare Match B
13 | 0x000D | TIMER0_COMPB_vect | Alias - provided by ATTinyCore
14 | 0x000E | TIM0_OVF_vect | Timer/Counter0 Overflow
14 | 0x000E | TIMER0_OVF_vect | Alias - provided by ATTinyCore
15 | 0x000F | SPI_vect | SPI Serial Transfer Complete
16 | 0x0010 | USART0_RXS_vect | USART0 Rx Start
17 | 0x0011 | USART0_RXC_vect | USART0 Rx Complete
18 | 0x0012 | USART0_DRE_vect | USART0 Data Register Empty
19 | 0x0013 | USART0_TXC_vect | USART0 Tx Complete
20 | 0x0014 | ADC_READY_vect | ADC Conversion Complete
21 | 0x0015 | EE_RDY_vect | EEPROM Ready
22 | 0x0016 | ANA_COMP_vect | Analog Comparator
23 | 0x0017 | TWI_vect | Two-Wire Interface
24 | 0x0018 | SPM_RDY_vect | Store Program Memory Ready
25 | 0x0019 | QTRIP_vect | QTouch
