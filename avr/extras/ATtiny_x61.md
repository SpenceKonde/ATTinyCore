# ATtiny 261/461/861(a)
![x61 pin mapping](Pinout_x61.jpg "Arduino Pin Mapping for ATtiny x61-family")

Specification         |      ATtiny861   |        ATtiny861 |      ATtiny861   |       ATtiny461  |       ATtiny461  |       ATtiny261  |
----------------------|------------------|------------------|------------------|------------------|------------------|------------------|
Bootloader (if any)   |                  |         Optiboot |    Micronucleus  |                  |         Optiboot |                  |
Uploading uses        |     ISP/SPI pins |   Serial Adapter |   USB (directly) |     ISP/SPI pins |   Serial Adapter |     ISP/SPI pins |
Flash available user  |       8192 bytes |       7552 bytes |        TBD bytes |       4096 bytes |       3456 bytes |             2048 |
RAM                   |        512 bytes |        512 bytes |        512 bytes |        256 bytes |        256 bytes |              128 |
EEPROM                |        512 bytes |        512 bytes |        512 bytes |        256 bytes |        256 bytes |              128 |
GPIO Pins             |       15 + RESET |       15 + RESET |       15 + RESET |       15 + RESET |       15 + RESET |       15 + RESET |
ADC Channels          |    11 (incl RST) |    11 (incl RST) |    11 (incl RST) |    11 (incl RST) |    11 (incl RST) |    11 (incl RST) |
Differential ADC      |1x/8x/20x/32 gain |1x/8x/20x/32 gain |1x/8x/20x/32 gain |1x/8x/20x/32 gain |1x/8x/20x/32 gain |1x/8x/20x/32 gain |
PWM Channels          | 3: PB1, PB3, PB5 | 3: PB1, PB3, PB5 | 3: PB1, PB3, PB5 | 3: PB1, PB3, PB5 | 3: PB1, PB3, PB5 | 3: PB1, PB3, PB5 |
Interfaces            |              USI |              USI |        vUSB, USI |              USI |              USI |              USI |
Clocking Options:     |           in MHz |           in MHz |           in MHz |           in MHz |           in MHz |           in MHz |
Int. Oscillator       |   16, 8, 4, 2, 1 |   16, 8, 4, 2, 1 |      16.5. 16, 8 |   16, 8, 4, 2, 1 |   16, 8, 4, 2, 1 |   16, 8, 4, 2, 1 |
Internal, with tuning |      16.5, 12, 8 |      16.5, 12, 8 |      16.5, 12, 8 |      16.5, 12, 8 |      16.5, 12, 8 |      16.5, 12, 8 |
External Crystal      |     All Standard |     All Standard |    Not supported |     All Standard |     All Standard |     All Standard |
External Clock        |     All Standard |     All Standard |    Not supported |     All Standard |     All Standard |     All Standard |
Int. WDT Oscillator   |          128 kHz | Not as sys. clock| Not as sys. clock|          128 kHz | Not as sys. clock|          128 kHz |
Default Pin Mapping   |        Clockwise |        Clockwise |        Clockwise |        Clockwise |        Clockwise |        Clockwise |
LED_BUILTIN           |              PB6 |              PB6 |       PB5 or PB6 |              PB6 |              PB6 |              PB6 |


The 261/461/861 and 261a/461a/861a are functionally very similar; the latter replaced the former in 2009, and uses slightly less power. Actual ATtiny861 parts are rarely seen in circulation today. They have the same signatures and are almost* fully interchangible. It is extremely common to refer to the ATtiny861a as an ATtiny861.

The ATtiny861 is a specialized microcontroller designed specifically to address the demands of brushless DC (BLDC) motor control.  To this end, it has a PLL and high speed timer like the ATtiny85, and it's timer has a mode where it can output three complementary PWM signals (with controllable dead time), as is needed for driving a three phase BLDC motor. For reasons that were never clear to me, in addition to the high speed timer, it also got a very fancy ADC, second only to the one in the ATtiny841 in the number of channels and programmable gain capabilities. Everything other than Timer1 and the ADC is sub-par, and frankly, most of us would be happier with a normal timer 0 and timer 1 too,  It can also be used as a general purpose microcontroller with more pins than the ATtiny84/841. It is available in 20-pin SOIC or DIP package, or a 32-pin no-lead package (there is no more modestly sized no-lead package - they for some reason seemed to love using 32-pin QFN packages for tinyAVRs with 20 pins.)

