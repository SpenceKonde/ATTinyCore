# ATtiny 441/841
![x41 Pin Mapping](Pinout_x41.jpg "Arduino Pin Mapping for ATtiny x41")

Specification         |    ATtiny841   |      ATtiny841 |    ATtiny841   |     ATtiny441  |      ATtiny441 |
----------------------|----------------|----------------|----------------|----------------|----------------|
Bootloader (if any)   |                |       Optiboot |  Micronucleus  |                |       Optiboot |
Uploading uses        |   ISP/SPI pins | Serial Adapter | USB (directly) |   ISP/SPI pins | Serial Adapter |
Flash available user  |     8192 bytes |     7552 bytes |     6586 bytes |     4096 bytes |     3456 bytes |
RAM                   |      512 bytes |      512 bytes |      512 bytes |      256 bytes |      256 bytes |
EEPROM                |      512 bytes |      512 bytes |      512 bytes |      256 bytes |      256 bytes |
GPIO Pins             |     11 + RESET |     11 + RESET |     11 + RESET |     11 + RESET |     11 + RESET |
ADC Channels          |  12 (incl RST) |  12 (incl RST) |  12 (incl RST) |  12 (incl RST) |  12 (incl RST) |
Differential ADC      |  Yes, v. fancy |  Yes, v. fancy |  Yes, v. fancy |  Yes, v. fancy |  Yes, v. fancy |
PWM Channels          |              6 |      2 (9, 10) |      2 (9, 10) |      2 (9, 10) |      2 (9, 10) |
Interfaces            |  2x USART, SPI |       SPI, I2C | vUSB, SPI, I2C |       SPI, I2C |       SPI, I2C |
Interfaces            |      I2C slave |      I2C slave |      I2C slave |      I2C slave |      I2C slave |
Clocking Options:     |         in MHz |         in MHz |         in MHz |         in MHz |         in MHz |
Int. Oscillator       |     8, 4, 2, 1 |     8, 4, 2, 1 |  Not supported |     8, 4, 2, 1 |     8, 4, 2, 1 |
Internal, with tuning | 16, 12, 8@5/3v3| 16, 12, 8@5/3v3| 16, 12, 8@5/3v3| 16, 12, 8@5/3v3| 16, 12, 8@5/3v3|
External Crystal      |   All Standard |   All Standard |  Not supported |   All Standard |   All Standard |
External Clock        |   All Standard |   All Standard |  Not supported |   All Standard |   All Standard |
Int. ULP Oscillator   |   32, 64, 128, |  Not supported |  Not supported |   32, 64, 128, |  Not supported |
Int. ULP Osc. Cont.   | 256 or 512 kHz |                |                | 256 or 512 kHz |                |
Default Pin Mapping   |      Clockwise |      Clockwise |      Clockwise |      Clockwise |      Clockwise |
LED_BUILTIN           |        PIN_PB2 |


Two pinout options are available, clockwise and counterclockwise, see below for more information. Be sure that the one you have selected is the one that you intend!

These parts are available in a SOIC-14, 4x4mm QFN-20, or 3x3mm VQFN-20. No, I don't know why they didn't use a 16-pin QFN either.

The ATtiny x41-family is a more advanced version of the ATtiny x4-family. It is pin compatible, though available only in surface mount packages, and offers an incredible array of peripherals, whilst costing only slightly more than an ATtiny 84. Tests have shown that despite manufacturer spec'ed max speed of 16 MHz, they typically work without issue at 20 MHz @ 5v and room temperature.

## Programming
Any of these parts can be programmed by use of any ISP programmer. 4k and 8k parts can be programmed over the software serial port using Optiboot, and 8k parts can be programmed via Micronucleus. Be sure to read the section of the main readme on the ISP programmers and IDE versions. 1.8.13 is recommended for best results.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny841/441, operating on the hardware UART0 (Serial) port at 115200 baud for 12 MHz or higher, or 57600 when running at lower speeds. The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on these parts, which do not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application (versions of the core prior to 1.2.0 used WDT vector, so WDT cannot be used as an interrupt - we recommend burning bootloader with the new version if this is an issue). A version of the bootloader that operates on Serial1 is included as well (choose the desired UART when burning the booloader).

### Micronucleus VUSB Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny841, allowing sketches to be uploaded directly over USB. The board definition runs at 8 MHz via the internal oscillator. For low power applications, it can be prescaled as listed in the table, or cranked up to 16 MHz for the adventurous. See the document on [Micronucleus usage](Ref_Micronucleus.md) for more information. In order to achieve the 12 MHz clock during USB operation, the OSCCAL is drastically increased to 12MHz while the bootloader is running, but is set back down before running the sketch.

