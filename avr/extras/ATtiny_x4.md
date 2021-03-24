### ATtiny 24/44/84(a)
![x4 pin mapping](http://drazzy.com/e/img/PinoutT84a.jpg "Arduino Pin Mapping for ATtiny x4-family")

 Specifications |  .
------------ | -------------
Flash (program memory)   | 2048b/4096b/8192b (3456b/7552b with Optiboot, 6522b with Micronucleus)
RAM           | 128/256/512 bytes
EEPROM | 128/256/512 bytes
Bootloader | Yes, Optiboot (serial) or Micronucleus (VUSB)
GPIO Pins | 11
ADC Channels | 8, plus many differential channels
PWM Channels | 4
Interfaces | USI
Clock options | Internal 1/8 MHz, external crystal or clock* up to 20 MHz
Clock options | Micronucleus 8 MHz, 8/4/1 MHz from 8 MHz internal
Packages | SOIC-14, DIP-14, MLF-20 (QFN)

The 24/44/84 and 24a/44a/84a are functionally identical; the latter replaced the former in 2008, and uses slightly less power, and actual ATtiny84 parts are rarely seen in circulation today. They have the same signatures and are fully interchangible. It is extremely common to refer to the ATtiny84a as an ATtiny84.

* Manual steps required. See notes in README under "Using external CLOCK (not crystal).

Two pinouts are available for compatibility with other cores, see below for more information. Be sure you are using the pinout you think you are!

## Programming
All of these parts can be programmed via an ISP

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny84/44, operating using software serial at 19200 baud - the software serial uses the AIN0 and AIN1 pins (see UART section below). The bootloader uses 640b of space, leaving 3456 or7552b available for user code. In order to work on the 84, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

Programming the ATtiny84/44 via ISP without the bootloader is fully supported; the 24 is supported only for ISP programming.

### Micronucleus Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny84(a), allowing sketches to be uploaded directly over USB. It runs at 8 MHz via the internal oscillator. For low power applications, it can be prescaled as listed in the table. See the document on [Micronucleus usage](UsingMicronucleus.md) for more information. In order to achieve the 12 MHz clock during USB operation, the OSCCAL is drastically increased to 12MHz, but is set back down before running the sketch. USB libraries are not supported (yet). D- is on PIN_PB0, D+ is on PIN_PB1.

**Currently the version of micronucleus supplied with ATTinyCore enters the bootloader for all reset sources. This will be made an option in a future release, as will higher clock frequencies, including ones high enough for USB libraries**

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
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common).

### UART (Serial) Support
There is no hardware UART. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](TinySoftSerialBaud.md)

To disable the RX channel (to use only TX), the following commands should be used after calling Serial.begin(). No special action is needed to disable the TX line if only RX is needed.
```
ACSR &=~(1<<ACIE);
ACSR |=~(1<<ACD);
```

## ADC Features
The ATtiny84 has a surprisingly sophisticated ADC with multiple differential channels and a gain of 1x or 20x As of ATTinyCore 2.0.0, these are available through analogRead!  When used to read a pair of analog pins in differential mode, the ADC normally runs in unipolar mode: The voltage on the positive pin must be higher than that on the negative one, but the difference is measured to the full precision of the ADC. It can be put into bipolar mode, where the voltage on the negative side can go below the voltage on the positive side and generate meaningful measurements (it will return a signed value, which costs 1 bit of accuracy for the sign bit). This can be enabled by calling the helper function `setADCBipolarMode(true or false)`. On many AVR devices with a differential ADC, only bipolar mode is available.

### Reference options
* DEFAULT: Vcc
* EXTERNAL: Voltage applied to AREF pin
* INTERNAL1V1: Internal 1.1v reference
* INTERNAL: synonym for INTERNAL1V1

### Internal Sources
The following voltages can also be measured using the ADC
* ADC_INTERNAL1V1
* ADC_GROUND
* ADC_TEMPERATURE

