### ATtiny 87/167
![x7 pin mapping](Pinout_x7.jpg "Arduino Pin Mapping for ATtiny x7-family")

| Specification           |    ATtiny167   |    ATtiny167   |    ATtiny167   |    ATtiny87    |    ATtiny87    |    ATtiny87    |
|-------------------------|----------------|----------------|----------------|----------------|----------------|----------------|
| Bootloader (if any)     |                |       Optiboot |  Micronucleus  |                |       Optiboot |  Micronucleus  |
| Uploading uses          |   ISP/SPI pins | Serial Adapter | USB (directly) |   ISP/SPI pins | Serial Adapter | USB (directly) |
| Flash available user    |    16384 bytes |    15744 bytes |    14842 bytes |     8192 bytes |     7552 bytes |     6554 bytes |
| RAM                     |      512 bytes |      512 bytes |      512 bytes |      512 bytes |      512 bytes |      512 bytes |
| EEPROM                  |      512 bytes |      512 bytes |      512 bytes |      512 bytes |      512 bytes |      512 bytes |
| GPIO Pins               |             15 |             15 |          ** 13 |             15 |             15 |          ** 13 |
| ADC Channels            |             11 |             11 |             11 |             11 |             11 |             11 |
| PWM Channels            | 1 fixed 2 flex | 1 fixed 2 flex | 1 fixed 2 flex | 1 fixed 2 flex | 1 fixed 2 flex | 1 fixed 2 flex |
| Differential ADC        |    8x/20x gain |    8x/20x gain |    8x/20x gain |    8x/20x gain |    8x/20x gain |    8x/20x gain |
| Interfaces              |       LIN/UART |       LIN/UART | vUSB, LIN/UART |       LIN/UART |       LIN/UART | vUSB, LIN/UART |
| Interfaces (cont'd)     |       USI, SPI |       USI, SPI |       USI, SPI |       USI, SPI |       USI, SPI |       USI, SPI |
| Clocking Options:       |         in MHz |         in MHz |         in MHz |         in MHz |         in MHz |         in MHz |
| Int. Oscillator         |     8, 4, 2, 1 |     8, 4, 2, 1 |  Not supported |     8, 4, 2, 1 |     8, 4, 2, 1 |  Not supported |
| Int. WDT Oscillator     |        128 kHz |  Not supported |  Not supported |        128 kHz |  Not supported |  Not supported |
| Internal, with tuning   |          8, 12 |          8, 12 |  Not supported |          8, 12 |          8, 12 |  Not supported |
| External Crystal        |   All Standard |   All Standard | **16**,8,4,2,1 |   All Standard |   All Standard | **16**,8,4,2,1 |
| External Clock          |   All Standard |   All Standard |  Not supported |   All Standard |   All Standard |  Not supported |
| Default Pin Mapping     |       Standard |       Standard |      Digispark |       Standard |       Standard |      Digispark |
| LED_BUILTIN             | PA6 PB1 or PB0 | PA6 PB1 or PB0 | PB1 PA6 or PB0 | PA6 PB1 or PB0 | PA6 PB1 or PB0 | PB1 PA6 or PB0 |
| Bootloader LED          |            n/a |            PA6 |     PB1 or PA6 |            n/a |            PA6 |     PB1 or PA6 |

`*` - the bootloader will always use either PA6 or PB1 unless you build your own binaries. The legacy pinout that it was paired with is absolutely godawful, and nobody should ever use it!
`**` - The bootloader will always run at this speed. The sketch may be set to run at a lower speed by prescaling this.

## Programming
Any of these parts can be programmed by use of any supported ISP programmer. It is recommended to use Arduino 1.8.13 or later; earlier versions will show all programmers, instead of just the ones that will work with this core.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny87 and 167, operating on the hardware UART/LIN port at 115200 baud for 12 or 16 MHz clock speed, and 57600 when running at 8 MHz. In order to work on the x7 series, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the WDT vector gets pointed to the start of the application.  This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

#### Supported Entry Conditions:
* Reset only, upload must start w/in 1 second (for use with autoreset)
* Reset and Power On, with 8-secind wait (for use without autoreset)

### Micronucleus VUSB Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny167, allowing sketches to be uploaded directly over USB. The board definition runs at 16 MHz via external crystal (if USB is not required, it can be prescaled as listed in the table for low power applications). See the document on [Micronucleus usage](Ref_Micronucleus.md) for more information. D- is on PIN_PB3, D+ is on pin PIN_PB6.

Two versions of the bootloader are provided, one for use with Digispark Pro boards with the LED on PB1, and one for use on our Azduino boards with the LED on PA6 (this is easier to route on the 0.425" wide PCB, which is sized to plug into a DIP-24 socket - 6 pin positions near the middle are unused, 2 forced by the width of the chip, and 2 by the fact that you don't *want* the crystal pins broken out).

## Features

### Alternate pinout options
There was an old ATtiny x7 core with a different and more awkward pinout. This is supported, for compatibility purposes, via the "Legacy" pinmapping option. It should be used only if you are trying to use an old sketch that was written for that pin mapping. The Digispark Pro boards have pins labeled with yet another pin mapping. All pin mappings can be chosen for both Digispark/VUSB and non-VUSB boards, for compatibility. This is selected from the Tools -> Pin Mapping submenu. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for `PINMAPPING_OLD`, `PINMAPPING_NEW`, or `PINMAPPING_DIGI` macro (eg, `#ifdef PINMAPPING_OLD` - I would recommend checking if the compatible one is not defined and immediately #error'ing in that case). Alternately, also as of 1.4.0, with any pin mapping selected, you can always refer to pins by their port and number within that port, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7, which is pin 7 in the clockwise mapping and pin 3 in the counterclockwise mapping (don't use PIN_xn or Pxn) - in this case the pin mapping won't matter.

Example of a "guard" against wrong pin mapping:
```
#ifndef PINMAPPING_NEW
#error "Sketch was written for new pinmapping!"
#endif
```
The pin mapping for the Digispark Pro is very, very strange. Note that on the An constants for analogRead() n is the number of the digital pin, not the the ADC channel for that mapping.

The legacy pin mapping is even stranger - and also really really bad. It is as if someone tried to make a pin mapping that precluded any sort of simplification of the math, anywhere - and they were largely successful. It's the only place where I had to use a lookup table to convert between analog and digital pin numbers....

LED_BUILTIN is on PB0 on the legacy option, PB1 on digispark, and PA6 on the new pinout.

### Flexible PWM support
The two channels of Timer1 can each output on one or more of 4 pins, albeit with the same duty cycle. The OCR1Ax and OCR1Bx pins each share the channel. All of those pins can be used for PWM. If you do `analogWrite(PIN_PB0,64);`, you get 25% dutycycle, if you then do `analogWrite(PIN_PB2,128);` (these are OCR1AU and OCR1AW, respectively) both of the pins will be outputting 50% dutycycle after the second command. To stop the PWM output, call digitalWrite() or analogWrite() with 0 or 255 on the pin.

### Tone Support
Tone() uses Timer1. For best results, use a pin on port B - those will use the hardware output compare rather than an interrupt to generate the tone. Using tone() will disable all PWM pins except PIN_PA2.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. This is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all. There is no need for libraries like TinyWire or USIWire or that kind of thing.

### SPI Support
There is a full hardware SPI port and the normal SPI library can be used.

### UART (Serial) with LIN support
There is one full hardware Serial port with LIN support, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. The ATtiny x7-family has LIN support, unique among the ATtiny linup; LIN (Local Interconnect Network) is frequently used in automotive and industrial applications. One consequence of this additional feature is that the baud rate generator is able to match baud rates much more closely than a "standard" UART module.

## ADC Features
The ATtiny x7 series features a mid-range ADC - it has the second reference voltage, a built-in voltage divider on AVcc (which is nominally tied to Vcc, optionally with measures taken to reduce noise; nobody takes such measures in Arduino-land, and it generally appears somewhat rare in the wild - note that per datasheet AVcc must be within 0.3V of AVcc), and a modest selection of differential pairs. It also has the rare feature of being able to *OUTPUT* it's internal analog reference voltages.

| Reference Option    | Reference Voltage           | Uses AREF Pin                 |
|---------------------|-----------------------------|-------------------------------|
| `DEFAULT`           | Vcc                         | No, pin available             |
| `EXTERNAL`          | Voltage applied to AREF pin | Yes, ext. voltage             |
| `INTERNAL1V1`       | Internal 1.1V reference     | No, pin available             |
| `INTERNAL2V56`      | Internal 2.56V reference    | No, pin available             |
| `INTERNAL1V1_XREF`  | Internal 1.1V reference     | Yes, reference output on AREF |
| `INTERNAL2V56_XREF` | Internal 2.56V reference    | Yes, reference output on AREF |
| `INTERNAL`          | Same as `INTERNAL1V1`       | No, pin available             |

### Internal Sources
| Voltage Source    | Description                            |
|-------------------|----------------------------------------|
| `ADC_INTERNAL1V1` | Reads the INTERNAL1V1 reference        |
| `ADC_GROUND`      | Reads ground - for offset correction?  |
| `ADC_AVCCDIV4`    | Reads AVCC divided by 4                |
| `ADC_TEMPERATURE` | Reads internal temperature sensor      |

### Differential ADC channels
Though it's a far cry from what some of the classic tinyAVR parts have, the x7-series does offer a modest selection of ADC channels with 8x and 20x selectable gain.  ATTinyCore (v2.0.0+) allows you to read from them with `analogRead()` by using the channel names shown below. If it is required to know the numeric values of the channels, they are shown below as well. If you must work with channel numbers, instead of a names, when passing them to `analogRead()`, use the `ADC_CH()` macro (ex: `analogRead(ADC_CH(0x11))` to read ADC0 and ADC1 at 20x gain, equivalent to `analogRead(DIFF_A0_A1_20X)`), otherwise they will be interpreted as a (likely non-existent) digital pin.

On the Digispark pinout, the analog numbers they used were the same as the digital pin numbers. So to make things logically consistent (would it make sense that to measure difference between A6 and A7 you had to use `DIFF_A0_A1_8X`? Of course not! So we number the differential channels accordingly. The result is almost as horrendous as if we hadn't done so and stubbornly insisted on using ADC channel numbers in the differential names. It sucks either way. For this reason, it is recommended that the default pin mapping be used if you are using the differential ADC.

|  Positive  |   Negative  | 8X Gain |20X Gain|  Name (8x Gain)  |  Name (20x Gain)  |    Digispark 8x   |   Digispark 20x    |
|------------|-------------|---------|--------|------------------|-------------------|-------------------|--------------------|
| ADC0 (PA0) |  ADC1 (PA1) |   0x10  |  0x11  |  `DIFF_A0_A1_8X` |  `DIFF_A0_A1_20X` |   `DIFF_A6_A7_8X` |   `DIFF_A6_A7_20X` |
| ADC1 (PA1) |  ADC2 (PA2) |   0x12  |  0x13  |  `DIFF_A1_A2_8X` |  `DIFF_A1_A2_20X` |  `DIFF_A7_A13_8X` |  `DIFF_A7_A13_20X` |
| ADC2 (PA2) |  ADC3 (PA3) |   0x14  |  0x15  |  `DIFF_A2_A3_8X` |  `DIFF_A2_A3_20X` |  `DIFF_A13_A9_8X` |  `DIFF_A13_A9_20X` |
| ADC4 (PA4) |  ADC5 (PA5) |   0x16  |  0x17  |  `DIFF_A4_A5_8X` |  `DIFF_A4_A5_20X` | `DIFF_A10_A11_8X` | `DIFF_A10_A11_20X` |
| ADC5 (PA5) |  ADC6 (PA6) |   0x18  |  0x19  |  `DIFF_A5_A6_8X` |  `DIFF_A5_A6_20X` | `DIFF_A11_A12_8X` | `DIFF_A11_A12_20X` |
| ADC6 (PA6) |  ADC7 (PA7) |   0x1A  |  0x1B  |  `DIFF_A6_A7_8X` |  `DIFF_A6_A7_20X` |  `DIFF_A12_A5_8X` |  `DIFF_A12_A5_20X` |
| ADC8 (PB5) |  ADC9 (PB6) |   0x1C  |  0x1D  |  `DIFF_A8_A9_8X` |  `DIFF_A8_A9_20X` |   `DIFF_A8_A3_8X` |   `DIFF_A8_A3_20X` |
| ADC9 (PB6) | ADC10 (PB7) |   0x1E  |  0x1F  | `DIFF_A9_A10_8X` | `DIFF_A9_A10_20X` |  `DIFF_A3_A15_8X` |  `DIFF_A3_A15_20X` |

### Temperature Measurement
To measure the temperature, select the 1.1v internal voltage reference, and analogRead(ADC_TEMPERATURE); This value changes by approximately 1 LSB per degree C. This requires calibration on a per-chip basis to translate to an actual temperature, as the offset is not tightly controlled - take the measurement at a known temperature (we recommend 25C - though it should be close to the nominal operating temperature, since the closer to the single point calibration temperature the measured temperature is, the more accurate that calibration will be without doing a more complicated two-point calibration (which would also give an approximate value for the slope)) and store it in EEPROM (make sure that `EESAVE` fuse is set first, otherwise it will be lost when new code is uploaded via ISP) if programming via ISP, or at the end of the flash if programming via a bootloader (same area where oscillator tuning values are stored). See the section below for the recommended locations for these.

Note that while the text of this section of the datasheet is essentially copied verbatim between most of the classic tinyAVR parts, the ATtiny87/167 datasheet has a different set of "typical values"... and these are inconsistent with what the text is saying by a huge margin. However, it did not escape my notice that the same table also contains a typo in the notation (0c01B8 instead of 0x01B8), and that the inconsistencies are suspiciously close to what might happen if someone attempted to convert from decimal to hex recognizing that the third digit is 256s but not that second digit is 16's (230 middle value is 300 on other sheets, 0x144 here. 300 - 256 = 44, so a 1 in the 256's and 44 in the tens and ones, high value 370 on other sheets, 0x1B8 here. Very close to 370 - 256 = 114 11 is B in hex so 0x1B4, though I can't account for the extra 4. The low value is harder to explain via math errors but if my bad-math theory above is correct, I have no confidence in his ability to add and subtract correctly either).

### Tuning Constant Locations
These are the recommended locations to store tuning constants. In the case of OSCCAL, they are what are checked during startup when a tuned configuration is selected. They are not otherwiseused by the core.

ISP programming: Make sure to have EESAVE fuse set, stored in EEPROM

Optiboot used: Saved between end of bootloader and end of flash.

| Tuning Constant        | Location EEPROM | Location Flash |
|------------------------|-----------------|----------------|
| Temperature Offset     | E2END - 3       | FLASHEND - 5   |
| Temperature Slope      | E2END - 2       | FLASHEND - 4   |
| Tuned OSCCAL 12 MHz    | E2END - 1       | FLASHEND - 3   |
| Tuned OSCCAL 8 MHz     | E2END           | FLASHEND - 2   |
| Bootloader Signature 1 | Not Used        | FLASHEND - 1   |
| Bootloader Signature 2 | Not Used        | FLASHEND       |

Mironucleus used: Micronucleus boards are locked to the crystal, no oscillator calibration is possible.

| Tuning Constant        | Location Flash |
|------------------------|----------------|
| Temperature Offset     | FLASHEND - 1   |
| Temperature Slope      | FLASHEND       |

### Purchasing ATtiny167 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny167 boards through my Tindie store - your purchases support the continued development of this core. Unfortunately this design is currently out of stock; a revised version is in the works.
* Micronucleus boards are readily available all over the internet, fairly cheaply, in several models. Search for things like "Digispark Pro", "Digispark ATtiny167", "ATtiny167 USB" and so on.

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x7-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" - however, when that interrupt is triggered, the device will (at best) immediately reset - and not cleanly either. The catastrophic nature of the failure often makes debugging challenging. Vector addresses shown are "word addressed". The `#` column is the number you are shown in the event of a duplicate vector error, among other things.
Addresses are for 87 and 167 - the 167, having 16k of flash, has 4-byte vectors instead of 2-byte vectors.

|  # |87 addr |167 addr| Vector Name         | Interrupt Definition                |
|----|--------|--------|---------------------|-------------------------------------|
|  0 | 0x0000 | 0x0000 | `RESET_vect`        | External Pin, Power-on Reset        |
|  1 | 0x0001 | 0x0002 | `INT0_vect`         | External Interrupt Request 0        |
|  2 | 0x0002 | 0x0004 | `INT1_vect`         | External Interrupt Request 1        |
|  3 | 0x0003 | 0x0006 | `PCINT0_vect`       | Pin Change Interrupt (PORTA)        |
|  4 | 0x0004 | 0x0008 | `PCINT1_vect`       | Pin Change Interrupt (PORTB)        |
|  5 | 0x0005 | 0x000A | `WDT_vect`          | Watchdog Time-out (Interrupt Mode)  |
|  6 | 0x0006 | 0x000C | `TIMER1_CAPT_vect`  | Timer/Counter1 Capture              |
|  7 | 0x0007 | 0x000E | `TIMER1_COMPA_vect` | Timer/Counter1 Compare Match        |
|  8 | 0x0008 | 0x0010 | `TIMER1_COMPB_vect` | Timer/Coutner1 Compare Match        |
|  9 | 0x0009 | 0x0012 | `TIMER1_OVF_vect`   | Timer/Counter1 Overflow             |
| 10 | 0x000A | 0x0014 | `TIMER0_COMPA_vect` | Timer/Counter0 Compare Match        |
| 11 | 0x000B | 0x0016 | `TIMER0_OVF_vect`   | Timer/Counter0 Overflow             |
| 12 | 0x000C | 0x0018 | `LIN_TC_vect`       | LIN/UART Transfer Complete          |
| 13 | 0x000D | 0x001A | `LIN_ERR_vect`      | LIN/UART Error                      |
| 14 | 0x000E | 0x001C | `SPI_STC_vect`      | SPI Serial Transfer Complete        |
| 15 | 0x000F | 0x001E | `ADC_READY_vect`    | Conversion Complete                 |
| 16 | 0x0010 | 0x0020 | `EE_READY_vect`     | EEPROM Ready                        |
| 17 | 0x0011 | 0x0022 | `ANALOG_COMP_vect`  | Analog Comparator                   |
| 18 | 0x0012 | 0x0024 | `USI_START_vect`    | USI Start Condition                 |
| 19 | 0x0013 | 0x0026 | `USI_OVF_vect`      | USI Counter Overflow                |