Three USB pin mapping options are provided, with D+ on PB0, PB1 or PB2, with D- on either PB0 or PB1. These correspond to hardware seen in the wild.

## Features

### Internal Oscillator voltage dependence
Prior to 1.4.0, many users had encountered issues due to the voltage dependence of the oscillator. While the calibration is very accurate between 2.7 and 4v, as the voltage rises above 4.5v, the speed increases significantly. Although the magnitude of this is larger than on many of the more common parts, the issue is not as severe as had long been thought - the impact had been magnified by the direction of baud rate error, and the fact that many US ports actually supply 5.2-5.3v. As of 1.4.0, a simple solution was implemented to enable the same bootloader to work across the 8 MHz (Vcc < 4.5v) and 8 MHz (Vcc > 4.5 MHz ) board definitions, as well as the 16 MHz Internal option (albeit running at 8MHz) - it should generally work between 2.7v and 5.25v - though the extremes of that range may be dicey. We do still provide a >4.5v clock option in order to improve behavior of the running sketch - it will nudge the oscillator calibration down to move it closer to the nominal 8MHz clock speed; sketches uploaded with the higher voltage option. This is not perfect, but it is generally good enough to work with Serial on around 5v (including 5.25v often found on USB ports to facilitate chargeing powerhungry devices), and millis()/micros() will keep better time than in previous versions.

### Internal 16 MHz oscillator?
The oscillator on the ATtiny841 and ATtiny441 is very unusual in terms of how high the frequency can be turned up. While the datasheet only claims 7.3 - 8.1 MHz - it is in fact possible to crank it up all the way to a full 16 MHz! Furthermore, the behavior is consistent enough that the vast majaority work without individual tuning, so this is now available from the tools submenu with the other options.

~Unfortunately, because the same oscillator is used to time Flash and EEPROM writes, these should never be written to or erased while the device is running at 16 MHz (or for that matter, faster than 8.8, according to the datasheet). For the bootloader, this is handled by simply using the same bootloader as used for the 8 MHz Internal options - the oscillator is reconfigured by the sketch, not the bootloader); when the part is reset to run the bootloader, k. **The EEPROM is a more complicated issue** - this was addressed in the EEPROM.h library included with the core by calling newly provided functions to slow the oscillator back down while each byte is written, and speed it back up and correct the millis()/micros() timekeeping for the disruption. **During the write, all PWM frequencies will be halved, and on pins driven by Timer0, after this 3.3-3.4ms disruption a brief glitch can be seen on the PWM pins as the timer is put back in a state consistent with where it would have been had the disruption not occurred.** Due to this clock disruption, **Serial data sent or received during this time will be mangled**. When writing to the EEPROM, ensure that all data has been sent with Serial.flush(), and that it is not expected to receive any serial data during the write.~
On the basis of new information received from the esteemed Ralph Doncaster (@nerdralph), who has tested this on other AVRs with the internal oscillator cranked all the way to 255, including baking parts to reflow-soldering temperatures, flash written at higher speeds seems to function just fine! The functionality to slow down the clock during EEPROM writes has been removed as of ATTinyCore 2.0.0 - please be on guard for problems flashing and report them promptly.
Note that Optiboot is never run at the full 16 MHz; if the chip has been tuned with our tuning sketch, Optiboot will use the tuned 8 MHz value, not the 16 MHz one. (TODO: If this really does work as well as claimed, maybe we should run it at 12 MHz - 115200 baud is much better at 12 MHz than at 8 or 16)

