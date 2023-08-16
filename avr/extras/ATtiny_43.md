# ATtiny43U
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
| LED_BUILTIN             |        PIN_PA5 |

## Overview
The ATtiny43 is a very unusual microcontroller - the featureset is unremarkable, inferior to just about anything else in the ATtiny product line - except for one thing: A built-in boost converter that allows it to run off of just ~1.1v (startup - it will keep running as low as 0.7v) - so you can run a project off a single alkaline battery. It generates ~3v while in active mode, and can provide up to 30mA for peripherals. When using the boost converter, you must run at 4MHz or less. The "Internal 4MHz" option sets the fuses to start at 1MHz and then switches to 4MHz upon startup. See the datasheet for details of the layout, external components required, and further details of the boost converter operation.

### First thing's first - do you really want this part?
If you are constrained to running from a single alkaline battery with an AVR, you really have little choice. The moment you can count on Vdd >= 1.8v you would have to be insane to use these. The flash is small, the peripherals lame. Frankly it has little to recommend it... except for that intelligent on-chip boost converter that lets you run from a single alkaline battery. If you can;t run from 2 AA batteries, or from a LiPo/similar, and you need to run a small non-intensive task from one measly alkaline cell, instantly everything is revolving around that and nothing else. The quiescent current of most boost converters is far too high. This one, on the other hand, has a bit of extra knowledge about the part, and two operating modes, Active Low Current and Active Regulated.

In Active Regulated mode, you get a constant 3.0V out with minimum ripple - but current draw is higher.

What's really interesting is active low power mode. This mode uses very little power,

### Boost Converter Capability
The boost converter will start up as long as the battery voltage is 1.2v or higher (according to the datasheet - in my testing, it seems to start at 1.1v), generating 3v. It will keep running as long as VBat is at least 0.8v, possibly even lower. It is capable of supplying 30mA to external devices as long as VBat > 1.0v (at lower VBat, the maximum current is lower, and if overloaded, the 3v output is, of course, not guaranteed). The main benefit here as compared to an external boost converter is realized when there  is minimal external load, and the chip spends a large portion of it's time in sleep modes, during which the demands on the boost converter are relaxed: the voltage is allowed to fall over the course of a cycle i, permitting a very low duty cycle, with the supply voltage jumping up and down ("active low power mode"), normally achieved when in sleep on processor and the power consumption falls significantly.

#### Boost converter part requirements
Obviously, you can buy my lovely tiny43 board with the buck converter ready to go. But if you're designing your own or integrating it into  It only requires four external components:
* One inductor

 You want, ideally, a device which is prohibited by the laws of physics - a diode with near zero forward drop, but even more importantly, you need a diode with very VERY low reverse leakage current. If you just optiomi


### PWM frequency
TC0 is always run in Fast PWM mode: We use TC0 for millis, and phase correct mode can't be used on the millis timer - you need to read the count to get micros, but that doesn't tell you the time in phase correct mode because you don't know if it's upcounting or downcounting in phase correct mode. On this part, the TC1 is uniquely bad - it has a different, shorter list of possible WGMs, and is only 8 bits.

| F_CPU  | F_PWM<sub>TC0</sub> | F_PWM<sub>TC1</sub>   | Notes                        |
|--------|---------------------|-----------------------|------------------------------|
| 1  MHz | 1/8/256=     488 Hz |  1/8/256=      488 Hz |                              |
| 2  MHz | 2/8/256=     977 Hz |  2/8/256=      977 Hz |                              |
| <4 MHz | x/8/256= 488 * x Hz |  x/8/512=  244 * x Hz | Phase correct TC1            |
| 4  MHz | 4/8/256=    1960 Hz |  4/8/512=      977 Hz | Phase correct TC1            |
| <8 MHz | x/64/256= 61 * x Hz |  x/8/512=  244 * x Hz | Between 4 and 8 MHz, the target range is elusive | Phase correct TC1 |
| 8  MHz | 8/64/256=    488 Hz |  8/64/256=     488 Hz |                              |
| >8 MHz | x/64/256= 61 * x Hz |  x/64/256=  61 * x Hz |                              |
| 12 MHz | 12/64/256=   735 Hz | 12/64/256=     735 Hz |                              |
| 16 MHz | 16/64/256=   977 Hz | 16/64/256=     977 Hz |                              |
|>16 MHz | x/64/256= 61 * x Hz |  x/64/512=  31 * x Hz | Phase correct TC1            |
| 20 MHz | 20/64/256=  1220 Hz | 20/64/512=     610 Hz | Phase correct TC1            |

Phase correct PWM counts up to 255, turning the pin off as it passes the compare value, updates it's double-buffered registers at TOP, then it counts down to 0, flipping the pin back as is passes the compare value. This is considered preferable for motor control applications; the "Phase and Frequency Correct" mode is not available on this part.

For more information see the [Changing PWM Frequency](Ref_ChangePWMFreq.md) reference.

### Tone Support
Tone() uses Timer1. For best results, use pin 5 or 6 (PIN_PB5, PIN_PB6) as this will use the hardware output compare to generate the square wave instead of using interrupts. Any use of tone() will take out PWM on pins PB5 and PB5. It doesn't do a great job because of the limitations of the timer these parts have.

