# ATtiny 87/167
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
| External Crystal        |   All Standard |   All Standard | **16** `**` ,8,4,2,1 |   All Standard |   All Standard | **16** `**` ,8,4,2,1 |
| External Clock          |   All Standard |   All Standard |  Not supported |   All Standard |   All Standard |  Not supported |
| Default Pin Mapping     |       Standard |       Standard |      Digispark |       Standard |       Standard |      Digispark |
| LED_BUILTIN ***         | PA6 PB1 or PB0 | PA6 PB1 or PB0 | PB1 PA6 or PB0 | PA6 PB1 or PB0 | PA6 PB1 or PB0 | PB1 PA6 or PB0 |
| Bootloader LED *        |            n/a |            PA6 |     PB1 or PA6 |            n/a |            PA6 |     PB1 or PA6 |

`*` - the bootloader will always use either PA6 or PB1 unless you build your own binaries. The legacy pinout that it was paired with is absolutely godawful, and nobody should ever use it!
`**` - The bootloader will always run at this speed. The sketch may be set to run at a lower speed by prescaling this.
`***` - The pin used for the builtin LED will depend on the selected pin mapping.

## Programming
Any of these parts can be programmed by use of any ISP programmer. 4k and 8k parts can be programmed over the software serial port using Optiboot, and 8k parts can be programmed via Micronucleus. Be sure to read the section of the main readme on the ISP programmers and IDE versions. 1.8.13 is recommended for best results.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny87 and 167, operating on the hardware UART/LIN port at 115200 baud for 12 or 16 MHz clock speed, and 57600 when running at 8 MHz. In order to work on the x7 series, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the WDT vector gets pointed to the start of the application.  This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

#### Supported Entry Conditions
* Reset only, upload must start w/in 1 second (for use with autoreset)
* Reset and Power On, with 8-secind wait (for use without autoreset)

### Micronucleus VUSB Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny167, allowing sketches to be uploaded directly over USB. The board definition runs at 16 MHz via external crystal (if USB is not required, it can be prescaled as listed in the table for low power applications). See the document on [Micronucleus usage](Ref_Micronucleus.md) for more information. D- is on PIN_PB3, D+ is on pin PIN_PB6.

