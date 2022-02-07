### ATtiny43U
![43 pin mapping](Pinout_43.jpg "Arduino Pin Mapping for ATtiny 43")

| Specification           |    ATtiny43U   |
|-------------------------|----------------|
| Uploading uses          |   ISP/SPI pins |
| Flash available user    |     4096 bytes |
| RAM                     |      256 bytes |
| EEPROM                  |       64 bytes |
| GPIO Pins               |             15 |
| ADC Channels            |              4 |
| Differential ADC        |             No |
| PWM Channels            |        4 fixed |
| Interfaces (cont'd)     |            USI |
| Clocking Options:       |         in MHz |
| Int. Oscillator         |     8, 4, 2, 1 |
| Int. WDT Oscillator     |        128 kHz |
| Internal, with tuning   |  Not supported |
| External Crystal        |  Not supported |
| External Clock          |              8 |
| LED_BUILTIN             |            PA5 |

The ATtiny43 is a very unusual microcontroller - the featureset is unremarkable, inferior to just about anything else in the ATtiny product line - except for one thing: A built-in boost converter that allows it to run off of just ~1.1v (startup - it will keep running as low as 0.7v) - so you can run a project off a single alkaline battery. It generates ~3v while in active mode, and can provide up to 30mA for peripherals. When using the boost converter, you must run at 4MHz or less. The "Internal 4MHz" option sets the fuses to start at 1MHz and then switches to 4MHz upon startup. See the datasheet for details of the layout, external components required, and further details of the boost converter operation.

### Boost Converter Capability The boost converter will start up as long as
the battery voltage is 1.2v or higher (according to the datasheet - in my
testing, it seems to start at 1.1v), generating 3v. It will keep running as
long as VBat is at least 0.8v, possibly even lower. It is capable of supplying
30mA to external devices as long as VBat > 1.0v (at lower VBat, the maximum
current is lower, and if overloaded, the 3v output is, of course, not
guaranteed). The main benefit here as compared to an external boost converter
is realized when there  is minimal external load, and the chip spends a large
portion of it's time in sleep modes, during which the demands on the boost
converter are relaxed: the voltage is allowed to fall over the course of a
cycle i, permitting a very low dty cyv and the power consumption falls
significantly.

### Tone Support
Tone() uses Timer1. For best results, use pin 5 or 6 (PIN_PB5, PIN_PB6) as this will use the hardware output compare to generate the square wave instead of using interrupts. Any use of tone() will take out PWM on pins 5 amd 6.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common).

### UART (Serial) Support
There is no hardware UART support. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX defaults to AIN0 (PA4), RX is always on AIN1 (PA5). Although it is named Serial, it is still a software implementation, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](TinySoftSerialBaud.md)

Though TX defaults to AIN0 (PA4), it can be moved to any pin on PORTA using Serial.setTxBit(b) where b is the number in the pin name using Pxn notation (only pins on PORTA are valid, and unless it is set for TX only, AIN1 (PA5) is not valid) (2.0.0+ only - was broken in earlier versions).

To disable the RX channel (to use only TX), select "TX only" from the Builtin SoftSerial tools menu. To disable the TX channel, simply don't print anything to it, and set it to the desired pinMode after Serial.begin()

### ADC Reference options
* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1

### Tuning Constant Locations
These are the recommended locations to store tuning constants. In the case of OSCCAL, they are what are checked during startup when a tuned configuration is selected. They are not otherwiseused by the core.

ISP programming: Make sure to have EESAVE fuse set, stored in EEPROM

|------------------------|-----------------|
| Temperature Offset     | E2END - 2       |
| Temperature Slope      | E2END - 1       |
| Tuned OSCCAL 8 MHz     | E2END           |

## Purchasing ATtiny43 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny841 boards through my Tindie store - your purchases support the continued development of this core.
* [Assembled Board, including boost converter](https://www.tindie.com/products/16617/)
* **Bare Boards** Coming soon


## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny43, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number that you will be shown if you get a duplicate vector number.

| num | address| Vector Name        | Interrupt Definition
|-----|--------|--------------------|--------------------------------------
|   0 | 0x0000 | RESET_vect         | Any reset (pin, WDT, power-on, BOD)
|   1 | 0x0001 | INT0_vect          | External Interrupt Request 0
|   2 | 0x0002 | PCINT0_vect        | Pin Change Interrupt 0 (PORT A)
|   3 | 0x0003 | PCINT1_vect        | Pin Change Interrupt 1 (PORT B)
|   4 | 0x0004 | WDT_vect           | Watchdog Time-out (interrupt mode)
|   5 | 0x0005 | TIMER1_COMPA_vect  | Timer/Counter1 Compare Match A
|   6 | 0x0006 | TIMER1_COMPB_vect  | Timer/Counter1 Compare Match B
|   7 | 0x0007 | TIMER1_OVF_vect    | Timer/Counter1 Overflow
|   8 | 0x0008 | TIMER0_COMPA_vect  | Timer/Counter0 Compare Match A
|   9 | 0x0009 | TIMER0_COMPB_vect  | Timer/Counter0 Compare Match B
|  10 | 0x000A | TIMER0_OVF_vect    | Timer/Counter0 Overflow
|  11 | 0x000B | ANA_COMP_vect      | Analog Comparator
|  12 | 0x000C | ADC_vect           | ADC Conversion Complete
|  13 | 0x000D | EE_RDY_vect        | EEPROM Ready
|  14 | 0x000E | USI_START_vect     | USI Start
|  15 | 0x000F | USI_OVF_vect       | USI Overflow
