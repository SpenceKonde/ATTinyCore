### ATtiny 441/841
![x41 Pin Mapping](Pinout_x41.jpg "Arduino Pin Mapping for ATtiny x41")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 4096b / 8192b ( 3456b/7552b with bootloader)
RAM  | 256/512 bytes
EEPROM | 256/512 bytes
Bootloader | Yes, Optiboot w/Virtual Boot
GPIO Pins | 11
ADC Channels | 12 (including the one on reset), many differential channels
PWM Channels | 6
Interfaces | 2x UART, SPI, slave I2C
Clock options | Internal 1/8MHz, external crystal or clock* up to 16MHz, overclocking to 20MHz.
Packages | SOIC-14, MLF-20 (QFN), VQFN-20

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

Two pinout options are available, clockwise and counterclockwise. Be sure that the one you have selected is the one that you intend!

The ATtiny x41 series is a more advanced version of the ATtiny 84. It is pin compatible, though available only in surface mount packages, and offers an incredible array of peripherals, whilst costing only slightly more than an ATtiny 84. Tests have shown that despite manufacturer spec'ed max speed of 16mhz, they typically work without issue at 20mhz @ 5v and room temperature.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny841/441, operating on the hardware UART0 (Serial) port at 115200 baud for 12mhz or higher, or 57600 when running at lower speeds. The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on the 841, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application (versions of the core prior to 1.2.0 used WDT vector, so WDT cannot be used as an interrupt - we recommend burning bootloader with the new version if this is an issue). A version of the bootloader that operates on Serial1 is included as well (choose the desired UART when burning the booloader).

Programming the ATTiny841/441 via the selected ISP Programmer (without the bootloader) is also supported.

### Internal Oscillator voltage dependence
Previously, many users had encountered issues due to the voltage dependance of the oscillator. While the calibration is very tight 2.7 and 4v, as the voltage rises above 4.5v, the speed increases significantly. Although the magnitude of this is larger than on many of the more common parts, the issue is not as severe as had long been thought - the impact had been magnified by the direction of baud rate error, and the fact that many US ports actually supply 5.2-5.3v. As of 1.4.0, a simple solution was implemented to enable the same bootloader to work across the 8 MHz (Vcc < 4.5v) and 8 MHz (Vcc > 4.5 MHz ) - as well as the experimental 16 MHz internal bootloader. Additionally, sketched uploaded with the higher voltage option will nudge the Oscillator calibration down a bit, which can make a noticable improvement in timeing accuracy dramatically reduce the incidence of bard rate mismatch.

### Internal 16 MHz oscillator
The oscillator on the ATtiny841 and ATtiny441 is very unusual in terms of how high the frequency can be turned up. While the datasheet only claims 7.3 - 8.1 MHz - it is in fact possible to crank it up all the way to a full 16 MHz! Furthermore, the behavior is consistent enough that the vast majaority work without individual tuning, so this is now available in from the tools submenu with the other options. Unfortunately, because the same oscillator is used to time Flash and EEPROM writes, these should never be written to or erased while the device is running at 16 MHz (or for that matter, faster than 8.8, according to the datasheet). For the bootloader, this is handled by simply using the same bootloader as used for the 8 MHz Internal o[tions - the oscillator is reconfigured by the sketch, not the bootloader); when the part is reset to run the bootloader, k. The EEPROM is a more complicated issue - this was addressed in the EEPROM.h library included with the core by calling newly provided functions to slow the oscillator back down while each byte is written, and speed it back up and correct the millis()/micros() timekeeping for the disruption. During the write, all PWM frequencies will be halved, and on pins driven by Timer0, after this 3.3-3.4ms disruption a brief glitch can be seen on the PWM pins as the timer is put back in a state consistent with where it would have been had the disruption not occured. Due to this clock disruption, Serial data sent or received during this time will be mangled. When writing to the EEPROM, ensure that all data has been sent with Serial.flush(), and that it is not expected to receive any serial data during the write.

### I2C support
There is no I2C master functionality implemented in hardware. As of version 1.1.3, the included Wire.h library will use a software implementation to provide I2C master functionality, and the hardware I2C slave for slave functionality, and can be used as a drop-in replacement for Wire.h with the caveat that clock speed cannot be set.

### SPI Support
There is hardware SPI support. Use the normal SPI module.

### UART (Serial) Support
There are two hardware serial ports, Serial and Serial1. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels (for Serial1, use UCSR1B instead)
```
UCSR0B &=~(1<<TXEN0); // disable TX
UCSR0B &=~(1<<RXEN0); // disable RX
```
### Weird I/O-pin related features
There are a few strange features relating to the GPIO pins on the ATtiny x41 family which are found only in a small number of other parts released around the same time.

#### Special "high sink" pins
PA5 and PA7 have strongler sink capability than other pins - that is, the output voltage rises less when sinking a given amount of current.

```
PHDE=(1<<PHDEA0)|(1<<PHDEA1); //PHDEA0 controls PA5, PHDEA1 controls PA7.
```

This is no great shakes - the Absolute Maximum current rating of 40mA still applies and all... but it does pull closer to ground with a a "large" 10-20mA load. A very strange feature of these parts; as far as I can tell it is only found on on the x41 family and the closely related ATtiny828. Which is also the only classic AVR I know of that allows you to map the timer PWM outputs to different pins so freely.

#### Separate pullup-enable register
Like the ATtiny828 and ATtiny1634, these have a fourth register for each port, PUEx, which controls the pullups (rather than PORTx when DDRx has pin set as input). Unlike those other parts, though, these are not IO-space (they're normal registers, unlike the PORTx/PINx/DDRx registers, which can be used with the CBI/SBI/OUT/IN instructions).

#### Break-before-make
The ATtiny x41 family also has a "Break-Before-Make" mode that can be enabled on a per-port basis with the `PORTCR` register, which will keep the pin tristated for 1 system clock cycle when a DDR bit is set from input to output. This is not used by the core, and I'm not sure what the intended use case was...

```
PORTCR=(1<<BBMA)|(1<<BBMB); //BBMA controls PORTA, BBMB controls PORTB.
```

### Purchasing ATTiny841 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny841 boards through my Tindie store - your purchases support the continued development of this core.

![Picture of ATTiny841 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2015-06-16T05:30:56.533Z-T841RA_Assembled.png.855x570_q85_pad_rcrop.png)
### [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny841-dev-board-woptiboot-assembled/)
### [Bare Boards](https://www.tindie.com/products/DrAzzy/attiny84184-breakout-wserial-header-bare-board/)

### Interrupt Vectors

The names of interrupt vectors used in the 841 and 441 are:
```
INT0_vect
PCINT0_vect
PCINT1_vect
WDT_vect   (note: this is unavailable when using the bootloader before 1.2.0)
TIMER1_CAPT_vect
TIMER1_COMPA_vect
TIMER1_COMPB_vect
TIMER1_OVF_vect
TIMER0_COMPA_vect
TIMER0_COMPB_vect
TIMER0_OVF_vect
ANA_COMP0_vect
ADC_vect
EE_RDY_vect (note: this is unavailable when using the bootloader on 1.2.0 and later; note that it is not used by the EEPROM library)
ANA_COMP1_vect
TIMER2_CAPT_vect
TIMER2_COMPA_vect
TIMER2_COMPB_vect
TIMER2_OVF_vect
SPI_vect
USART0_START_vect
USART0_RX_vect
USART0_UDRE_vect
USART0_TX_vect
USART1_START_vect
USART1_RX_vect
USART1_UDRE_vect
USART1_TX_vect
TWI_SLAVE_vect
```
