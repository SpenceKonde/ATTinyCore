### ATtiny 24/44/84(a)
![x4 pin mapping](http://drazzy.com/e/img/PinoutT84a.jpg "Arduino Pin Mapping for ATtiny x4-family")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b/8192b (3456b/7552b with Optiboot, 6658b with Micronucleus)
RAM  | 128/256/512 bytes
EEPROM | 128/256/512 bytes
Bootloader | Yes, Optiboot (serial) or Micronucleus (VUSB)
GPIO Pins | 11
ADC Channels | 8, plus many differential channels
PWM Channels | 4
Interfaces | USI
Clock options | Internal 1/8 MHz, external crystal or clock* up to 20 MHz

The 24/44/84 and 24a/44a/84a are functionally identical; the latter replaced the former in 2008, and uses slightly less power, and actual ATtiny84 parts are rarely seen in circulation today. They have the same signatures and are fully interchangible. It is extremely common to refer to the ATtiny84a as an ATtiny84.

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

Two pinouts are available for compatibility with other cores, see below for more information. Be sure you are using the pinout you think you are!

## Programming
All of these parts can be programmed via an ISP

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny84/44, operating using software serial at 19200 baud - the software serial uses the AIN0 and AIN1 pins (see UART section below). The bootloader uses 640b of space, leaving 3456 or7552b available for user code. In order to work on the 84, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

Programming the ATtiny84/44 via ISP without the bootloader is fully supported; the 24 is supported only for ISP programming.

### Micronucleus Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny84(a), allowing sketches to be uploaded directly over USB. It runs at 8 MHz via the internal oscillator. See the document on [Micronucleus usage](UsingMicronucleus.md) for more information. In order to achieve the 12 MHz clock during USB operation, the OSCCAL is drastically increased to 12MHz, but is set back down before running the sketch. D- is on PIN_PB0, D+ is on PIN_PB1.

## Features

### Pin mapping options
Throughout the history of ATtiny Arduino support, two pin mappings have been used. Here, they are referred to descriptively: the clockwise, and counterclockwise pinout. The desired pin mapping can be chosen from the Tools -> Pin Mapping submenu. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for PINMAPPING_CCW or PINMAPPING_CW macro (eg, `#ifdef PINMAPPING_CCW` - I would recommend checking for the incompatible one, and immediately #error'ing if you find it). Alternately, also as of 1.4.0, with either pin mapping selected, you can always refer to pins by their port and number within that port, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7, which is pin 7 in the clockwise mapping and pin 3 in the counterclockwise mapping (don't use PIN_xn or Pxn).

Example of a "guard" against wrong pin mapping:
```
#ifdef PINMAPPING_CCW
#error "Sketch was written for clockwise pin mapping!"
#endif
```

### Tone Support
Tone() uses Timer1. For best results, use pin 6 and 5 (4 and 5 with alternate pinout - PA6 and PA5), as this will use the hardware output compare to generate the square wave instead of using interrupts.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common).

### UART (Serial) Support
There is no hardware UART. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](TinySoftSerialBaud.md)

To disable the RX channel (to use only TX), the following commands should be used after calling Serial.begin(). No special action is needed to disable the TX line if only RX is needed.
```
ACSR &=~(1<<ACIE);
ACSR |=~(1<<ACD);
```

### ADC Reference options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1


## Purchasing ATtiny84 Boards
As the ATtiny84 is available in an easy-to-solder through-hole DIP package, a board can be easily made by just soldering the part into prototyping board.
I (Spence Konde) sell a specialized prototyping board that combines an ISP header with prototyping space and outlines to fit common SMD parts.
* [ATtiny84 prototyping board](https://www.tindie.com/products/drazzy/attiny84-project-board/)
Micronucleus boards can be bought here, from one of my collaborators:
* [Micronucleus ATtiny84a](https://www.tindie.com/products/svdbor/tiniest-arduino-compatible-board-with-micronucleus/)

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x4-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
vect_num | Vector Address | Vector Name | Interrupt Definition
------------ | ------------- | ------------ | -------------
0 | 0x0000 | RESET_vect | Any reset (pin, WDT, power-on, BOD)
1 | 0x0001 | INT0_vect | External Interrupt Request 0
2 | 0x0002 | PCINT0_vect | Pin Change Interrupt 0 (PORT A)
3 | 0x0003 | PCINT1_vect | Pin Change Interrupt 1 (PORT B)
4 | 0x0004 | WDT_vect | Watchdog Time-out (Interrupt Mode)
5 | 0x0005 | TIM1_CAPT_vect | Timer/Counter1 Capture Event
5 | 0x0005 | TIMER1_CAPT_vect | Alias - provided by ATTinyCore
6 | 0x0006 | TIM1_COMPA_vect | Timer/Counter1 Compare Match A
6 | 0x0006 | TIMER1_COMPA_vect | Alias - provided by ATTinyCore
7 | 0x0007 | TIM1_COMPB_vect | Timer/Counter1 Compare Match B
7 | 0x0007 | TIMER1_COMPB_vect | Alias - provided by ATTinyCore
8 | 0x0008 | TIM1_OVF_vect | Timer/Counter1 Overflow
8 | 0x0008 | TIMER1_OVF_vect | Alias - provided by ATTinyCore
9 | 0x0009 | TIM0_COMPA_vect | Timer/Counter0 Compare Match A
9 | 0x0009 | TIMER0_COMPA_vect | Alias - provided by ATTinyCore
10 | 0x000A | TIM0_COMPB_vect | Timer/Counter0 Compare Match B
10 | 0x000A | TIMER0_COMPB_vect | Alias - provided by ATTinyCore
11 | 0x000B | TIM0_OVF_vect | Timer/Counter0 Overflow
11 | 0x000B | TIMER0_OVF_vect | Alias - provided by ATTinyCore
12 | 0x000C | ANA_COMP_vect | Analog Comparator
13 | 0x000D | ADC_vect | ADC Conversion Complete
14 | 0x000E | EE_RDY_vect | EEPROM Ready
16 | 0x000F | USI_STR_vect | USI START
17 | 0x0010 | USI_OVF_vect | USI Overflow
