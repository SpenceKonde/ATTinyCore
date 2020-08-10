### ATtiny 48/88
![x8 Pin Mapping](http://drazzy.com/e/img/PinoutT88.jpg "Arduino Pin Mapping for ATtiny 88/48 in TQFP")
![x8 Pin Mapping](Pinout_x8-PU.jpg "Arduino Pin Mapping for ATtiny 88/48 in DIP")


 Specifications |  .
------------ | -------------
Flash (program memory)   | 4096b/8192b (3456b/7552b with Optiboot, 6550b with Micronucleus)
RAM  | 256/512 bytes
EEPROM | 64 bytes
Bootloader |  Yes, Optiboot (s/w serial) or Micronucleus (VUSB)
GPIO Pins | 27
ADC Channels | 8
PWM Channels | 4
Interfaces | SPI, I2C
Clock options | Internal 1/4/8 MHz, external clock up to 12 MHz, 16 MHz (overclocked)
Clock options | Micronucleus 16 MHz w/USB, 8/4/1 MHz w/out USB from 16MHz ext. clock

The ATtiny x8-family is intended as a low cost option compatible with the popular ATmega x8 series. As such, they have a nearly identical pinout (with a couple of extra GPIO pins in the TQFP version). Although these have the full hardware I2C and SPI peripherals, they lack both a hardware serial port and the option to use a crystal as a clock source. A Micronucleus board is available with a 16 MHz external CLOCK under the name "MH Tiny". They use a pin numbering scheme that differs significantly from the standard one; a pin mapping is provided which matches the markings on the board.

## Programming
Any of these parts can be programmed by use of any ISP programmer. If using a version of Arduino prior to 1.8.13, be sure to choose a programmer with (ATTinyCore) after it's name (in 1.8.13 and later, only those will be shown), and connect the pins as normal for that ISP programmer.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny88/48, operating using software serial at 19200 baud - the software serial uses the AIN0 and AIN1 pins, marked on pinout chart (see also UART section below). The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on the 88/48, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

### Micronucleus Bootloader
The Micronucleus bootloader for these parts uses a 16 MHz external clock source. Boards are commercially available (and cheap) under the name MH Tiny (also MH ET and several other names). USB is on pins 1 and 2, and the LED is on pin 0. As of 1.4.1, the new entry mode options detailed in [Using Micronucleus](UsingMicronucleus.md) are available for the Tiny88 (MH-ET). Be aware that there are many bootloaders circulating, including the ones shipped with 1.4.0, which do not actually work on the MH-ET boards; if you have uploaded one of those, you can restore functionality by bootloading using an ISP programmer (provided you hadn't previously disabled reset, of course).

### Alternate pinout options
The MH Tiny boards have pins labeled with a different pin mapping. Pins up to 13 (all of PORTD and first 6 pins of PORTB) are the same, PB6 is not available because it is the clock input, and from there on out, order is different as well. A Tools -> Pin Mapping option is available for both pin mappings, regardless of which bootloader is in use. This is selected from the Tools -> Pin Mapping submenu (defaulting to the "standard" pinmapping except for Micronucleus boards. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for the `PINMAPPING_MHTINY` macro (eg, `#ifdef PINMAPPING_MHTINY` - I would recommend checking if the compatible one is not defined and immediately #error'ing in that case). Alternately, also as of 1.4.0, with either pin mapping selected, you can always refer to pins by their port and number within that port, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7, which is pin 7 in the clockwise mapping and pin 3 in the counterclockwise mapping (don't use PIN_xn or Pxn) - in this case the pin mapping won't matter.

### Clock options
The ATtiny x8-family of microcontrollers, in the interest of lowering costs, does not provide support for using an external crystal as a clock source, only the internal oscillator (at ~8 or ~1 MHz) or an external *clock* source. The internal oscillator is only factory calibrated to +/- 10%, so for timing critical tasks, other arrangements (or a different chip) must be used.

While the +/- 10% accuracy is not close enough to guarantee UART serial will work (that requires ~2% accuracy), every chip I have tested at room temperature and 3.3 ~ 5v has been close enough to 8MHz for serial to work without calibration.

#### Micronucleus clock options
Micronucleus is supported with an external 16 MHz external clock only. It may optionally be prescaled to 8, 4, or 1 MHz for low power applications; this is generated by prescaling the 16 MHz clock after the application starts, at these lower clock speeds, VUSB functionality is not supported.

#### Using external CLOCK on 48/88
These parts do not support using an external crystal. External clock, however, is supported - this requires an external clock source (not just a crystal) connected to the CLKI pin. **DANGER** if this clock source is not present, you must supply a clock source to CLKI pin before it can be reprogrammed, including to use a different clock source. **This is not the same as external crystal - do not use this option if you are unsure about the difference between external clock and external crystal!**

### I2C Support
There is full Hardware I2C support using Wire.h

### SPI Support
There is full Hardware SPI support using SPI.h

### UART (Serial) Support
There is no hardware UART. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART).

If running off the internal oscillator (since this chip does not support a crystal), you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work, though as noted above, this is rarely necessary under typical operating conditions. While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](TinySoftSerialBaud.md)

To disable the RX channel (to use only TX), the following commands should be used after calling Serial.begin(). No special action is needed to disable the TX line if only RX is needed.
```
ACSR &=~(1<<ACIE);
ACSR |=~(1<<ACD);
```

### ADC Reference options
* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1

### Purchasing ATtiny88 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny88 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATtiny88 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2016-05-18T04:57:39.963Z-AZB-8_V2_Asy.png.855x570_q85_pad_rcrop.png)
* [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny88-or-828-breakout-board-assembled/)
* [Bare Boards](https://www.tindie.com/products/DrAzzy/atmega-x8attiny-x8828atmega-x8pb-breakout/)

MH-Tiny boards with USB are available from Aliexpress for around $2/ea, and much higher prices on ebay/Amazon.

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x8-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
vect_num | Vector Address | Vector Name | Interrupt Definition
------------ | ------------- | ------------ | -------------
0 | 0x0000 | RESET_vect | Any reset (pin, WDT, power-on, BOD)
1 | 0x0001 | INT0_vect | External interrupt request 0
2 | 0x0002 | INT1_vect | External interrupt request 1
3 | 0x0003 | PCINT0_vect | Pin Change Interrupt 0 (PORT A)
4 | 0x0004 | PCINT1_vect | Pin Change Interrupt 1 (PORT B)
5 | 0x0005 | PCINT2_vect | Pin Change Interrupt 2 (PORT C)
6 | 0x0006 | PCINT3_vect | Pin Change Interrupt 3 (PORT D)
7 | 0x0007 | WDT_vect | Watchdog Time-out (Interrupt Mode)
8 | 0x0008 | TIMER1_CAPT_vect | Timer/Counter1 capture event
9 | 0x0009 | TIMER1_COMPA_vect | Timer/Counter1 compare match A
10 | 0x000A | TIMER1_COMPB_vect | Timer/Counter1 compare match B
11 | 0x000B | TIMER1_OVF_vect | Timer/Counter1 overflow
12 | 0x000C | TIMER0_COMPA_vect | Timer/Counter0 compare match A
13 | 0x000D | TIMER0_COMPB_vect | Timer/Counter0 compare match B
14 | 0x000E | TIMER0_OVF_vect | Timer/Counter0 overflow
15 | 0x000F | SPI_STC_vect | SPI serial transfer complete
16 | 0x0010 | ADC_vect | ADC conversion complete
17 | 0x0011 | EE_RDY_vect | EEPROM ready
18 | 0x0012 | ANA_COMP_vect | Analog comparator
19 | 0x0013 | TWI_vect | 2-wire serial interface
