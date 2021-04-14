### ATtiny 441/841
![x41 Pin Mapping](ATtiny_x41.png "Arduino Pin Mapping for ATtiny x41")

 Specifications |  .1
------------ | -------------
Flash (program memory)   | 4096b / 8192b ( 3456b/7552b with Optiboot, 6586b with Micronucleus)
RAM  | 256/512 bytes
EEPROM | 256/512 bytes
Bootloader | Yes, Optiboot (serial) or Micronucleus (VUSB)
GPIO Pins | 11
ADC Channels | 12 (including the one on reset), many differential channels
PWM Channels | 6
Interfaces | 2x UART, SPI, slave I2C
Clock options | Internal 1/8 MHz, ~16 MHz w/caveats, external crystal or clock* up to 16MHz, o/c to 20MHz.
Clock options | Micronucleus 8 MHz Internal, or 1 or ~16 MHz from 8 MHz internal
Packages | SOIC-14, MLF-20 (QFN), VQFN-20

* Manual steps required. See notes in README under "Using external CLOCK (not crystal)"

Two pinout options are available, clockwise and counterclockwise, see below for more information. Be sure that the one you have selected is the one that you intend!

The ATtiny x41-family is a more advanced version of the ATtiny x4-family. It is pin compatible, though available only in surface mount packages, and offers an incredible array of peripherals, whilst costing only slightly more than an ATtiny 84. Tests have shown that despite manufacturer spec'ed max speed of 16 MHz, they typically work without issue at 20 MHz @ 5v and room temperature.

## Programming
Any of these parts can be programmed by use of any ISP programmer. If using a version of Arduino prior to 1.8.13, be sure to choose a programmer with (ATTinyCore) after it's name (in 1.8.13 and later, only those will be shown), and connect the pins as normal for that ISP programmer.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny841/441, operating on the hardware UART0 (Serial) port at 115200 baud for 12 MHz or higher, or 57600 when running at lower speeds. The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on these parts, which do not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application (versions of the core prior to 1.2.0 used WDT vector, so WDT cannot be used as an interrupt - we recommend burning bootloader with the new version if this is an issue). A version of the bootloader that operates on Serial1 is included as well (choose the desired UART when burning the booloader).

### Micronucleus VUSB Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny841, allowing sketches to be uploaded directly over USB. The board definition runs at 8 MHz via the internal oscillator. For low power applications, it can be prescaled as listed in the table, or cranked up to 16 MHz for the adventurous. See the document on [Micronucleus usage](UsingMicronucleus.md) for more information. In order to achieve the 12 MHz clock during USB operation, the OSCCAL is drastically increased to 12MHz while the bootloader is running, but is set back down before running the sketch. D- is on PIN_PB0, D+ is on PIN_PB1.

**Currently the version of micronucleus supplied with ATTinyCore enters the bootloader on external reset only (hence it cannot be used with the reset pin set as GPIO), like the Nanite841. This will be made an option in a future release, as will higher clock frequencies, including ones high enough for USB libraries**

## Features

### Internal Oscillator voltage dependence
Prior to 1.4.0, many users had encountered issues due to the voltage dependence of the oscillator. While the calibration is very accurate between 2.7 and 4v, as the voltage rises above 4.5v, the speed increases significantly. Although the magnitude of this is larger than on many of the more common parts, the issue is not as severe as had long been thought - the impact had been magnified by the direction of baud rate error, and the fact that many US ports actually supply 5.2-5.3v. As of 1.4.0, a simple solution was implemented to enable the same bootloader to work across the 8 MHz (Vcc < 4.5v) and 8 MHz (Vcc > 4.5 MHz ) board definitions, as well as the 16 MHz Internal option (albeit running at 8MHz) - it should generally work between 2.7v and 5.25v - though the extremes of that range may be dicey. We do still provide a >4.5v clock option in order to improve behavior of the running sketch - it will nudge the oscillator calibration down to move it closer to the nominal 8MHz clock speed; sketches uploaded with the higher voltage option. This is not perfect, but it is generally good enough to work with Serial on around 5v (including 5.25v often found on USB ports to facilitate chargeing powerhungry devices), and millis()/micros() will keep better time than in previous versions.