### Differential ADC
There are 12 differential pairs available, all with selectable gain. On A0, A3, and A7, the positive and negative inputs can be set to the same pin; these can be used to determine the offset error in the gain stage, which can then be subtracted from future measurements. All pairs can have the two inputs swapped, too. ATTinyCore (v2.0.0+) allows you to read from them with `analogRead()` by using the channel names shown below. If it is required to know the numeric values of the channels, they are shown below as well. If you must work with channel numbers, instead of a names, when passing them to `analogRead()`, use the `ADC_CH()` macro (ex: `analogRead(ADC_CH(0x08))` to read ADC0 and ADC1 at 20x gain, equivalent to `analogRead(DIFF_A0_A1_20X)`), otherwise they will be interpreted as a (likely non-existent) digital pin (any time an analog channel is used, the high bit is set, and on numbers used for digital pins, it is not. So `ADC_CH()` is just `#define ADC_CH(x) (0x80 | (x))`  - but it's a heckova lot easier to understand when reading code). In the case of duplicates in the list of gain options, the one that does not use the `GSEL` bit to select the gain is named followed by an A (ex, `DIFF_A6_A5_20XA`).

### Differential ADC channels
|  Positive  |  Negative  |Gain 1x|Gain 20x| Name (1x Gain) | Name (20x Gain) |
|------------|------------|-------|--------|----------------|-----------------|
| ADC0 (PA0) | ADC0 (PA0) |       |  0x23  |                |  DIFF_A0_A0_20X |
| ADC0 (PA0) | ADC1 (PA1) | 0x08  |  0x09  |  DIFF_A0_A1_1X |  DIFF_A0_A1_20X |
| ADC0 (PA0) | ADC3 (PA3) | 0x0A  |  0x0B  |  DIFF_A0_A3_1X |  DIFF_A0_A3_20X |
| ADC1 (PA1) | ADC0 (PA0) | 0x28  |  0x29  |  DIFF_A1_A0_1X |  DIFF_A1_A0_20X |
| ADC1 (PA1) | ADC2 (PA2) | 0x0C  |  0x0D  |  DIFF_A1_A2_1X |  DIFF_A1_A2_20X |
| ADC1 (PA1) | ADC3 (PA3) | 0x0E  |  0x0F  |  DIFF_A1_A3_1X |  DIFF_A1_A3_20X |
| ADC2 (PA2) | ADC1 (PA1) | 0x2C  |  0x2D  |  DIFF_A2_A1_1X |  DIFF_A2_A1_20X |
| ADC2 (PA2) | ADC3 (PA3) | 0x10  |  0x11  |  DIFF_A2_A3_1X |  DIFF_A2_A3_20X |
| ADC3 (PA3) | ADC0 (PA0) | 0x2A  |  0x2B  |  DIFF_A3_A0_1X |  DIFF_A3_A0_20X |
| ADC3 (PA3) | ADC1 (PA1) | 0x2E  |  0x2F  |  DIFF_A3_A1_1X |  DIFF_A3_A1_20X |
| ADC3 (PA3) | ADC2 (PA2) | 0x30  |  0x31  |  DIFF_A3_A2_1X |  DIFF_A3_A2_20X |
| ADC3 (PA3) | ADC3 (PA3) | 0x24  |  0x25  |  DIFF_A3_A3_1X |  DIFF_A3_A3_20X |
| ADC3 (PA3) | ADC4 (PA4) | 0x12  |  0x13  |  DIFF_A3_A4_1X |  DIFF_A3_A4_20X |
| ADC3 (PA3) | ADC5 (PA5) | 0x14  |  0x15  |  DIFF_A3_A5_1X |  DIFF_A3_A5_20X |
| ADC3 (PA3) | ADC6 (PA6) | 0x16  |  0x17  |  DIFF_A3_A6_1X |  DIFF_A3_A6_20X |
| ADC3 (PA3) | ADC7 (PA7) | 0x18  |  0x19  |  DIFF_A3_A7_1X |  DIFF_A3_A7_20X |
| ADC4 (PA4) | ADC3 (PA3) | 0x32  |  0x33  |  DIFF_A4_A3_1X |  DIFF_A4_A3_20X |
| ADC4 (PA4) | ADC5 (PA5) | 0x1A  |  0x1B  |  DIFF_A4_A5_1X |  DIFF_A4_A5_20X |
| ADC5 (PA5) | ADC3 (PA3) | 0x34  |  0x35  |  DIFF_A5_A3_1X |  DIFF_A5_A3_20X |
| ADC5 (PA5) | ADC4 (PA4) | 0x3A  |  0x3B  |  DIFF_A5_A4_1X |  DIFF_A5_A4_20X |
| ADC5 (PA5) | ADC6 (PA6) | 0x1C  |  0x1D  |  DIFF_A5_A6_1X |  DIFF_A5_A6_20X |
| ADC6 (PA6) | ADC3 (PA3) | 0x36  |  0x37  |  DIFF_A6_A3_1X |  DIFF_A6_A3_20X |
| ADC6 (PA6) | ADC5 (PA5) | 0x3C  |  0x3D  |  DIFF_A6_A5_1X |  DIFF_A6_A5_20X |
| ADC6 (PA6) | ADC7 (PA7) | 0x1E  |  0x1F  |  DIFF_A6_A7_1X |  DIFF_A6_A7_20X |
| ADC7 (PA7) | ADC3 (PA3) | 0x38  |  0x39  |  DIFF_A7_A3_1X |  DIFF_A7_A3_20X |
| ADC7 (PA7) | ADC6 (PA6) | 0x3E  |  0x3F  |  DIFF_A7_A6_1X |  DIFF_A7_A6_20X |
| ADC7 (PA7) | ADC7 (PA7) | 0x26  |  0x27  |  DIFF_A7_A7_1X |  DIFF_A7_A7_20X |

#### ADC Differential Pair Matrix

 | N\P |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
 |-----|----|----|----|----|----|----|----|----|
 |   0 |  X |  X |    |  X |    |    |    |    |
 |   1 |  X |    |  X |  X |    |    |    |    |
 |   2 |    |  X |    |  X |    |    |    |    |
 |   3 |  X |  X |  X |  X |  X |  X |  X |  X |
 |   4 |    |    |    |  X |    |  X |    |    |
 |   5 |    |    |    |  X |  X |    |  X |    |
 |   6 |    |    |    |  X |    |  X |    |  X |
 |   7 |    |    |    |  X |    |    |  X |  X |


By default, differential measurements are taken with the gain stage in "unipolar" mode; in order to get meaningful data, the positive side must be higher than the negative side. This is great if you you know ahead of time that one of them will definitely be higher (and often you do). But you can also use it in bipolar mode - in this mode there are 9 bits of resolution, plus a sign bit; by letting it go negative,

### Temperature Measurement
To measure the temperature, select the 1.1v internal voltage reference, and analogRead(ADC_TEMPERATURE); This value changes by approximately 1 LSB per degree C. This requires calibration on a per-chip basis to translate to an actual temperature, as the offset is not tightly controlled - take the measurement at a known temperature (we recommend 25C - though it should be close to the nominal operating temperature, since the closer to the single point calibration temperature the measured temperature is, the more accurate that calibration will be without doing a more complicated two-point calibration (which would also give an approximate value for the slope)) and store it in EEPROM (make sure that `EESAVE` fuse is set first, otherwise it will be lost when new code is uploaded via ISP) if programming via ISP, or at the end of the flash if programming via a bootloader (same area where oscillator tuning values are stored). See the section below for the recommended locations for these.


## Purchasing ATtiny84 Boards
As the ATtiny84 is available in an easy-to-solder through-hole DIP package, a board can be easily made by just soldering the part into prototyping board.
I (Spence Konde) sell a specialized prototyping board that combines an ISP header with prototyping space and outlines to fit common SMD parts.
* [ATtiny84 prototyping board](https://www.tindie.com/products/drazzy/attiny84-project-board/)
* Micronucleus boards can be bought from one of my collaborators: [Micronucleus ATtiny84a](https://www.tindie.com/products/svdbor/tiniest-arduino-compatible-board-with-micronucleus/)

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x4-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses are "word addressed". vect_num is the number you are shown in the event of a duplicate vector error, among other things. As shown in the below table, the core provides aliases of the names timer interrupts with names starting with TIMn and TIMERn. During the era that these parts were released, Atmel was not naming their vectors consistently. The names starting with TIMERn are to be preferred.

| vect_num | Vector Address |    Vector Name    |      Interrupt Definition           |
|----------|----------------|-------------------|-------------------------------------|
|        0 |        0x0000  | RESET_vect        | Any reset (pin, WDT, power-on, BOD) |
|        1 |        0x0001  | INT0_vect         | External Interrupt Request 0        |
|        2 |        0x0002  | PCINT0_vect       | Pin Change Interrupt 0 (PORT A)     |
|        3 |        0x0003  | PCINT1_vect       | Pin Change Interrupt 1 (PORT B)     |
|        4 |        0x0004  | WDT_vect          | Watchdog Time-out (Interrupt Mode)  |
|        5 |        0x0005  | TIM1_CAPT_vect    | Timer/Counter1 Capture Event        |
|        5 |        0x0005  | TIMER1_CAPT_vect  | Alias - provided by ATTinyCore      |
|        6 |        0x0006  | TIM1_COMPA_vect   | Timer/Counter1 Compare Match A      |
|        6 |        0x0006  | TIMER1_COMPA_vect | Alias - provided by ATTinyCore      |
|        7 |        0x0007  | TIM1_COMPB_vect   | Timer/Counter1 Compare Match B      |
|        7 |        0x0007  | TIMER1_COMPB_vect | Alias - provided by ATTinyCore      |
|        8 |        0x0008  | TIM1_OVF_vect     | Timer/Counter1 Overflow             |
|        8 |        0x0008  | TIMER1_OVF_vect   | Alias - provided by ATTinyCore      |
|        9 |        0x0009  | TIM0_COMPA_vect   | Timer/Counter0 Compare Match A      |
|        9 |        0x0009  | TIMER0_COMPA_vect | Alias - provided by ATTinyCore      |
|       10 |        0x000A  | TIM0_COMPB_vect   | Timer/Counter0 Compare Match B      |
|       10 |        0x000A  | TIMER0_COMPB_vect | Alias - provided by ATTinyCore      |
|       11 |        0x000B  | TIM0_OVF_vect     | Timer/Counter0 Overflow             |
|       11 |        0x000B  | TIMER0_OVF_vect   | Alias - provided by ATTinyCore      |
|       12 |        0x000C  | ANA_COMP_vect     | Analog Comparator                   |
|       13 |        0x000D  | ADC_vect          | ADC Conversion Complete             |
|       14 |        0x000E  | EE_RDY_vect       | EEPROM Ready                        |
|       16 |        0x000F  | USI_STR_vect      | USI START                           |
|       17 |        0x0010  | USI_OVF_vect      | USI Overflow                        |

## 84 vs 84a - you said "almost" fully interchangible?
Okay, there is one difference I'm aware of that makes them distinct: The older 861 design has the old, bifurcated calibration curve for the internal oscillator, that is, the speed jumps backwards as you increase the `OSCCAL` register from 127 to 128. The "bifurcated" oscillators are also generally less accurate and less stable than ones like the one in the ATtiny84A. This is most relevant with Micronucleus using the internal oscillator. Since the reliability of USB on VUSB-using parts depends on accuracy of the clock (since USB is picky about timing) the A-version should work better. No testing was conducted with non-A parts.
