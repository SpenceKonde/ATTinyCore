### ATtiny 87/167
![x7 pin mapping](Pinout_x7.jpg "Arduino Pin Mapping for ATtiny x7-family")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 8096b/16768b ( 7552b/15744b with Optiboot, 14842 with Micronucleus)
RAM  | 512 bytes
EEPROM | 512 bytes
Bootloader | Yes, Optiboot (serial) or Micronucleus (VUSB)
GPIO Pins | 15
ADC Channels | 11
PWM Channels | 3
Interfaces | LIN/UART, USI, SPI
Clock options | Internal 1/8 MHz, external crystal or clock* up to 20 MHz
Clock options | Micronucleus 16 MHz w/USB, 8/4/1 MHz w/out USB from 16MHz ext. crystal

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

## Programming
Any of these parts can be programmed by use of any ISP programmer. If using a version of Arduino prior to 1.8.13, be sure to choose a programmer with (ATTinyCore) after it's name (in 1.8.13 and later, only those will be shown), and connect the pins as normal for that ISP programmer.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny87 and 167, operating on the hardware UART/LIN port at 115200 baud for 12 or 16 MHz clock speed, and 57600 when running at 8 MHz. In order to work on the x7 series, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the WDT vector gets pointed to the start of the application.  This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

### Micronucleus VUSB Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny167, allowing sketches to be uploaded directly over USB. The board definition runs at 16 MHz via external crystal (if USB is not required, it can be prescaled as listed in the table for low power applications). See the document on [Micronucleus usage](UsingMicronucleus.md) for more information. D- is on PIN_PB3, D+ is on pin PIN_PB6.

**Currently the version of Micronucleus supplied with ATTinyCore for this part enters the bootloader upon power-on only. This will be made an option in future versions** The "stock" bootloader found on commercially available boards usually enters the bootloader on all reset sources.

## Features

### Alternate pinout options
There was an old ATtiny x7 core with a different and more awkward pinout. This is supported, for compatibility purposes, via the "Legacy" pinmapping option. It should be used only if you are trying to use an old sketch that was written for that pin mapping. The Digispark Pro boards have pins labeled with yet another pin mapping. All pin mappings can be chosen for both Digispark/VUSB and non-VUSB boards, for compatibility. This is selected from the Tools -> Pin Mapping submenu. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for `PINMAPPING_OLD`, `PINMAPPING_NEW`, or `PINMAPPING_DIGI` macro (eg, `#ifdef PINMAPPING_OLD` - I would recommend checking if the compatible one is not defined and immediately #error'ing in that case). Alternately, also as of 1.4.0, with any pin mapping selected, you can always refer to pins by their port and number within that port, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7, which is pin 7 in the clockwise mapping and pin 3 in the counterclockwise mapping (don't use PIN_xn or Pxn) - in this case the pin mapping won't matter.

Example of a "guard" against wrong pin mapping:
```
#ifndef PINMAPPING_NEW
#error "Sketch was written for new pinmapping!"
#endif
```
The pin mapping for the Digispark Pro is very, very strange. Note that on the An constants for analogRead() n is the number of the digital pin, not the the ADC channel!

### Flexible PWM support (New 1.4.0)
The two channels of Timer1 can each output on one or more of 4 pins, albeit with the same duty cycle. The OCR1Ax and OCR1Bx pins each share the channel. All of those pins can be used for PWM. If you do `analogWrite(PIN_PB0,64);`, you get 25% dutycycle, if you then do `analogWrite(PIN_PB2,128);` (these are OCR1AU and OCR1AW, respectively) both of the pins will be outputting 50% dutycycle after the second ommand. To stop the PWM output, call digitalWrite() or analogWrite() with 0 or 255 on the pin.

### Tone Support
Tone() uses Timer1. For best results, use a pin on port B - those will use the hardware output compare rather than an interrupt to generate the tone. Using tone() will disable all PWM pins except PIN_PA2.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire.h library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all.

### SPI Support
There is a hardware SPI port and the normal SPI library can be used.

### UART (Serial) with LIN support
There is one full hardware Serial port with LIN support, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. The ATtiny x7-family has LIN support, unique among the ATtiny linup; LIN (Local Interconnect Network) is frequently used in automotive and industrial applications. One consequence of this additional feature is that the baud rate generator is able to match baud rates much more closely than a "standard" UART module.

### ADC Reference Options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1
* INTERNAL2V56: Internal 2.56v

### Purchasing ATtiny167 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny167 boards through my Tindie store - your purchases support the continued development of this core.
![Picture of ATtiny167 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-04-19T01:35:24.770Z-AZB7_Asy.png.855x570_q85_pad_rcrop.png)
* [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny-861-or-167-development-board-assembled/)
* [Bare Boards](https://www.tindie.com/products/DrAzzy/attiny-16787861461261-breakout-bare-board/)
* Micronucleus boards are readily available all over the internet, fairly cheaply, in several models. Search for things like "Digispark Pro", "Digispark ATtiny167", "ATtiny167 USB" and so on.


## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x7-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
Addresses are for 87 and 167 - the 167, having 16k of flash, has 4-byte vectors instead of 2-byte vectors.
vect_num | Vector Address | Vector Name | Interrupt Definition
------------ | ------------- | ------------ | -------------
0 | 0x0000/0x0000 | RESET_vect | External Pin, Power-on Res
1 | 0x0001/0x0002 | INT0_vect | External Interrupt Request 0
2 | 0x0002/0x0004 | INT1_vect | External Interrupt Request 1
3 | 0x0003/0x0006 | PCINT0_vect | Pin Change Interrupt (PORT A)
4 | 0x0004/0x0008 | PCINT1_vect | Pin Change Interrupt (PORT B)
5 | 0x0005/0x000A | WDT_vect | Watchdog Time-out (Interrupt Mode)
6 | 0x0006/0x000C | TIMER1_CAPT_vect | Timer/Counter1 Capture
7 | 0x0007/0x000E | TIMER1_COMPA_vect | Timer/Counter1 Compare Match
8 | 0x0008/0x0010 | TIMER1_COMPB_vect | Timer/Coutner1 Compare Match
9 | 0x0009/0x0012 | TIMER1_OVF_vect | Timer/Counter1 Overflow
10 | 0x000A/0x0014 | TIMER0_COMPA_vect | Timer/Counter0 Compare Match
11 | 0x000B/0x0016 | TIMER0_OVF_vect | Timer/Counter0 Overflow
12 | 0x000C/0x0018 | LIN_TC_vect | LIN/UART Transfer Complete
13 | 0x000D/0x001A | LIN_ERR_vect | LIN/UART Error
14 | 0x000E/0x001C | SPI_STC_vect | SPI Serial Transfer Complete
15 | 0x000F/0x001E | ADC_READY_vect | Conversion Complete
16 | 0x0010/0x0020 | EE_READY_vect | EEPROM Ready
17 | 0x0011/0x0022 | ANALOG_COMP_vect | Analog Comparator
18 | 0x0012/0x0024 | USI_START_vect | USI Start Condition
19 | 0x0013/0x0026 | USI_OVF_vect | USI Counter Overflow