### Internal 16 MHz oscillator?!
The oscillator on the ATtiny841 and ATtiny441 is very unusual in terms of how high the frequency can be turned up. While the datasheet only claims 7.3 - 8.1 MHz - it is in fact possible to crank it up all the way to a full 16 MHz! Furthermore, the behavior is consistent enough that the vast majaority work without individual tuning, so this is now available from the tools submenu with the other options. Unfortunately, because the same oscillator is used to time Flash and EEPROM writes, these should never be written to or erased while the device is running at 16 MHz (or for that matter, faster than 8.8, according to the datasheet). For the bootloader, this is handled by simply using the same bootloader as used for the 8 MHz Internal options - the oscillator is reconfigured by the sketch, not the bootloader); when the part is reset to run the bootloader, k. **The EEPROM is a more complicated issue** - this was addressed in the EEPROM.h library included with the core by calling newly provided functions to slow the oscillator back down while each byte is written, and speed it back up and correct the millis()/micros() timekeeping for the disruption. **During the write, all PWM frequencies will be halved, and on pins driven by Timer0, after this 3.3-3.4ms disruption a brief glitch can be seen on the PWM pins as the timer is put back in a state consistent with where it would have been had the disruption not occurred.** Due to this clock disruption, **Serial data sent or received during this time will be mangled**. When writing to the EEPROM, ensure that all data has been sent with Serial.flush(), and that it is not expected to receive any serial data during the write.