## Programming
Any of these parts can be programmed by use of any ISP programmer. 4k and 8k parts can be programmed over the software serial port using Optiboot, and 8k parts can be programmed via Micronucleus. Be sure to read the section of the main readme on the ISP programmers and IDE versions. 1.8.13 is recommended for best results.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny861/461, operating using software serial at 19200 baud - the software serial uses the AIN0 and AIN1 pins, marked on pinout chart (see also UART section below). The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on the 861/461, which does not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

### Micronucleus Bootloader
As of 2.0.0, a Micronucleus bootloader is included as well! With a PLL begging to be nudged up half a Mhz for the the 16.5 MHz option, it has finally fulfilled it's destiny.

## Features

### Pin mapping options
Historically, there was one pin mapping for the x61-series used by ATTinyCore. It was inherited from older versions of this core. During developoment of 1.5.0, it was realized that the traditional pinout, to put it bluntly, sucked - really badly.  A rational pinout that allowed necessary values to use the I/O pins to be calculated in the cases of the  was created. Be very sure that you have selected the one that you wrote your sketch for - or, preferably, use the PIN_Pxn constants to refer to pins. Regardless of what pin mapping is selected, those constants always point towards that logical pin. The new pinout is recommended for all new development; it is not only more coherent, but also more efficient, as it simplifies runtime math that the core needs to do.

Example of a "guard" against wrong pin mapping:
```c
#ifdef PINMAPPING_LEGACY
  #error "Sketch was written for clockwise pin mapping!"
#endif
```

Both of these options assume that an LED is connected to PB6 - except for non-prototype Micronucleus boards, which expect it on PA4 instead.

### PLL Clock
The ATtiny x61-family parts have an on-chip PLL. This is clocked off the internal oscillator and nominally runs at 64 MHz when enabled. It is possible to clock the chip off 1/4th of the PLL clock speed, providing a 16MHz clock option without a crystal (this has the same accuracy problems as the internal oscillator driving it). Alternately, or in addition to using it to derive the system clock, Timer1 can be clocked off the PLL. See below. For use with USB libraries, a 16.5 MHz clock option is available; with the Micronucleus bootloader, a tuned value calculated from the USB clock is used, and this is the default clock option, otherwise, a heuristic is used to determine the tuning to get 16.5 MHz if the chip has not been "tuned" with a tuning sketch. .

### Timer1 is a high speed timer
This means it can be clocked at 64 MHz from the on-chip PLL. In the past a menu option was provided to configure this. It never worked, and in any event is insufficient to do much of practical use with. It was eliminated for 2.0.0. Instead, see the [ATTinyCore library](../libraries/ATTinyCore/README.md)

### PWM frequency
TC0 does not support PWM, and is used for millis. TC1 gives 3 outputs, and can operate in phase correct or fast pwm mode, or variations on those that operate only in PWM6 mode, which is designed specifically for controlling brushless DC (BLDC) motors. We always use phase correct when we can reach the target frequency with it. Which, on these parts, is all the time.

| F_CPU  | No PWM from TC0     | F_PWM<sub>TC1</sub>   | Notes                        |
|--------|---------------------|-----------------------|------------------------------|
| 1  MHz |                   - |  1/4/512=      488 Hz | Phase & Freq correct TC1     |
| 2  MHz |                   - |  2/8/512=      488 Hz | Phase & Freq correct TC1     |
| <4 MHz |                   - |  x/8/512=  244 * x Hz | Phase & Freq correct TC1     |
| 4  MHz |                   - |  4/8/512=      977 Hz | Phase & Freq correct TC1     |
| <8 MHz |                   - |  x/16/512= 122 * x Hz | Phase & Freq correct TC1     |
| 8  MHz |                   - |  8/32/512=     488 Hz | Phase & Freq correct TC1     |
| >8 MHz |                   - |  x/32/512=  61 * x Hz | Phase & Freq correct TC1     |
| 12 MHz |                   - | 12/32/512=     735 Hz | Phase & Freq correct TC1     |
| 16 MHz |                   - | 16/32/512=     977 Hz | Phase & Freq correct TC1     |
|>16 MHz |                   - |  x/64/512=  31 * x Hz | Phase & Freq correct TC1     |
| 20 MHz |                   - | 20/64/512=     610 Hz | Phase & Freq correct TC1     |