### Servo support
No. if you'ure using a 43, you have 1.8-2V. Servos take 5-6, and it has a uniquely bad timer so adding support would be difficult and the result unsatisfactory

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. This is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all. We only support use of the builtin universal Wire.h library. If you try to use other libraries and encounter issues, please contact the author or maintainer of that library - there are too many of these poorly written libraries for us to provide technical support for.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI. This should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common, and the only mode that the SPI library has ever supported). As with I2C, we only support SPI through the included universal SPI library, not through any other libraries that may exist, and can provide no support for third party SPI libraries.

### UART (Serial) Support
There is no hardware UART support. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX defaults to AIN0 (PA4), RX is always on AIN1 (PA5). Although it is named Serial, it is still a software implementation, and you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](Ref_TinySoftSerial.md)

Though TX defaults to AIN0 (PA4), it can be moved to any pin on PORTA using Serial.setTxBit(b) where b is the number in the pin name using Pxn notation (only pins on PORTA are valid, and unless it is set for TX only, AIN1 (PA5) is not valid) (2.0.0+ only - was broken in earlier versions).

To disable the RX channel (to use only TX), select "TX only" from the Builtin SoftSerial tools menu. To disable the TX channel, simply don't print anything to it, and set it to the desired pinMode after Serial.begin()

### ADC Reference options
* DEFAULT: Vcc
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1

### Temperature Measurement
To measure the temperature, select the 1.1v internal voltage reference, and analogRead(ADC_TEMPERATURE); This value changes by approximately 1 LSB per degree C. This requires calibration on a per-chip basis to translate to an actual temperature, as the offset is not tightly controlled - take the measurement at a known temperature (we recommend 25C - though it should be close to the nominal operating temperature, since the closer to the single point calibration temperature the measured temperature is, the more accurate that calibration will be without doing a more complicated two-point calibration (which would also give an approximate value for the slope)) and store it in EEPROM (make sure that `EESAVE` fuse is set first, otherwise it will be lost when new code is uploaded via ISP) if programming via ISP, or at the end of the flash if programming via a bootloader (same area where oscillator tuning values are stored). See the section below for the recommended locations for these.s are stored). See the section below for the recommended locations for these.

### Tuning Constant Locations
These are the recommended locations to store tuning constants. In the case of OSCCAL, they are what are checked during startup when a tuned configuration is selected. They are not otherwise used by the core.

ISP programming: Make sure to have EESAVE fuse set, stored in EEPROM

|------------------------|-----------------|
| Temperature Offset     | E2END - 2       |
| Temperature Slope      | E2END - 1       |
| Tuned OSCCAL 8 MHz     | E2END           |

## Purchasing ATtiny43 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny43 boards through my Tindie store - your purchases support the continued development of this core.
* [Assembled Board, including boost converter](https://www.tindie.com/products/16617/)

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny43, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the handler will not exist, and the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging, which is one of the reasons I have done everything I can to make it impossible to turn off warnings. The addresses shown below are "word addressed" (1 word = 2 bytes - this is what is used internally by the hardware as well, since program memory is mostly read to read the next instruction, and the instructions are all 16 bits or 32 bits in length). As a part with 8k or less flash, these are 1 word vectors, all of which will always be an rjmp instruction pointing to the appropriate ISR (if any). vect_num is the number that you will be shown if you get a duplicate vector number. For more indepth treatment of this, see [my vector table description](https://github.com/SpenceKonde/AVR-Guidance/blob/master/LowLevel/VectorTable.md)

| num | Address| Vector Name        | Interrupt Definition                                          |
|-----|--------|--------------------|---------------------------------------------------------------|
|   0 | 0x0000 | RESET_vect         | Not an interrupt - this is a jump to the start of your code.  |
|   1 | 0x0001 | INT0_vect          | External Interrupt Request 0                                  |
|   2 | 0x0002 | PCINT0_vect        | Pin Change Interrupt 0 (PORT A)                               |
|   3 | 0x0003 | PCINT1_vect        | Pin Change Interrupt 1 (PORT B)                               |
|   4 | 0x0004 | WDT_vect           | Watchdog Time-out (interrupt mode)                            |
|   5 | 0x0005 | TIMER1_COMPA_vect  | Timer/Counter1 Compare Match A                                |
|   6 | 0x0006 | TIMER1_COMPB_vect  | Timer/Counter1 Compare Match B                                |
|   7 | 0x0007 | TIMER1_OVF_vect    | Timer/Counter1 Overflow                                       |
|   8 | 0x0008 | TIMER0_COMPA_vect  | Timer/Counter0 Compare Match A                                |
|   9 | 0x0009 | TIMER0_COMPB_vect  | Timer/Counter0 Compare Match B                                |
|  10 | 0x000A | TIMER0_OVF_vect    | Timer/Counter0 Overflow                                       |
|  11 | 0x000B | ANA_COMP_vect      | Analog Comparator                                             |
|  12 | 0x000C | ADC_vect           | ADC Conversion Complete                                       |
|  13 | 0x000D | EE_RDY_vect        | EEPROM Ready                                                  |
|  14 | 0x000E | USI_START_vect     | USI Start                                                     |
|  15 | 0x000F | USI_OVF_vect       | USI Overflow                                                  |