### Pin mapping options
Throughout the history of ATtiny Arduino support, two pin mappings have been used. Here, they are referred to descriptively: the clockwise, and counterclockwise pinout. The desired pin mapping can be chosen from the Tools -> Pin Mapping submenu. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for PINMAPPING_CCW or PINMAPPING_CW macro (eg, `#ifdef PINMAPPING_CCW` - I would recommend checking for the incompatible one, and immediately #error'ing if you find it). Alternately, also as of 1.4.0, with either pin mapping selected, you can always refer to pins by their port and number within that port, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7, which is pin 7 in the clockwise mapping and pin 3 in the counterclockwise mapping (don't use PIN_xn or Pxn) - in this case the pin mapping won't matter.

Example of a "guard" against wrong pin mapping:
```
#ifdef PINMAPPING_CCW
#error "Sketch was written for clockwise pin mapping!"
#endif
```

### Tone support
The standard Tone() function is supported on these parts. For best results, use PA5 (pin 5 on either pinout) or PA6 (pin 4 on counterclockwise, pin 6 on clockwise), as this will use hardware output compare to generate the square wave, instead of interrupts.

### I2C support
There is no I2C master functionality implemented in hardware. As of version 1.1.3, the included Wire.h library will use a software implementation to provide I2C master functionality, and the hardware I2C slave for slave functionality, and can be used as a drop-in replacement for Wire.h with the caveat that clock speed cannot be set. **You must have external pullup resistors installed** in order for I2C functionality to work reliably.

### Advanced Differential ADC
The ATtinyx41 family boasts one of the most advanced ADCs in the entire classic tinyAVR/megaAVR product line, with no fewer than 62 channels (counting single ended and differential pairs - admittedly, some of the differential pairs are of dubious usefulness, consisting of the same channel compared to itself; presumably this is meant as a way to adjust for differences between the positive and negative sides of the ADC), 8 reference options, and - for the differential pairs, selectable gain of 1x, 20x, or 100x - if you happen to need insane ADC capability in a small AVR, there is still, as of mid-2020, no other 8-bit AVR - to my knowledge - that can compete. See the table at the bottom of this page for a list of the channels; they can all be used with analogRead(). Of course, it can also just be used as an normal AVR where analogRead() happens to work on every pin, too.

### SPI Support
There is hardware SPI support. Use the normal SPI module.

### UART (Serial) Support
There are two hardware serial ports, Serial and Serial1. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels (for Serial1, use UCSR1B instead)
```
UCSR0B &=~(1<<TXEN0); // disable TX
UCSR0B &=~(1<<RXEN0); // disable RX
```

### Overclocking
Experience has shown that the ATtiny x41-family, operating at 5v and room temperature, will typically function at 20 MHz at 5v and room temperature without issue, although this is outside of the manufacturer's specification.

### Weird I/O-pin related features
There are a few strange features relating to the GPIO pins on the ATtiny x41 family which are found only in a small number of other parts released around the same time.

#### Special "high sink" pins
PA5 and PA7 have strongler sink capability than other pins - that is, the output voltage rises less when sinking a given amount of current.

```
PHDE=(1<<PHDEA0)|(1<<PHDEA1); //PHDEA0 controls PA5, PHDEA1 controls PA7.
```

This is no great shakes - the Absolute Maximum current rating of 40mA still applies and all... but it does pull closer to ground with a a "large" 10-20mA load. A very strange feature of these parts; as far as I can tell it is only found on on the x41 family and the closely related ATtiny828. Which is also the only classic AVR I know of that allows you to map the timer PWM outputs to different pins the way these parts do.

**These high-sink pins have asymmetric drivers - thus PA5 and PA7 should not be used with PWM + RC filter to generate analog voltages** as the resulting voltage will be lower than expected.

#### Separate pullup-enable register
Like the ATtiny828 and ATtiny1634, these have a fourth register for each port, PUEx, which controls the pullups (rather than PORTx when DDRx has pin set as input). Unlike those other parts, though, these are not IO-space (they're normal registers, unlike the PORTx/PINx/DDRx registers, which can be used with the CBI/SBI/OUT/IN instructions).

#### Break-before-make
The ATtiny x41 family also has a "Break-Before-Make" mode that can be enabled on a per-port basis with the `PORTCR` register, which will keep the pin tristated for 1 system clock cycle when a DDR bit is set from input to output. This is not used by the core, and I'm not sure what the intended use case was...

```
PORTCR=(1<<BBMA)|(1<<BBMB); //BBMA controls PORTA, BBMB controls PORTB.
```

## Purchasing ATtiny841 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny841 boards through my Tindie store - your purchases support the continued development of this core!

![Picture of ATtiny841 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2015-06-16T05:30:56.533Z-T841RA_Assembled.png.855x570_q85_pad_rcrop.png)
### [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny841-dev-board-woptiboot-assembled/)
### [Bare Boards](https://www.tindie.com/products/DrAzzy/attiny84184-breakout-wserial-header-bare-board/)
The Wattuino board does not support maintenance of this core, but it does support the work they did to get that working:
### [Wattuino Nanite (VUSB)](https://shop.watterott.com/Wattuino-Nanite-841-ATtiny841-with-USB-Bootloader)


## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x41-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things.
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
12 | 0x000C | ANA_COMP0_vect | Analog Comparator 0
13 | 0x000D | ADC_READY_vect | ADC Conversion Complete
14 | 0x000E | EE_RDY_vect | EEPROM Ready
15 | 0x000F | ANA_COMP1_vect | Analog Comparator 1
16 | 0x0010 | TIM2_CAPT_vect | Timer/Counter2 Capture Event
16 | 0x0010 | TIMER2_CAPT_vect | Alias - provided by ATTinyCore
17 | 0x0011 | TIM2_COMPA_vect | Timer/Counter2 Compare Match A
17 | 0x0011 | TIMER2_COMPA_vect | Alias - provided by ATTinyCore
18 | 0x0012 | TIM2_COMPB_vect | Timer/Counter2 Compare Match B
18 | 0x0012 | TIMER2_COMPB_vect | Alias - provided by ATTinyCore
19 | 0x0013 | TIM2_OVF_vect | Timer/Counter2 Overflow
19 | 0x0013 | TIMER2_OVF_vect | Alias - provided by ATTinyCore
20 | 0x0014 | SPI_vect | SPI Serial Transfer Complete
21 | 0x0015 | USART0_RXS_vect | USART0 Rx Start
22 | 0x0016 | USART0_RXC_vect | USART0 Rx Complete
23 | 0x0017 | USART0_DRE_vect | USART0 Data Register Empty
24 | 0x0018 | USART0_TXC_vect | USART0 Tx Complete
25 | 0x0019 | USART1_RXS_vect | USART1 Rx Start
26 | 0x001A | USART1_RXC_vect | USART1 Rx Complete
27 | 0x001B | USART1_DRE_vect | USART1 Data Register Empty
28 | 0x001C | USART1_TXC_vect | USART1 Tx Complete
29 | 0x001D | TWI_vect | TWI Slave Interrupt
30 | 0x001E | QTRIP_vect | QTouch

## ADC Channels
Below, MUXPOS refers to the internal "channel" used (value of ADMUXA). You can pass this value directly to analogRead().

### Reference options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference, not connected to AREF
* INTERNAL: synonym for INTERNAL1V1
* INTERNAL1V1_AREF: Internal 1.1v reference, connected to AREF
* INTERNAL2V2: Internal 2.2v reference, not connected to AREF
* INTERNAL2V2_AREF: Internal 2.2v, connected to AREF
* INTERNAL4V096: 4.096V, not connected to AREF
* INTERNAL4V096_AREF: 4.096V, connected to AREF

### Single Ended
MUXPOS | ADC Channel | Pin
------ | ------- | ------
0x00 | ADC0 | PIN_PA0
0x01 | ADC1 | PIN_PA1
0x02 | ADC2 | PIN_PA2
0x03 | ADC3 | PIN_PA3
0x04 | ADC4 | PIN_PA4
0x05 | ADC5 | PIN_PA5
0x06 | ADC6 | PIN_PA6
0x07 | ADC7 | PIN_PA7
0x08 | ADC8 | PIN_PB2
0x09 | ADC9 | PIN_PB3
0x0A | ADC10 | PIN_PB1
0x0B | ADC11 | PIN_PB0
0x0C | Temperature sensor |
0x0D | Internal 1.1V reference |
0x0E | 0V (AGND) |
0x0F | Supply voltage |
0x2E | Unused |
0x2F | Unused |

### Differential

MUXPOS | Pos. Chan | Pos. Pin | Neg. Chan | Neg. Pin
------ | ------- | ------ | -------| ----
0x10 | ADC0 | PIN_PA0 | ADC1 | PIN_PA1
0x11 | ADC0 | PIN_PA0 | ADC3 | PIN_PA3
0x12 | ADC1 | PIN_PA1 | ADC2 | PIN_PA2
0x13 | ADC1 | PIN_PA1 | ADC3 | PIN_PA3
0x14 | ADC2 | PIN_PA2 | ADC3 | PIN_PA3
0x15 | ADC3 | PIN_PA3 | ADC4 | PIN_PA4
0x16 | ADC3 | PIN_PA3 | ADC5 | PIN_PA5
0x17 | ADC3 | PIN_PA3 | ADC6 | PIN_PA6
0x18 | ADC3 | PIN_PA3 | ADC7 | PIN_PA7
0x19 | ADC4 | PIN_PA4 | ADC5 | PIN_PA5
0x1A | ADC4 | PIN_PA4 | ADC6 | PIN_PA6
0x1B | ADC4 | PIN_PA4 | ADC7 | PIN_PA7
0x1C | ADC5 | PIN_PA5 | ADC6 | PIN_PA6
0x1D | ADC5 | PIN_PA5 | ADC7 | PIN_PA7
0x1E | ADC6 | PIN_PA6 | ADC7 | PIN_PA7
0x1F | ADC8 | PIN_PB2 | ADC9 | PIN_PB3
0x20 | ADC0 | PIN_PA0 | ADC0 | PIN_PA0
0x21 | ADC1 | PIN_PA1 | ADC1 | PIN_PA1
0x22 | ADC2 | PIN_PA2 | ADC2 | PIN_PA2
0x23 | ADC3 | PIN_PA3 | ADC3 | PIN_PA3
0x24 | ADC4 | PIN_PA4 | ADC4 | PIN_PA4
0x25 | ADC5 | PIN_PA5 | ADC5 | PIN_PA5
0x26 | ADC6 | PIN_PA6 | ADC6 | PIN_PA6
0x27 | ADC7 | PIN_PA7 | ADC7 | PIN_PA7
0x28 | ADC8 | PIN_PB2 | ADC8 | PIN_PB2
0x29 | ADC9 | PIN_PB3 | ADC9 | PIN_PB3
0x2A | ADC10 | PIN_PB1 | ADC8 | PIN_PB2
0x2B | ADC10 | PIN_PB1 | ADC9 | PIN_PB2
0x2C | ADC11 | PIN_PB0 | ADC8 | PIN_PB2
0x2D | ADC11 | PIN_PB0 | ADC9 | PIN_PB2
0x30 | ADC1 | PIN_PA1 | ADC0 | PIN_PA0
0x31 | ADC3 | PIN_PA3 | ADC0 | PIN_PA0
0x32 | ADC2 | PIN_PA2 | ADC1 | PIN_PA1
0x33 | ADC3 | PIN_PA3 | ADC1 | PIN_PA1
0x34 | ADC3 | PIN_PA3 | ADC2 | PIN_PA2
0x35 | ADC4 | PIN_PA4 | ADC3 | PIN_PA3
0x36 | ADC5 | PIN_PA5 | ADC3 | PIN_PA3
0x37 | ADC6 | PIN_PA6 | ADC3 | PIN_PA3
0x38 | ADC7 | PIN_PA7 | ADC3 | PIN_PA3
0x39 | ADC5 | PIN_PA5 | ADC4 | PIN_PA4
0x3A | ADC6 | PIN_PA6 | ADC4 | PIN_PA4
0x3B | ADC7 | PIN_PA7 | ADC4 | PIN_PA4
0x3C | ADC6 | PIN_PA6 | ADC5 | PIN_PA5
0x3D | ADC7 | PIN_PA7 | ADC5 | PIN_PA5
0x3E | ADC7 | PIN_PA7 | ADC6 | PIN_PA6
0x3F | ADC9 | PIN_PB3 | ADC8 | PIN_PB2