Phase & frequency correct PWM counts up to 255, turning the pin off as it passes the compare value, then it counts down to 0, flipping the pin back as is passes the compare value, and then updates it's double-buffered registers at BOTTOM. Phase and frequency correct PWM is almost never worse than normal PWM - as long as the

For more information see the [Changing PWM Frequency](Ref_ChangePWMFreq.md) reference.

### Tone Support
Tone() uses Timer1. If the high speed functionality of Timer1 has been enabled (see link above), tone() will not produce the expected frequencies, but rather ones 2 or 4 times higher. For best results, use pin 1 or 4 for tone - this will use Timer1's output compare unit to generate the tone, rather than generating an interrupt to toggle the pin. In this way, "tones" can be generated up into the MHz range.  If using SoftwareSerial or the builtin software serial "Serial", tone() will work on the PWM pins while the software serial is active but not on any other pins.  As only Timer1 is capable of hardware PWM on the x61 series, tone() will break all PWM functionality.

### I2C Support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire.h library included with this core.. This was definitely broken prior to 1.5.0. **You must have external pullup resistors installed** in order for I2C functionality to work at all. There is no need for libraries like TinyWire or USIWire or that kind of thing.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI. This should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common, and the only mode that the SPI library has ever supported).

### UART (Serial) Support
There is no hardware UART. If running off the internal oscillator, you may need to tune it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs. Because the analog comparator here is more sophisticated, you can choose to use either AIN0, AIN1, or AIN2 as the RX pin from the Builtin SoftSerial menu. Although it is named Serial, it is still a software implementation, so you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](Ref_TinySoftSerial.md)

| RX      | Pin | TX  (default) | Pin |
|---------|-----|---------------|-----|
| AIN1    | PA7 | AIN0          | PA6 |
| AIN2    | PA5 | AIN0          | PA6 |
| AIN0    | PA6 | AIN2          | PA5 |
| TX only | N/A | AIN0          | PA6 |


Though TX defaults to AIN0 (or AIN2), it can be moved to any pin on PORTA using Serial.setTxBit(b) where b is the number in the pin name using Pxn notation (only pins on PORTA are valid) (2.0.0+ only - was broken in earlier versions).

To disable the RX channel (to use only TX), select "TX only" from the Builtin SoftSerial tools menu. To disable the TX channel, simply don't print anything to it, and set it to the desired pinMode after Serial.begin()

