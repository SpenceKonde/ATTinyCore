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
This core includes an Optiboot bootloader for the ATtiny841/441, operating on the hardware UART0 (Serial) port at 115200 baud for 12mhz or higher, or 57600 when running at lower speeds. The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on the 841, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application (versions of the core prior to 1.2.0 used WDT vector, so WDT cannot be used as an interrupt - we recommend burning bootloader with the new version if this is an issue). A version of the bootloader that operates on Serial1 is included in the bootloaders folder, though you will have to program it manually or modify boards.txt to use it.

Programming the ATTiny841 via ISP without the bootloader is fully supported.

### Internal Oscillator voltage dependence
The internal oscillator is factory calibrated to +/- 2%. +/- 2% is good enough for serial communication - however, this spec is only valid below 4v - above 4v, the oscillator runs significantly faster; enough so that serial communication does not work. This would interfere with uploads using the bootloader - to work around this, a version of Optiboot is included built assuming the slightly higher operating frequency; this will be used if you select the >4.0v operating voltage prior to doing Burn Bootloader. Although this fixes uploads, it is an awkward workaround: Uploads won't work below 4v, and it doesn't address the issue within the sketch - for this reason is recommended to use a crystal when using the serial ports above 3.3v.

### I2C Support
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