Two versions of the bootloader are provided, one for use with Digispark Pro boards with the LED on PB1, and one for use on our Azduino boards with the LED on PA6 (this is easier to route on the 0.425" wide PCB, which is sized to plug into a DIP-24 socket - 6 pin positions near the middle are unused, 2 forced by the width of the chip, and 2 by the fact that you don't *want* the crystal pins broken out).

## Features

### Alternate pinout options
There was an old ATtiny x7 core with a different and more awkward pinout. This is supported, for compatibility purposes, via the "Legacy" pinmapping option. It should be used only if you are trying to use an old sketch that was written for that pin mapping. The Digispark Pro boards have pins labeled with yet another pin mapping. All pin mappings can be chosen for both Digispark/VUSB and non-VUSB boards, for compatibility. This is selected from the Tools -> Pin Mapping submenu. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for `PINMAPPING_OLD`, `PINMAPPING_NEW`, or `PINMAPPING_DIGI` macro (eg, `#ifdef PINMAPPING_OLD` - I would recommend checking if the compatible one is not defined and immediately #error'ing in that case). **Alternately, also as of 2.0.0, with any pin mapping selected, you can always refer to pins by their port and number within that port**, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7 (don't use PIN_xn or Pxn, those mean different things that are rarely useful) - in this case any pinmapping will wor - however the size of the compiled binary and the speed of some internal options will still vary based on it.

Example of a "guard" against wrong pin mapping:
```c
#ifndef PINMAPPING_NEW
  #error "Sketch was written for new pin mapping!"
#endif
```
The pin mapping for the Digispark Pro is very, very strange. Note that on the An constants for analogRead() n is the number of the digital pin, not the the ADC channel for that mapping.

The legacy pin mapping is even stranger - and also really really bad. It is as if someone tried to make a pin mapping that precluded any sort of simplification of the math, anywhere - and they were largely successful. It's the only place where I had to use a lookup table to convert between analog and digital pin numbers.

LED_BUILTIN is on PB0 on the legacy option (that was a dumb choice), PB1 on digispark (this is equally dumb) - those two choices are both used by the USI! On the new pinout, we use PA6 for LED_BUILTIN. It is probably the least useful pin on the chip (and being the SS pin for SPI, one that you need to keep as an output if using SPI anyway, unless you're making an SPI slave but I am aware of nobody having done that in an Arduin context)

### Flexible PWM support
The two channels of Timer1 can each output on one or more of 4 pins, but each channel can only output one duty cycle. Thus, on the x67, the OCR1Ax and OCR1Bx pins, which are the even and odd pins in PORTB respectively) each share a single channel, while the lone compare output on PA3 from timer0 is independent. So while all 8 of those pins can be used for PWM, you can only have a single duty cycle on the odd pins and a different one on the even ones. If you do `analogWrite(PIN_PB0,64);`, you get 25% dutycycle on PB0, if you then do `analogWrite(PIN_PB2,128);` (these are OCR1AU and OCR1AW, respectively) both of the pins will be outputting 50% dutycycle after the second command. To stop the PWM output, call digitalWrite() or analogWrite() with 0 or 255 on the pin you want to turn off. However if you did `analogWrite(PIN_PB0,128); analogWrite(PIN_PB1,128);` (OCR1AU and OCR1BU), you would get 25% on PB0 and 50% on PB1.

#### PWM frequency
TC0 is always run in Fast PWM mode: We use TC0 for millis, and phase correct mode can't be used on the millis timer - you need to read the count to get micros, but that doesn't tell you the time in phase correct mode because you don't know if it's upcounting or downcounting in phase correct mode.

| F_CPU  | F_PWM<sub>TC0</sub> | F_PWM<sub>TC1</sub>   | Notes                        |
|--------|---------------------|-----------------------|------------------------------|
| 1  MHz | 1/8/256=     488 Hz |  1/8/256=      488 Hz |                              |
| 2  MHz | 2/8/256=     977 Hz |  2/8/256=      977 Hz |                              |
| <4 MHz | x/8/256= 488 * x Hz |  x/8/512=  244 * x Hz | Phase correct TC1            |
| 4  MHz | 4/32/256=    488 Hz |  4/8/512=      977 Hz | /32 prescale on TC0 x7 only. Phase correct TC1 |
| <8 MHz | x/32/256=122 * x Hz |  x/8/512=  244 * x Hz | /32 prescale on TC0 x7 only. Between 4 and 8 MHz, the target range is elusive ||
| 8  MHz | 8/32/256=    977 Hz |  8/64/256=     488 Hz | /32 prescale on TC0 x7 only. |
| >8 MHz | x/64/256= 61 * x Hz |  x/64/256=  61 * x Hz |                              |
| 12 MHz | 12/64/256=   732 Hz | 12/64/256=     732 Hz |                              |
| 16 MHz | 16/64/256=   977 Hz | 16/64/256=     977 Hz |                              |
|>16 MHz | x/128/256=30 * x Hz |  x/64/512=  30 * x Hz | Phase correct TC1            |
| 20 MHz | 20/128/255=  610 Hz | 20/64/512=     610 Hz | Phase correct TC1            |

Where the /32 or /128 prescaler, not available on most parts, is used, it sigificantly improves the output frequency in the most desirable range for typical applications - unfortunately timer0 has only a single output (it's not the standard TC0 - it has much in common the the ATmega TC2 async, potentially externally-clocked timer.

Where speeds above or below a certain speed are specified, it's implied that the other end of the range is the next marked value. So >16 in that table is for 16-20 MHz clocks. The formula is given as a constant times x where x is expressed as MHz (the division above gets the time in megahertz - in the interest of readability I did not include the MHz to Hz conversion - I'm sure you all know how to multiply by a million)

Phase correct PWM counts up to 255, flipping the pin off (er... that may not be the best choice of words, I wasn't thinking about the colloquial sense...) as it passes the compare value, updates it's double-buffered registers at TOP, then it counts down to 0, flipping the pin back as is passes the compare value. This is considered preferable for motor control applications, though the "Phase and Frequency Correct" mode is better if the period is ever adjusted by a large amount at a time, because it updates the doublebuffered registers at BOTTOM, and thus produces a less problematic glitch in the duty cycle, but doesn't have any modes that don't require setting ICR1 too.

For more information see the [Changing PWM Frequency](Ref_ChangePWMFreq.md) reference.

### Tone Support
Tone() uses Timer1. For best results, use a pin on port B - those will use the hardware output compare rather than an interrupt to generate the tone. Using tone() will disable all PWM pins except PIN_PA2.

### Servo Support
The standard Servo library is hardcoded to work on specific parts only, we include a builtin Servo library that supports the Tiny x7 series. As always, while a software serial port is receiving or transmitting, the servo signal will glitch. See [the Servo/Servo_ATTinyCore library](../libraries/Servo/README.md) for more details. This will disable all PWM pins except PIN_PA2, and is incompatible with tone().

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. This is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all. We only support use of the builtin universal Wire.h library. If you try to use other libraries and encounter issues, please contact the author or maintainer of that library - there are too many of these poorly written libraries for us to provide technical support for. Unlike most parts with a USI, this also has a proper SPI port!

### SPI Support
There is a full hardware SPI port and the normal SPI library can be used. Third party libraries that  for tinyAVR parts and which try to use things like USIWire et. al. should not be used - there are only 6 tinyAVRs with a full SPI peripheral,

### UART (Serial) with LIN support
There is one full hardware Serial port with LIN support, named Serial. It works the same as Serial on any normal Arduino - it is not a software implementation. The ATtiny x7-family has LIN support, unique among the classic ATtiny linup; LIN (Local Interconnect Network) is frequently used in automotive and industrial applications. One consequence of this additional feature is that the baud rate generator is able to match baud rates much more closely than a "standard" UART module.

## ADC Features
The ATtiny x7 series features a mid-range ADC - it has the second reference voltage, a built-in voltage divider on AVcc (which is nominally tied to Vcc, optionally with measures taken to reduce noise; nobody takes such measures in Arduino-land, and it generally appears somewhat rare in the wild - note that per datasheet AVcc must be within 0.3V of AVcc), and a modest selection of differential pairs. It also has the rare feature of being able to *OUTPUT* it's internal analog reference voltages with the expectation that other parts might be using them. It is not clear if the pin can be used for other purposes when an internal reference is used. It helpfully states "The internal voltage reference options may not be used if an external voltage is being applied to the AREF pin" - now, does that mean I can still use it as an output without concern? Or does that mean that if an external reference is used

| Reference Option    | Reference Voltage           | Uses AREF Pin                 |
|---------------------|-----------------------------|-------------------------------|
| `DEFAULT`           | Vcc                         | No, pin available             |
| `EXTERNAL`          | Voltage applied to AREF pin | Yes, ext. voltage             |
| `INTERNAL1V1`       | Internal 1.1V reference     | Unclear, looks like yes?      |
| `INTERNAL2V56`      | Internal 2.56V reference    | Unclear, looks like yes?      |
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

#### Bipolar Input Mode (BIN)
Normally the x7 operates in unipolar mode, that is from Neg = Pos = (reading 0), to (pos-neg) < VREF). But maybe you want a bipolar measuremebt (-512 to 511) so you don't need to know which value is higher, this has become the norm, and the BIN option was dropped See the part specific docs. BIN set and cleared normally
uint8_t ctrlb = ADC0.CTRLB
ctrlb = AD

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
I (Spence Konde / Dr. Azzy) sell ATtiny167 boards through my Tindie store - your purchases support the continued development of this core. A new version is now available. In order to fit in the same form factor as my other tinyAVR breakout boards, these use the TSSOP-20 package version insteaad of the bulky SOIC-20.
* Azduino Tiny167 Pro - bare board - pending verification of functionality.
* Azduino Tiny167 Pro - pending verification and assembly.
* Ultramini - fits a DIP-24 socket - pending verification of functionality.

## Package variants
The 87 and 167 are available in three package variations. Additionally, the 167 only can be had in a fourth package.
* SOIC-20 (wide) - bigger than the side of a house, but easy to hand solder
* TSSOP-20 - Slightly more demanding to solder. While it is hard to imagine being able to read this text and miss a bridge between adjacent pins on a SOIC-20, the same cannot be said for a SSOP-20 - depending on your eyesight, you may need magnification or more attention to lighting in order to spot bridges visually
* VQFN32 - with 12 unused pins - Atmel seemed to REALLY like this package - a lot of 20-pin tinyAVR parts got this as their QFN instead of a proper QFN20-type. It's an annoying package, though - it's very fine pitch, large for a QFN (5mm x 5mm), and the unused pins don't appear to have been arranged with consideration of the layout. They're in the same order as the pins up and down the two sides of the SOIC/SSOP parts (probably a technical constraint I've never seen a chip with what was believed to have the same die, *not* have the same pin order, so I think bond wires have to make straight lines that don't cross each other from the die to the pin), but the decisons for where those dummy pins would go appears to have been based only on their convenience.

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x7-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" (for example, if you misspell a vector name) - however, when that interrupt is triggered, the device will (at best) immediately reset (and not clearly - I refer to this as a "dirty reset") The catastrophic nature of the failure often makes debugging challenging.

Note: The shown addresses below are "byte addressed" as that has proven more readily recognizable. The vector number is the number you are shown in the event of a duplicate vector error, as well as the interrupt priority (lower number = higher priority), if, for example, several interrupt flags are set while interrupts are disabled, the lowest numbered one would run first. Notice that INT0 is (as always) the highest priority interrupt.

Addresses for 87 and 167 are different - the 167, having 16k of flash, has 4-byte vectors, because an rjmp instruction can only reach the entire flash on parts with not more than 8k of flash.

**WARNING** The datasheet here is WRONG. It innacuratesly lists the vectors as being the same size on the 87 and 167. They are not, these were done correctly - the 8k part has 1 word vectors and the 16k part has 2 word vectors.

|  # |87 addr |167 addr| Vector Name         | Interrupt Definition                |
|----|--------|--------|---------------------|-------------------------------------|
|  0 | 0x0000 | 0x0000 | `RESET_vect`        | Not an interrupt - this is a jump to the start of your code.  |
|  1 | 0x0002 | 0x0004 | `INT0_vect`         | External Interrupt Request 0        |
|  2 | 0x0004 | 0x0008 | `INT1_vect`         | External Interrupt Request 1        |
|  3 | 0x0006 | 0x000C | `PCINT0_vect`       | Pin Change Interrupt (PORTA)        |
|  4 | 0x0008 | 0x0010 | `PCINT1_vect`       | Pin Change Interrupt (PORTB)        |
|  5 | 0x000A | 0x0014 | `WDT_vect`          | Watchdog Time-out (Interrupt Mode)  |
|  6 | 0x000C | 0x0018 | `TIMER1_CAPT_vect`  | Timer/Counter1 Capture              |
|  7 | 0x000E | 0x001C | `TIMER1_COMPA_vect` | Timer/Counter1 Compare Match        |
|  8 | 0x0010 | 0x0020 | `TIMER1_COMPB_vect` | Timer/Coutner1 Compare Match        |
|  9 | 0x0012 | 0x0024 | `TIMER1_OVF_vect`   | Timer/Counter1 Overflow             |
| 10 | 0x0014 | 0x0028 | `TIMER0_COMPA_vect` | Timer/Counter0 Compare Match        |
| 11 | 0x0016 | 0x002C | `TIMER0_OVF_vect`   | Timer/Counter0 Overflow             |
| 12 | 0x0018 | 0x0030 | `LIN_TC_vect`       | LIN/UART Transfer Complete          |
| 13 | 0x001A | 0x0034 | `LIN_ERR_vect`      | LIN/UART Error                      |
| 14 | 0x001C | 0x0038 | `SPI_STC_vect`      | SPI Serial Transfer Complete        |
| 15 | 0x001E | 0x003C | `ADC_READY_vect`    | Conversion Complete                 |
| 16 | 0x0020 | 0x0040 | `EE_READY_vect`     | EEPROM Ready                        |
| 17 | 0x0022 | 0x0044 | `ANALOG_COMP_vect`  | Analog Comparator                   |
| 18 | 0x0024 | 0x0048 | `USI_START_vect`    | USI Start Condition                 |
| 19 | 0x0026 | 0x004C | `USI_OVF_vect`      | USI Counter Overflow                |