### Servo Support
Although the timers are quite different, and historically there have been issues with the Servo library, we include a builtin Servo library that supports the Tiny x61 series. As always, while a software serial port is receiving or transmitting, the servo signal will glitch (this includes the builtin software serial "Serial).  On prior versions, a third party library must be used. The servo library will disable PWM on pin 4, regardless of which pin is used for output, and cannot be used at the same time as Tone. If you have installed a version of Servo through Library Manager, instead include `Servo_ATTinyCore.h` or it will use the incompatible library installed through library manager.

## ADC Features
The ATtiny861 has a surprisingly sophisticated ADC, one more advanced than many ATmega parts, with many differential channels, most with selectable gain. As of ATTinyCore 2.0.0, these are available through analogRead!  When used to read a pair of analog pins in differential mode, the ADC normally runs in unipolar mode: The voltage on the positive pin must be higher than that on the negative one, but the difference is measured to the full precision of the ADC. It can be put into bipolar mode, where the voltage on the negative side can go below the voltage on the positive side and generate meaningful measurements (it will return a signed value, which costs 1 bit of accuracy for the sign bit). This can be enabled by calling the helper function `setADCBipolarMode(true or false)`. On many AVR devices with a differential ADC, only bipolar mode is available.

### ADC Reference options
The ATtiny x61-series has two internal references, one of which can (optionally) use the AREF pin with an external capacitor for improved stability. It can also use an external reference voltage or the supply voltage. For historical reasons, there are several aliases available for some of these options; these are deprecated.

| Reference Option   | Reference Voltage           | Uses AREF Pin        | Aliases/synonyms                         |
|--------------------|-----------------------------|----------------------|------------------------------------------|
| `DEFAULT`          | Vcc                         | No, pin available    |                                          |
| `EXTERNAL`         | Voltage applied to AREF pin | Yes, ext. voltage    |                                          |
| `INTERNAL1V1`      | Internal 1.1V reference     | No, pin available    | `INTERNAL`                               |
| `INTERNAL2V56`     | Internal 2.56V reference    | No, pin available    | `INTERNAL2V56_NO_CAP` `INTERNAL2V56NOBP` |
| `INTERNAL2V56_CAP` | Internal 2.56V reference    | Yes, w/cap. on AREF  |                                          |

### Internal Sources
| Voltage Source  | Description                            |
|-----------------|----------------------------------------|
| ADC_INTERNAL1V1 | Reads the INTERNAL1V1 reference        |
| ADC_GROUND      | Reads ground - for offset correction?  |
| ADC_TEMPERATURE | Reads internal temperature sensor      |

### Differential ADC
There are 24 different differential pairs available. Seven of those are measuring the same positive and negative channel, these can be used to determine the offset error in the gain stage, which can then be subtracted from future measurements with the same gain selected. There are 31 combinations of gain and channel available with selectable gain (typically each pair offers both 1x/8x and 20/32x gain options), and 19 with fixed gain, 10 of which duplicate the ones with selectable gain, for a total of 81 possible settings. Using ATTinyCore (v2.0.0+) you can read from them with `analogRead()` by using the channel names shown below. If it is required to know the numeric values of the channels, they are shown below as well. If you must work with channel numbers, instead of a names, when passing them to `analogRead()`, use the `ADC_CH()` macro; the gain selection bit is passed as the 6th bit of the channel (ex: `analogRead(ADC_CH(0x20|0x40))` to read ADC0 and ADC1 at 32x gain, equivalent to `analogRead(DIFF_A0_A1_32X)`), otherwise they will be interpreted as a (likely non-existent) digital pin. In the case of duplicates in the list of gain options, the one that does not use the `GSEL` bit to select the gain is named followed by an A (ex, `DIFF_A6_A5_20XA`).

| Positive   | Negative   |   Gain  | Channel| Name 1x/20x mode | Name 8x/32x mode | Notes            |
|------------|------------|---------|--------|------------------|------------------|------------------|
| ADC2 (PA2) | ADC3 (PA4) |      1x |   0x10 | DIFF_A2_A3_1X    |                  |                  |
| ADC3 (PA4) | ADC3 (PA4) |     20x |   0x11 | DIFF_A3_A3_20X   |                  | For offset cal.  |
| ADC4 (PA5) | ADC3 (PA4) |     20x |   0x12 | DIFF_A4_A3_20X   |                  |                  |
| ADC4 (PA5) | ADC3 (PA4) |      1x |   0x13 | DIFF_A4_A3_1X    |                  |                  |
| ADC8 (PB5) | ADC9 (PB6) |     20x |   0x19 | DIFF_A8_A9_20X   |                  |                  |
| ADC8 (PB5) | ADC9 (PB6) |      1x |   0x1A | DIFF_A8_A9_1X    |                  |                  |
| ADC9 (PB6) | ADC9 (PB6) |     20x |   0x1B | DIFF_A9_A9_20X   |                  | For offset cal.  |
| ADC10(PB7) | ADC9 (PB6) |     20x |   0x1C | DIFF_A10_A9_20X  |                  |                  |
| ADC10(PB7) | ADC9 (PB6) |      1x |   0x1D | DIFF_A10_A9_1X   |                  |                  |
| ADC0 (PA0) | ADC1 (PA1) | 20x/32x |   0x20 | DIFF_A0_A1_20X   | DIFF_A0_A1_32X   |                  |
| ADC0 (PA0) | ADC1 (PA1) |   1x/8x |   0x21 | DIFF_A0_A1_1X    | DIFF_A0_A1_8X    |                  |
| ADC1 (PA1) | ADC0 (PA0) | 20x/32x |   0x22 | DIFF_A1_A0_20X   | DIFF_A1_A0_32X   |                  |
| ADC1 (PA1) | ADC0 (PA0) |   1x/8x |   0x23 | DIFF_A1_A0_1X    | DIFF_A1_A0_8X    |                  |
| ADC1 (PA1) | ADC2 (PA2) | 20x/32x |   0x24 | DIFF_A1_A2_20X   | DIFF_A1_A2_32X   |                  |
| ADC1 (PA1) | ADC2 (PA2) |   1x/8x |   0x25 | DIFF_A1_A2_1X    | DIFF_A1_A2_8X    |                  |
| ADC2 (PA2) | ADC1 (PA1) | 20x/32x |   0x26 | DIFF_A2_A1_20X   | DIFF_A2_A1_32X   |                  |
| ADC2 (PA2) | ADC1 (PA1) |   1x/8x |   0x27 | DIFF_A2_A1_1X    | DIFF_A2_A1_8X    |                  |
| ADC2 (PA2) | ADC0 (PA0) | 20x/32x |   0x28 | DIFF_A2_A0_20X   | DIFF_A2_A0_32X   |                  |
| ADC2 (PA2) | ADC0 (PA0) |   1x/8x |   0x29 | DIFF_A2_A0_1X    | DIFF_A2_A0_8X    |                  |
| ADC0 (PA0) | ADC2 (PA2) | 20x/32x |   0x2A | DIFF_A0_A2_20X   | DIFF_A0_A2_32X   |                  |
| ADC0 (PA0) | ADC2 (PA2) |   1x/8x |   0x2B | DIFF_A0_A2_1X    | DIFF_A0_A2_8X    |                  |
| ADC4 (PA5) | ADC5 (PA6) | 20x/32x |   0x2C | DIFF_A4_A5_20X   | DIFF_A4_A5_32X   |                  |
| ADC4 (PA5) | ADC5 (PA6) |   1x/8x |   0x2D | DIFF_A4_A5_1X    | DIFF_A4_A5_8X    |                  |
| ADC5 (PA6) | ADC4 (PA5) | 20x/32x |   0x2E | DIFF_A5_A4_20X   | DIFF_A5_A4_32X   |                  |
| ADC5 (PA6) | ADC4 (PA5) |   1x/8x |   0x2F | DIFF_A5_A4_1X    | DIFF_A5_A4_8X    |                  |
| ADC5 (PA6) | ADC6 (PA7) | 20x/32x |   0x30 | DIFF_A5_A6_20X   | DIFF_A5_A6_32X   |                  |
| ADC5 (PA6) | ADC6 (PA7) |   1x/8x |   0x31 | DIFF_A5_A6_1X    | DIFF_A5_A6_8X    |                  |
| ADC6 (PA7) | ADC5 (PA6) | 20x/32x |   0x32 | DIFF_A6_A5_20X   | DIFF_A6_A5_32X   |                  |
| ADC6 (PA7) | ADC5 (PA6) |   1x/8x |   0x33 | DIFF_A6_A5_1X    | DIFF_A6_A5_8X    |                  |
| ADC6 (PA7) | ADC4 (PA5) | 20x/32x |   0x34 | DIFF_A6_A4_20X   | DIFF_A6_A4_32X   |                  |
| ADC6 (PA7) | ADC4 (PA5) |   1x/8x |   0x35 | DIFF_A6_A4_1X    | DIFF_A6_A4_8X    |                  |
| ADC4 (PA5) | ADC6 (PA7) | 20x/32x |   0x36 | DIFF_A4_A6_20X   | DIFF_A4_A6_32X   |                  |
| ADC4 (PA5) | ADC6 (PA7) |   1x/8x |   0x37 | DIFF_A4_A6_1X    | DIFF_A4_A6_8X    |                  |
| ADC0 (PA0) | ADC0 (PA0) | 20x/32x |   0x38 | DIFF_A0_A0_20X   | DIFF_A0_A0_32X   | For offset cal.  |
| ADC0 (PA0) | ADC0 (PA0) |   1x/8x |   0x39 | DIFF_A0_A0_1X    | DIFF_A0_A0_8X    | For offset cal.  |
| ADC1 (PA1) | ADC1 (PA1) | 20x/32x |   0x3A | DIFF_A1_A1_20X   | DIFF_A1_A1_32X   | For offset cal.  |
| ADC2 (PA2) | ADC2 (PA2) | 20x/32x |   0x3B | DIFF_A2_A2_20X   | DIFF_A2_A2_32X   | For offset cal.  |
| ADC4 (PA5) | ADC4 (PA5) | 20x/32x |   0x3C | DIFF_A4_A4_20X   | DIFF_A4_A4_32X   | For offset cal.  |
| ADC5 (PA6) | ADC5 (PA6) | 20x/32x |   0x3D | DIFF_A5_A5_20X   | DIFF_A5_A5_32X   | For offset cal.  |
| ADC6 (PA7) | ADC6 (PA7) | 20x/32x |   0x3E | DIFF_A6_A6_20X   | DIFF_A6_A6_32X   | For offset cal.  |
| ADC0 (PA0) | ADC1 (PA1) |     20x |   0x0B | DIFF_A0_A1_20XA  |                  | Duplicate        |
| ADC0 (PA0) | ADC1 (PA1) |      1x |   0x0C | DIFF_A0_A1_1XA   |                  | Duplicate        |
| ADC1 (PA1) | ADC1 (PA1) |     20x |   0x0D | DIFF_A1_A1_20XA  |                  | Duplicate        |
| ADC2 (PA2) | ADC1 (PA1) |     20x |   0x0E | DIFF_A2_A1_20XA  |                  | Duplicate        |
| ADC2 (PA2) | ADC1 (PA1) |      1x |   0x0F | DIFF_A2_A1_1XA   |                  | Duplicate        |
| ADC4 (PA5) | ADC5 (PA6) |     20x |   0x14 | DIFF_A4_A5_20XA  |                  | Duplicate        |
| ADC4 (PA5) | ADC5 (PA6) |      1x |   0x15 | DIFF_A4_A5_1XA   |                  | Duplicate        |
| ADC5 (PA6) | ADC5 (PA6) |     20x |   0x16 | DIFF_A5_A5_20XA  |                  | Duplicate        |
| ADC6 (PA7) | ADC5 (PA6) |     20x |   0x17 | DIFF_A6_A5_20XA  |                  | Duplicate        |
| ADC6 (PA7) | ADC5 (PA6) |      1x |   0x18 | DIFF_A6_A5_1XA   |                  | Duplicate        |

Those 4 sets of 20x/1x channels are an exact copy of the channels on the ATtiny26 - the older version of these parts - with the same ADMUX values so that code and hardware could be directly copied. These do not support the GSEL for 8x or 32x gain. These are not "reversible" (meaning that if you guessed wrong about which was higher, all you could do is use the ADC in "bipolar input mode", which costs 1 bit of resolution). Then two of those "trios" of pins are available in both directions with GSEL: ADC0/ADC1/ADC2 and ADC4/ADC5/ADC6, and finally, ADC0-ADC0 is available with all gain options, and each of the other channels involved in the "second half" of the differential ADC are supported with 20x/32x gain. Since pairings and gain options available on the "first half" involving ADC1 and ADC5 are also available in the "second half" with additional functionality It is enough to make one wonder what the intent of the design was. Do they use the same pathways in the chip, or do you get different offsets when you, say measure ADC1 against itself using channel 0x0D (DIFF_A1_A1_20XA), vs channel 0x3A w/GSEL=0 (DIFF_A1_A1_20X)? If they use different pathways, then why did they choose to have two options to measure those 10 pairs - why not use those to support gain selection on more pins? If not - well, I guess it hangs together, they didn't want to add more mux channels and figured what they had was enough (I'm inclined to agree - this ADC is a beast for it's time).
#### ADC Differential Pair Matrix
* **bold** indicates that an option has all gain options available.
* *italic* indicates only 20x/32x gain
* Unstyled text indicates that only 1x and 20x are available
|  N\P  |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   8   |   9   |  10   |
|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
|   0   | **X** | **X** | **X** |       |       |       |       |       |       |       |
|   1   | **X** |  *X*  | **X** |       |       |       |       |       |       |       |
|   2   | **X** | **X** |  *X*  |       |       |       |       |       |       |       |
|   3   |       |       |   1x  |  20x  |   X   |       |       |       |       |       |
|   4   |       |       |       |       |  *X*  | **X** | **X** |       |       |       |
|   5   |       |       |       |       | **X** |  *X*  | **X** |       |       |       |
|   6   |       |       |       |       | **X** | **X** |  *X*  |       |       |       |
|   9   |       |       |       |       |       |       |       |   X   |  20x  |   X   |

### Temperature Measurement
To measure the temperature, select the 1.1v internal voltage reference, and analogRead(ADC_TEMPERATURE); This value changes by approximately 1 LSB per degree C. This requires calibration on a per-chip basis to translate to an actual temperature, as the offset is not tightly controlled - take the measurement at a known temperature (we recommend 25C - though it should be close to the nominal operating temperature, since the closer to the single point calibration temperature the measured temperature is, the more accurate that calibration will be without doing a more complicated two-point calibration (which would also give an approximate value for the slope)) and store it in EEPROM (make sure that `EESAVE` fuse is set first, otherwise it will be lost when new code is uploaded via ISP) if programming via ISP, or at the end of the flash if programming via a bootloader (same area where oscillator tuning values are stored). See the section below for the recommended locations for these.

### Tuning Constant Locations
These are the recommended locations to store tuning constants. In the case of the bootloader board definitions, it is critical to place the tuning values *into* the bootloader's flash pages, because otherwise every upload will erase the calibration. This in turn requires some measure of care, as you must be certain to rewrite the contents of the earlier parts of the page. (and of course that the bootloader has enough space after it to fit the these values )
ISP programming used: Make sure to have EESAVE fuse set, stored in EEPROM

Optiboot used: Saved between end of bootloader and end of flash.

| Tuning Constant        | Location EEPROM | Location Flash |
|------------------------|-----------------|----------------|
| Temperature Offset     | E2END - 5       | FLASHEND - 7   |
| Temperature Slope      | E2END - 4       | FLASHEND - 6   |
| Unspecified            | E2END - 3       | FLASHEND - 5   |
| Tuned OSCCAL 12 MHz    | E2END - 2       | FLASHEND - 4   |
| Tuned OSCCAL 8.25 MHz  | E2END - 1       | FLASHEND - 3   |
| Tuned OSCCAL 8 MHz     | E2END           | FLASHEND - 2   |
| Bootloader Signature 1 | Not Used        | FLASHEND - 1   |
| Bootloader Signature 2 | Not Used        | FLASHEND       |

Mironucleus used: Micronucleus boards store a tuning value to the application section, but a separate sketch could also use a different means of calibration and store a value in the flash of the bootloader section to persist the setting across uploads, but this seems to be an implausible use case.

| Tuning Constant        | Location Flash         |
|------------------------|------------------------|
| Tuned OSCCAL 8.25 MHz* | BOOTLOADER_ADDRESS - 4 |
| Temperature Offset     | FLASHEND - 3           |
| Temperature Slope      | FLASHEND - 2           |
| Tuned OSCCAL 8.25 MHz  | FLASHEND - 1           |
| Tuned OSCCAL 8 MHz     | FLASHEND -             |

`*` Calibrated at the USB voltage during programming

## Interrupt Vector Table
This table lists all of the interrupt vectors available on the ATtiny x61-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" (for example, if you misspell a vector name) - however, when that interrupt is triggered, the device will (at best) immediately reset (and not cleanly - I refer to this as a "dirty reset") The catastrophic nature of the failure often makes debugging challenging.

Note: The shown addresses below are "byte addressed" as that has proven more readily recognizable. The vector number is the number you are shown in the event of a duplicate vector error, as well as the interrupt priority (lower number = higher priority), if, for example, several interrupt flags are set while interrupts are disabled, the lowest numbered one would run first. Notice that INT0 is (as always) the highest priority interrupt. All of the parts  in this family are 8k or less flash, so they do not need to use 4-byte vectors.

**Note about PCINTs:** The x61's are abnormal here. There's a `PCMSK0` and `PCMSK1` for port A and B respectively, like normal. There are `PCIE0` and `PCIE1` bits in `GIMSK` to enable PCINTs on each port, like normal. But both of them call the same PCINT vector when triggered. That's right, *there's ONLY ONE PCINT VECTOR!*
"What? They made the ADCs able to use the same channels, but went from normal pcints on the tiny26 to weird ones here?"
Not at all! Where did you get the idea that the Tiny26 PCINTs were any less wacky or less primitive than the ones on the Tinyx61? I agree that from the list of vector names, it really does look like the tiny26 has two normal pcint banks. But if you read that chapter of the datasheet, you discover that those are the wackiest two (well, more like 1.5) banks of PCINTs you've ever seen. The single vector here is WAY more normal.

 # | Address | Vector Name          | Interrupt Definition
---|---------|----------------------|-------------
 0 |  0x0000 | RESET_vect           | Not an interrupt - this is a jump to the start of your code.
 1 |  0x0002 | INT0_vect            | External Interrupt Request 0
 2 |  0x0004 | PCINT_vect           | Pin Change Interrupt
 3 |  0x0006 | TIMER1_COMPA_vect    | Timer/Counter1 Compare Match A
 4 |  0x0008 | TIMER1_COMPB_vect    | Timer/Counter1 Compare Match B
 5 |  0x000A | TIMER1_OVF_vect      | Timer/Counter1 Overflow
 6 |  0x000C | TIMER0_OVF_vect      | Timer/Counter0 Overflow
 7 |  0x000E | USI_START_vect       | USI Start
 8 |  0x0010 | USI_OVF_vect         | USI Overflow
 9 |  0x0012 | EE_RDY_vect          | EEPROM Ready
10 |  0x0014 | ANA_COMP_vect        | Analog Comparator
11 |  0x0016 | ADC_vect             | ADC Conversion Complete
12 |  0x0018 | WDT_vect             | Watchdog Time-out (Interrupt Mode)
13 |  0x001A | INT1_vect            | External Interrupt Request 1
14 |  0x001C | TIMER0_COMPA_vect    | Timer/Counter0 Compare Match A
15 |  0x001E | TIMER0_COMPB_vect    | Timer/Counter0 Compare Match B
16 |  0x0020 | TIMER0_CAPT_vect     | Timer/Counter1 Capture Event
17 |  0x0022 | TIMER1_COMPD_vect    | Timer/Counter1 Compare Match D
18 |  0x0024 | FAULT_PROTECTION_vect| Timer/Counter1 Fault Protection

## 861 vs 861a - the one apparently relevant difference
There is one way in which the 861A is notably more usable than the 861. That is the fact that they have a markedly better internal oscillator. If you look at the graphs of internal oscillator speed vs OSCCAL, you will notice that some parts have a discontinuity in the middle. The 861 does. The 861A does not. These represent two different designs - The bifiurcated oscillator cal curve is associated an older, inferior oscillator design, and those parts are less suited to running micronucleus and, regardless of whether micronucleus is used, the bifurcated oscillator design is less more likely to have random problems caused by clock inaccuracy compared to the 861A with the non-bifurcated oscillator. Thankfully the 861-not-A is so unusual these days that it's more expensive than it's successor (same is true of the other tinies with an A after their number - The A is consistently cheaper than not-A, as they would love for everyone to switch to the strictly better A's