### Pin mapping options
Throughout the history of ATtiny Arduino support, two pin mappings have been used. Here, they are referred to descriptively: the clockwise, and counterclockwise pinout. The desired pin mapping can be chosen from the Tools -> Pin Mapping submenu. Be very sure that you have selected the one that you wrote your sketch for, as debugging these issues can be surprisingly timeconsuming. As of 1.4.0, your sketch can check for PINMAPPING_CCW or PINMAPPING_CW macro (eg, `#ifdef PINMAPPING_CCW` - I would recommend checking for the incompatible one, and immediately #error'ing if you find it). Alternately, also as of 1.4.0, with either pin mapping selected, you can always refer to pins by their port and number within that port, using the `PIN_Pxn` syntax - where x is the port letter, and n is the pin number, eg PIN_PA7 is PIN A7, which is pin 7 in the clockwise mapping and pin 3 in the counterclockwise mapping (don't use PIN_xn or Pxn) - in this case the pin mapping won't matter.

Example of a "guard" against wrong pin mapping:
```c
#ifdef PINMAPPING_CCW
#error "Sketch was written for clockwise pin mapping!"
#endif
```

All pin mapping options assume that PB2 has the LED (bootloaders will blink that pin, and LED_BUILTIN is defined as PIN_PB2), unless it's a micronucleus configuration with D+ on PB2, in which case it will instead use PB0.

### PWM frequency
TC0 is always run in Fast PWM mode: We use TC0 for millis, and phase correct mode can't be used on the millis timer - you need to read the count to get micros, but that doesn't tell you the time in phase correct mode because you don't know if it's upcounting or downcounting in phase correct mode. Unique among the tinyAVRs, the x41 parts have a third timer. TC1 and TC2 are both the "good" timers, the 16-bit-capable ones.

| F_CPU  | F_PWM<sub>TC0</sub> | F_PWM<sub>TC1/2</sub> | Notes                        |
|--------|---------------------|-----------------------|------------------------------|
| 1  MHz | 1/8/256=     488 Hz |  1/8/256=      488 Hz |                              |
| 2  MHz | 2/8/256=     977 Hz |  2/8/256=      977 Hz |                              |
| <4 MHz | x/8/256= 488 * x Hz |  x/8/512=  244 * x Hz | Phase correct TC1/2          |
| 4  MHz | 4/8/256=    1960 Hz |  4/8/512=      977 Hz | Phase correct TC1/2          |
| <8 MHz | x/64/256= 61 * x Hz |  x/8/512=  244 * x Hz | Between 4 and 8 MHz, the target range is elusive | Phase correct TC1 |
| 8  MHz | 8/64/256=    488 Hz |  8/64/256=     488 Hz |                              |
| >8 MHz | x/64/256= 61 * x Hz |  x/64/256=  61 * x Hz |                              |
| 12 MHz | 12/64/256=   735 Hz | 12/64/256=     735 Hz |                              |
| 16 MHz | 16/64/256=   977 Hz | 16/64/256=     977 Hz |                              |
|>16 MHz | x/64/256= 61 * x Hz |  x/64/512=  31 * x Hz | Phase correct TC1/2          |
| 20 MHz | 20/64/256=  1220 Hz | 20/64/512=     610 Hz | Phase correct TC1/2          |

Phase correct PWM counts up to 255, turning the pin off as it passes the compare value, updates it's double-buffered registers at TOP, then it counts down to 0, flipping the pin back as is passes the compare value. This is considered preferable for motor control applications, though the "Phase and Frequency Correct" mode is better if the period is ever adjusted by a large amount at a time, because it updates the doublebuffered registers at BOTTOM, and thus produces a less problematic glitch in the duty cycle, but doesn't have any modes that don't require setting ICR1 too.

For more information see the [Changing PWM Frequency](Ref_ChangePWMFreq.md) reference.

### Tone support
The standard Tone() function is supported on these parts. For best results, use PA5 (pin 5 on either pinout) or PA6 (pin 4 on counterclockwise, pin 6 on clockwise), as this will use hardware output compare to generate the square wave, instead of interrupts.

### I2C support
**There is no I2C master functionality implemented in hardware**. The included Wire.h library will use a software implementation to provide I2C master functionality, and the hardware I2C slave for slave functionality, and can be used as a drop-in replacement for Wire.h with the caveat that clock speed cannot be set. **You must have external pullup resistors installed** in order for I2C functionality to work reliably; be aware that error reporting of the software I2C master is hit or miss.

### SPI Support
There is hardware SPI support. Use the normal SPI module.

### UART (Serial) Support
There are **two** hardware serial ports, Serial and Serial1. It works the same as Serial on any normal Arduino - it is not a software implementation.

To use only TX or only RX channel, after Serial.begin(), one of the following commands will disable the TX or RX channels (for Serial1, use UCSR1B instead) - Note that this works on any hardware serial port from any classic AVR, with *very* few exceptions. (off the top of my head, some very early ATmegas don't have the 0 in the register names for their only USART, and the ATtiny87/167, because it doesn't have a USART, it has a fancypants LIN thing that makes a killer UART (no S - it doesn't support the rarely used synchronous mode, nor can you turn it into an SPI module like you can a normal USART) if you turn off all the LIN stuff)
```c
UCSR0B &=~(1<<TXEN0); // disable TX
UCSR0B &=~(1<<RXEN0); // disable RX
```

### Overclocking
Experience has shown that the ATtiny x41-family, operating at 5v and room temperature, will typically function at 20 MHz without issue, although this is outside of the manufacturer's specification. The internal oscillator can also be cranked up to 16 MHz as noted above!

## ADC Features
The ATtinyx41 family boasts one of the most advanced ADCs in the entire classic tinyAVR/megaAVR product line. All pins are connected to the ADC, analogRead() works on all pins. All pins can be used as one member of at least 3 differential pairs, and in differential mode, gain can be selected at 1x, 20x, or 100x. All pin combinations supported by the ATtiny84 are supported here, and more.

### Reference options
Three internal reference voltages are provided, and they may be connected to the AREF pin for external bypassing to improve reference voltage stability. The AREF pin, if used as AREF, should have a capacitor connected between AREF and Gnd, and should not be used for other purposes. This is likely unnecessary under typical operating conditions - it would become more important in differential ADC measurements with gain selected. You should ALWAYS use the names to refer to reference settings, never the numbers which you think they correspond to (in ATTinyCore 2.0.0, these aren't the numbers they were before: We now pre-calculate the result of the bitshift operations when the reference setting is used - hence it can be done at compile time instead of runtime, saving significant time and a small amount of flash. At 8 MHz, most parts were wasting about 4 uS per analogRead()

| Reference Option   | Reference Voltage           | Uses AREF Pin        |
|--------------------|-----------------------------|----------------------|
| `DEFAULT`          | Vcc                         | No, pin available    |
| `EXTERNAL`         | Voltage applied to AREF pin | Yes, ext. voltage    |
| `INTERNAL1V1`      | Internal 1.1V reference     | No, pin available    |
| `INTERNAL2V2`      | Internal 2.2V reference     | No, pin available    |
| `INTERNAL4V096`    | Internal 4.096V reference   | No, pin available    |
| `INTERNAL1V1_CAP`  | Internal 1.1V reference     | Yes, w/cap. on AREF  |
| `INTERNAL2V2_CAP`  | Internal 2.2V reference     | Yes, w/cap. on AREF  |
| `INTERNAL4V096_CAP`| Internal 4.096V reference   | Yes, w/cap. on AREF  |

Accuracy of voltage reference spec'ed as +/- 3% over rated temperature and voltage range for 1.1V and 2.2V references, and +/- 4%for the 4.096V

#### Synonyms for reference options
Due to the long and storied history of this core, there are a large number of synonyms or aliases of these to ensure that old code can still be compiled.
| Reference Name         | Alias of                | Remarks                        |
|------------------------|-------------------------|--------------------------------|
| `INTERNAL1V1_NO_CAP`   | `INTERNAL1V1`           | Clear, but verbose             |
| `INTERNAL2V2_NO_CAP`   | `INTERNAL2V2`           | Clear, but verbose             |
| `INTERNAL4V096_NO_CAP` | `INTERNAL4V096`         | Clear, but verbose             |
| `INTERNAL4V1`          | `INTERNAL4V096`         | It's only accurate to +/- 4%   |
| `INTERNAL`             | `INTERNAL1V1`           | deprecated                     |
| `INTERNAL4V`           | `INTERNAL4V096`         | deprecated                     |
| `INTERNAL1V1_AREF`     | `INTERNAL1V1_CAP`       | deprecated                     |
| `INTERNAL2V2_AREF`     | `INTERNAL2V2_CAP`       | deprecated                     |
| `INTERNAL4V096_AREF`   | `INTERNAL4V096_CAP`     | deprecated                     |
| `INTERNAL1V1NOBP`      | `INTERNAL1V1`           | deprecated                     |
| `INTERNAL2V2NOBP`      | `INTERNAL2V2`           | deprecated                     |
| `INTERNAL4V096NOBP`    | `INTERNAL4V096`         | deprecated                     |


### Internal Sources
| Voltage Source  | Description                            |
|-----------------|----------------------------------------|
| ADC_INTERNAL1V1 | Reads the INTERNAL1V1 reference        |
| ADC_GROUND      | Reads ground - for offset correction?  |
| ADC_TEMPERATURE | Reads internal temperature sensor      |

### Differential ADC channels

| Pos. Chan | Pin | Neg. Chan | Pin |  Chan. Name | Channel |
|-----------|-----|-----------|-----|-------------|---------|
|      ADC0 | PA0 |      ADC1 | PA1 |  DIFF_A0_A1 |    0x10 |
|      ADC0 | PA0 |      ADC3 | PA3 |  DIFF_A0_A3 |    0x11 |
|      ADC1 | PA1 |      ADC2 | PA2 |  DIFF_A1_A2 |    0x12 |
|      ADC1 | PA1 |      ADC3 | PA0 |  DIFF_A1_A3 |    0x13 |
|      ADC2 | PA2 |      ADC3 | PA3 |  DIFF_A2_A3 |    0x14 |
|      ADC3 | PA3 |      ADC4 | PA4 |  DIFF_A3_A4 |    0x15 |
|      ADC3 | PA3 |      ADC5 | PA5 |  DIFF_A3_A5 |    0x16 |
|      ADC3 | PA3 |      ADC6 | PA6 |  DIFF_A3_A6 |    0x17 |
|      ADC3 | PA3 |      ADC7 | PA7 |  DIFF_A3_A7 |    0x18 |
|      ADC4 | PA4 |      ADC5 | PA5 |  DIFF_A4_A5 |    0x19 |
|      ADC4 | PA4 |      ADC6 | PA6 |  DIFF_A4_A6 |    0x1A |
|      ADC4 | PA4 |      ADC7 | PA7 |  DIFF_A4_A7 |    0x1B |
|      ADC5 | PA5 |      ADC6 | PA6 |  DIFF_A5_A6 |    0x1C |
|      ADC5 | PA5 |      ADC7 | PA7 |  DIFF_A5_A7 |    0x1D |
|      ADC6 | PA6 |      ADC7 | PA7 |  DIFF_A6_A7 |    0x1E |
|      ADC8 | PB2 |      ADC9 | PB3 |  DIFF_A8_A9 |    0x1F |
|      ADC0 | PA0 |      ADC0 | PA0 |  DIFF_A0_A0 |    0x20 |
|      ADC1 | PA1 |      ADC1 | PA1 |  DIFF_A1_A1 |    0x21 |
|      ADC2 | PA2 |      ADC2 | PA2 |  DIFF_A2_A2 |    0x22 |
|      ADC3 | PA3 |      ADC3 | PA3 |  DIFF_A3_A3 |    0x23 |
|      ADC4 | PA4 |      ADC4 | PA4 |  DIFF_A4_A4 |    0x24 |
|      ADC5 | PA5 |      ADC5 | PA5 |  DIFF_A5_A5 |    0x25 |
|      ADC6 | PA6 |      ADC6 | PA6 |  DIFF_A6_A6 |    0x26 |
|      ADC7 | PA7 |      ADC7 | PA7 |  DIFF_A7_A7 |    0x27 |
|      ADC8 | PB2 |      ADC8 | PB2 |  DIFF_A8_A8 |    0x28 |
|      ADC9 | PB3 |      ADC9 | PB3 |  DIFF_A9_A9 |    0x29 |
|     ADC10 | PB1 |      ADC8 | PB2 | DIFF_A10_A8 |    0x2A |
|     ADC10 | PB1 |      ADC9 | PB2 | DIFF_A10_A9 |    0x2B |
|     ADC11 | PB0 |      ADC8 | PB2 | DIFF_A11_A8 |    0x2C |
|     ADC11 | PB0 |      ADC9 | PB2 | DIFF_A11_A9 |    0x2D |
|      ADC1 | PA1 |      ADC0 | PA0 |  DIFF_A1_A0 |    0x30 |
|      ADC3 | PA3 |      ADC0 | PA0 |  DIFF_A3_A0 |    0x31 |
|      ADC2 | PA2 |      ADC1 | PA1 |  DIFF_A2_A1 |    0x32 |
|      ADC3 | PA3 |      ADC1 | PA1 |  DIFF_A3_A1 |    0x33 |
|      ADC3 | PA3 |      ADC2 | PA2 |  DIFF_A3_A2 |    0x34 |
|      ADC4 | PA4 |      ADC3 | PA3 |  DIFF_A4_A3 |    0x35 |
|      ADC5 | PA5 |      ADC3 | PA3 |  DIFF_A5_A3 |    0x36 |
|      ADC6 | PA6 |      ADC3 | PA3 |  DIFF_A6_A3 |    0x37 |
|      ADC7 | PA7 |      ADC3 | PA3 |  DIFF_A7_A3 |    0x38 |
|      ADC5 | PA5 |      ADC4 | PA4 |  DIFF_A5_A4 |    0x39 |
|      ADC6 | PA6 |      ADC4 | PA4 |  DIFF_A6_A4 |    0x3A |
|      ADC7 | PA7 |      ADC4 | PA4 |  DIFF_A7_A4 |    0x3B |
|      ADC6 | PA6 |      ADC5 | PA5 |  DIFF_A6_A5 |    0x3C |
|      ADC7 | PA7 |      ADC5 | PA5 |  DIFF_A7_A5 |    0x3D |
|      ADC7 | PA7 |      ADC6 | PA6 |  DIFF_A7_A6 |    0x3E |
|      ADC9 | PB3 |      ADC8 | PB2 |  DIFF_A9_A8 |    0x3F |


#### ADC Differential Pair Matrix
The t x41-series parts offer a superset of the tinyx4-series; in the below table,**bold** indicates that an option was not available on the ATtiny x4-series
|  N\P  |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |   10  |   11  |
|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|   0   |   X   |   X   |       |   X   |       |       |       |       |       |       |       |       |
|   1   |   X   | **X** |   X   |   X   |       |       |       |       |       |       |       |       |
|   2   |       |   X   | **X** |   X   |       |       |       |       |       |       |       |       |
|   3   |   X   |   X   |   X   |   X   |   X   |   X   |   X   |   X   |       |       |       |       |
|   4   |       |       |       |   X   | **X** |   X   | **X** | **X** |       |       |       |       |
|   5   |       |       |       |   X   |   X   | **X** |   X   | **X** |       |       |       |       |
|   6   |       |       |       |   X   | **X** |   X   | **X** |   X   |       |       |       |       |
|   7   |       |       |       |   X   | **X** | **X** |   X   |   X   |       |       |       |       |
|   8   |       |       |       |       |       |       |       |       | **X** | **X** | **X** | **X** |
|   9   |       |       |       |       |       |       |       |       | **X** | **X** | **X** | **X** |


### Temperature Measurement
To measure the temperature, select the 1.1v internal voltage reference, and analogRead(ADC_TEMPERATURE); This value changes by approximately 1 LSB per degree C. This requires calibration on a per-chip basis to translate to an actual temperature, as the offset is not tightly controlled - take the measurement at a known temperature (we recommend 25C - though it should be close to the nominal operating temperature, since the closer to the single point calibration temperature the measured temperature is, the more accurate that calibration will be without doing a more complicated two-point calibration (which would also give an approximate value for the slope)) and store it in EEPROM (make sure that `EESAVE` fuse is set first, otherwise it will be lost when new code is uploaded via ISP) if programming via ISP, or at the end of the flash if programming via a bootloader (same area where oscillator tuning values are stored). See the section below for the recommended locations for these.

## Special I/O-pin related features
There are a few strange features relating to the GPIO pins on the ATtiny x41 family which are found only in a small number of other parts released around the same time.

### Separate pullup-enable register
Like the ATtiny828 and ATtiny1634, these have a fourth register for each port, PUEx, which controls the pullups (rather than PORTx when DDRx has pin set as input). Unlike those other parts, though, these are not IO-space (they're normal registers, unlike the PORTx/PINx/DDRx registers, which can be used with the CBI/SBI/OUT/IN instructions). The Arduino digital IO functions account for this, but if you are using direct port manipulation, you need to know that setting a pin's bit in PORTx will not enable pullups. Use `PUEA` or `PUEB` - setting the corresponding bit to 1 will turn on the pullup. The pullup will be left on no matter what you do with the port, including setting it as output and driving it low - that will make it continually consume power and is "NOT RECOMMENDED" according to the datasheet. I would note that for a bidirectional data line that is high when idle, it could make sense to leave the pullup on, and pulse the line low to send by toggling the DDR line - especially because of one glaring bad design decision (which was not made on either of the other chips that have the PUE registers, the t1634 and t828): The PUEA and PUEB registers are NOT in the low I/O space. While `DDRA |= 1 << x;` (where x is known at compile-time) compiles to a single atomic `sbi` (set bit index) instruction, `PUEA |= 1 << x;` is a non-atomic read-modify-write cycle (if an interrupt occurs between the read and the write, and writes to the same register, the write after that interrupt would reverse any changes. In fact, not only that, *the PUEx registers are not in the high I/O space either!* so that doesn't compile to in, ori, out (3 clock cycles, 6 bytes), that (likely) compiles to lds, ori, sts (5 clock cycles, 10 bytes). Mystifying, especially considering that: a) they have 4 "reserved" registers in low I/O they could have used instead, and b)

### Special "high sink" pins
PA5 and PA7 have stronger sink capability than other pins - that is, the output voltage rises less when sinking a given amount of current.

```c
PHDE=(1<<PHDEA0)|(1<<PHDEA1); //PHDEA0 controls PA5, PHDEA1 controls PA7.
```

This is no great shakes - the Absolute Maximum current rating of 40mA still applies and all... but it does pull closer to ground with a a "large" 10-20mA load. A very strange feature of these parts; as far as I can tell it is only found on on the x41 family and the closely related ATtiny828. Which is also the only classic AVR I know of that allows you to map the timer PWM outputs to different pins the way these parts do.

**These high-sink pins have asymmetric drivers - thus PA5 and PA7 should not be used with PWM + RC filter to generate analog voltages** as the resulting voltage will be lower than expected (by an amount depending on the value of the resistor used - the higher the resistance, the smaller this impact will be)

### Break-before-make
The ATtiny x41 family also has a "Break-Before-Make" mode that can be enabled on a per-port basis with the `PORTCR` register, which will keep the pin tristated for 1 system clock cycle when a DDR bit is set from input to output. This is not used by the core, and I'm not sure what the intended use case was... (other than that it must have involved output values being set ahead of time on PORTx registers, and more than one of those pins being enabled and/or disabled simultaneously with a write to DDRx)

```c
PORTCR=(1<<BBMA)|(1<<BBMB); //BBMA controls PORTA, BBMB controls PORTB.
```


## Tuning Constant Locations
The 841/441, owing to the incredible power of it's oscillator, can be run at many speeds from the internal oscillator with proper calibration. We support storage of 4 calibration values. The included tuner uses these 4 locations for OSCCAL tuning values. If tuning is enabled, the OSCCAL tuning locations are checked at startup if tuning is enabled.

**ISP programming (no bootloader)**: EESAVE fuse set, stored in EEPROM

Optiboot used: Saved between end of bootloader and end of flash. See File -> Examples -> Temperature and Voltage

| Tuning Constant         | Location EEPROM | Location Flash |
|-------------------------|-----------------|----------------|
| Temperature Offset      | E2END - 3       | FLASHEND - 7   |
| Temperature Slope       | E2END - 4       | FLASHEND - 6   |
| Tuned OSCCAL0 8 MHz/3V3 | E2END - 3       | FLASHEND - 5   |
| Tuned OSCCAL0 8 MHz/5V  | E2END - 2       | FLASHEND - 4   |
| Tuned OSCCAL0 12 MHz*   | E2END - 1       | FLASHEND - 3   |
| Tuned OSCCAL0 16 MHz*   | E2END - 0       | FLASHEND - 2   |
| Bootloader Signature 1  | Not Used        | FLASHEND - 1   |
| Bootloader Signature 2  | Not Used        | FLASHEND       |

Mironucleus used: Micronucleus boards store a tuning value to the application section, but a separate sketch could also use a different means of calibration and store a value in the flash. The recommended locationsare shown below.


| Tuning Constant         |         Location Flash |
|-------------------------|------------------------|
| Tuned OSCCAL 12 MHz**   | BOOTLOADER_ADDRESS - 4 |
| Temperature Offset      |           FLASHEND - 5 |
| Temperature Slope       |           FLASHEND - 4 |
| Tuned OSCCAL0 8 MHz/3V3 |           FLASHEND - 3 |
| Tuned OSCCAL0 8 MHz/5V  |           FLASHEND - 2 |
| Tuned OSCCAL0 12 MHz*   |           FLASHEND - 1 |
| Tuned OSCCAL0 16 MHz*   |           FLASHEND     |


`*` Calibration at aprx. 5v is assumed and implied
`**` Calibrated at the USB voltage during programming


## Purchasing ATtiny841 Boards
I (Spence Konde / Dr. Azzy) sell ATtiny841 boards through my Tindie store - your purchases support the continued development of this core!

![Picture of ATtiny841 boards](https://d3s5r33r268y59.cloudfront.net/77443/products/thumbs/2015-06-16T05:30:56.533Z-T841RA_Assembled.png.855x570_q85_pad_rcrop.png)
### [Assembled Boards](https://www.tindie.com/products/DrAzzy/attiny841-dev-board-woptiboot-assembled/)
### [Bare Boards](https://www.tindie.com/products/DrAzzy/attiny84184-breakout-wserial-header-bare-board/)
### ATTiny841 DIP-14 TODO: List product and add links

The Wattuino board does not support maintenance of this core, but it does support the work they did to get that working:
### [Wattuino Nanite (VUSB)](https://shop.watterott.com/Wattuino-Nanite-841-ATtiny841-with-USB-Bootloader)


## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x41-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" (for example, if you misspell a vector name) - however, when that interrupt is triggered, the device will (at best) immediately reset (and not cleanly - I refer to this as a "dirty reset") The catastrophic nature of the failure often makes debugging challenging.

Note: The shown addresses below are "byte addressed" as that has proven more readily recognizable. The vector number is the number you are shown in the event of a duplicate vector error, as well as the interrupt priority (lower number = higher priority), if, for example, several interrupt flags are set while interrupts are disabled, the lowest numbered one would run first. Notice that INT0 is (as always) the highest priority interrupt. All of the parts in this family are 8k or less flash, so they do not need to use 4-byte vectors.


vect_num | Addr.  | Vector Name       | Interrupt Definition
|--------|--------|-------------------|-------------------------------------|
|      0 | 0x0000 | RESET_vect        | Any reset (pin, WDT, power-on, BOD) |
|      1 | 0x0002 | INT0_vect         | External Interrupt Request 0        |
|      2 | 0x0004 | PCINT0_vect       | Pin Change Interrupt 0 (PORT A)     |
|      3 | 0x0006 | PCINT1_vect       | Pin Change Interrupt 1 (PORT B)     |
|      4 | 0x0008 | WDT_vect          | Watchdog Time-out (Interrupt Mode)  |
|      5 | 0x000A | ~TIM1_CAPT_vect~  | Timer/Counter1 Capture Event        |
|      5 | 0x000A | TIMER1_CAPT_vect  | Alias - provided by ATTinyCore      |
|      6 | 0x000C | ~TIM1_COMPA_vect~ | Timer/Counter1 Compare Match A      |
|      6 | 0x000C | TIMER1_COMPA_vect | Alias - provided by ATTinyCore      |
|      7 | 0x000E | ~TIM1_COMPB_vect~ | Timer/Counter1 Compare Match B      |
|      7 | 0x000E | TIMER1_COMPB_vect | Alias - provided by ATTinyCore      |
|      8 | 0x0010 | ~TIM1_OVF_vect~   | Timer/Counter1 Overflow             |
|      8 | 0x0010 | TIMER1_OVF_vect   | Alias - provided by ATTinyCore      |
|      9 | 0x0012 | ~TIM0_COMPA_vect~ | Timer/Counter0 Compare Match A      |
|      9 | 0x0012 | TIMER0_COMPA_vect | Alias - provided by ATTinyCore      |
|     10 | 0x0014 | ~TIM0_COMPB_vect~ | Timer/Counter0 Compare Match B      |
|     10 | 0x0014 | TIMER0_COMPB_vect | Alias - provided by ATTinyCore      |
|     11 | 0x0016 | ~TIM0_OVF_vect~   | Timer/Counter0 Overflow             |
|     11 | 0x0016 | TIMER0_OVF_vect   | Alias - provided by ATTinyCore      |
|     12 | 0x0018 | ANA_COMP0_vect    | Analog Comparator 0                 |
|     13 | 0x001A | ADC_READY_vect    | ADC Conversion Complete             |
|     14 | 0x001C | EE_RDY_vect       | EEPROM Ready                        |
|     15 | 0x001E | ANA_COMP1_vect    | Analog Comparator 1                 |
|     16 | 0x0020 | ~TIM2_CAPT_vect~  | Timer/Counter2 Capture Event        |
|     16 | 0x0020 | TIMER2_CAPT_vect  | Alias - provided by ATTinyCore      |
|     17 | 0x0022 | ~TIM2_COMPA_vect~ | Timer/Counter2 Compare Match A      |
|     17 | 0x0022 | TIMER2_COMPA_vect | Alias - provided by ATTinyCore      |
|     18 | 0x0024 | ~TIM2_COMPB_vect~ | Timer/Counter2 Compare Match B      |
|     18 | 0x0024 | TIMER2_COMPB_vect | Alias - provided by ATTinyCore      |
|     19 | 0x0026 | ~TIM2_OVF_vect~   | Timer/Counter2 Overflow             |
|     19 | 0x0026 | TIMER2_OVF_vect   | Alias - provided by ATTinyCore      |
|     20 | 0x0028 | SPI_vect          | SPI Serial Transfer Complete        |
|     21 | 0x002A | USART0_RXS_vect   | USART0 Rx Start                     |
|     22 | 0x002C | USART0_RXC_vect   | USART0 Rx Complete                  |
|     23 | 0x002E | USART0_DRE_vect   | USART0 Data Register Empty          |
|     24 | 0x0030 | USART0_TXC_vect   | USART0 Tx Complete                  |
|     25 | 0x0032 | USART1_RXS_vect   | USART1 Rx Start                     |
|     26 | 0x0034 | USART1_RXC_vect   | USART1 Rx Complete                  |
|     27 | 0x0036 | USART1_DRE_vect   | USART1 Data Register Empty          |
|     28 | 0x0038 | USART1_TXC_vect   | USART1 Tx Complete                  |
|     29 | 0x003A | TWI_vect          | TWI Slave Interrupt                 |
|     30 | 0x003B | QTRIP_vect        | QTouch                              |
