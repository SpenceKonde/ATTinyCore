# ATtiny 25/45/85
![x5 pin mapping](Pinout_x5.jpg "Arduino Pin Mapping for ATtiny x5-family")

Specification         |    ATtiny85    |      ATtiny85  |    ATtiny85    |     ATtiny45   |       ATtiny45 |      ATtiny25  |
----------------------|----------------|----------------|----------------|----------------|----------------|----------------|
Bootloader (if any)   |                |       Optiboot |  Micronucleus  |                |       Optiboot |                |
Uploading uses        |   ISP/SPI pins | Serial Adapter | USB (directly) |   ISP/SPI pins | Serial Adapter |   ISP/SPI pins |
Flash available user  |     8192 bytes |     7552 bytes |     6586 bytes |     4096 bytes |     3456 bytes |           2048 |
RAM                   |      512 bytes |      512 bytes |      512 bytes |      256 bytes |      256 bytes |            128 |
EEPROM                |      512 bytes |      512 bytes |      512 bytes |      256 bytes |      256 bytes |            128 |
GPIO Pins             |      5 + RESET |      5 + RESET |      5 + RESET |      5 + RESET |      5 + RESET |      5 + RESET |
ADC Channels          |   4 (incl RST) |   4 (incl RST) |   4 (incl RST) |   4 (incl RST) |   4 (incl RST) |   4 (incl RST) |
Differential ADC      |     1/20x gain |     1/20x gain |     1/20x gain |     1/20x gain |     1/20x gain |     1/20x gain |
PWM Channels          |  3: PB0, PB1/2 |  3: PB0, PB1/2 |  3: PB0, PB1/2 |  3: PB0, PB1/2 |  3: PB0, PB1/2 |  3: PB0, PB1/2 |
Interfaces            |            USI |            USI |      vUSB, USI |            USI |            USI |            USI |
Clocking Options:     |         in MHz |         in MHz |         in MHz |         in MHz |         in MHz |         in MHz |
Int. Oscillator or PLL| 16, 8, 4, 2, 1 | 16, 8, 4, 2, 1 |    16.5. 16, 8 | 16, 8, 4, 2, 1 | 16, 8, 4, 2, 1 | 16, 8, 4, 2, 1 |
Internal, with tuning |    16.5, 12, 8 |    16.5, 12, 8 |    16.5, 12, 8 |    16.5, 12, 8 |    16.5, 12, 8 |    16.5, 12, 8 |
External Crystal      |   All Standard |   All Standard |  Not supported |   All Standard |   All Standard |   All Standard |
External Clock        |   All Standard |   All Standard |  Not supported |   All Standard |   All Standard |   All Standard |
Int. WDT Oscillator   |        128 kHz |        128 kHz |        128 kHz |        128 kHz |        128 kHz |        128 kHz |
LED_BUILTIN           |        PIN_PB1 |        PIN_PB1 |        PIN_PB1 |        PIN_PB1 |        PIN_PB1 |        PIN_PB1 |

## Programming
Any of these parts can be programmed by use of any ISP programmer. 4k and 8k parts can be programmed over the software serial port using Optiboot, and 8k parts can be programmed via Micronucleus. Be sure to read the section of the main readme on the ISP programmers and IDE versions. 1.8.13 is recommended for best results.

### Optiboot Bootloader
This core includes an Optiboot bootloader for the ATtiny85/45, operating using software serial at at the standard ATTinyCore baud rates (which have changed in 2.0.0 for improved reliability see [the Optboot reference](./Ref_Optiboot.md)) - the software serial uses the AIN0 and AIN1 pins (see UART section below). The bootloader uses 640b of space, leaving 3456 or 7552b available for user code. In order to work on these parts, which do not have hardware bootloader support (hence no BOOTRST functionality), "Virtual Boot" is used. This works around this limitation by rewriting the vector table of the sketch as it's uploaded - the reset vector gets pointed at the start of the bootloader, while the EE_RDY vector gets pointed to the start of the application.

Due to a defect in Optiboot, it is possible for the bootloader to trash itself and the installed application; In this case ISP reprogramming is required to fix it. This means that **optiboot is not suitable for production systems on this part** - eventually, the bug will get triggered, and they will need to be rebootloaded; in a production setting this is simply not acceptable. I know an what must be done to fix this but getting from that to code which does that has proven extraordinarily difficult, I have attempted several times, each time reaching a point where I had no idea how to proceed to finish the fix. I know what value it needs to write to what address, but not now to get from where I was to there.

Optiboot is available for the 85 and 45. It would fill more than 1/4th of the flash on the 25 and make the device very difficult to do anything useful with, and so we do not provide false hope by offering support.

### Micronucleus VUSB Bootloader
This core includes a Micronucleus bootloader that supports the ATtiny85, allowing sketches to be uploaded directly over USB. The board definition runs at 16.5 MHz via the internal PLL, adjusting the clock speed up slightly to get 16.5 MHz, and leaves it that way when the sketch is launched unless a slower clock speed is selected. These lower clock speeds are not compatible with USB libraries. See the document on [Micronucleus usage](Ref_Micronucleus.md) for more information. D- is on pin 3, D+ is on pin 4. Note that for the most part, libraries that make an ATtiny85 work as a USB device don't work correctly.

In 2.0.0, all of the usual micronucleus entry methods are available. It is shockingly robust considering the hackjob it is built upon.

#### VUSB from within the sketch? Not so much
Note that VUSB is only supported for loading code. After much very disappointing discussion with relevant experts and background research I am forced to say that VUSB is not supported for emulating other USB peripherals, as the hardware does not provide a means to meet the timing constraints in the context of an arduino sketch. Some people have gotten limited functionality to work. Those is the exception not the rule. The official digispark core contorted itself into pretzens and jumped through hoops (sacrificing ordinary functionality) in order to get VUSB functionality that was by most accounts quite flaky. This core has not "climbed into that rathole", and it should not be expected for VUSB in sketches to work under normal conditions. The difference between bootloader and sketch is that the bootloader - because that's all it does, operates in "polled mode" and is very careful to check the USB pins directly when needed. On the other hand, in a sketch you need to operate in "interrupt driven" mode. The necessary response time of the USB interrupt is very close to the ceiling of what the processors are capable of generating in response to an interrupt, and while the interrupt handler is written with great care in asm, it still has to fire immediately in order to make the timing constraints. That can only happen when there is no other interrupt running (which disables interrupts. Millis requires a periodic interrupt) nor are interrupts disabled for any other reason (but interrupts often have to be briefly disabled - for example during a write to any multibyte variable that is used by an interrupt), or a read-modify-write of a register that might be written by an interrupt. The pin output registers are such a register - and digitalWrite() and similar API calls perform R-M-W sequences and thus must briefly disable interrupts, and even that half dozen clock cycles is enough to miss the timing constraints.

##### If you're going attempt to make it work anyway
For those brave souls who wish to try, your best hope is:
* Millis should be disabled by the tools submenu.
* Digital I/O functions should be avoided. They must be replaced with fast digital I/O, which writes to the pins with the atomic CBI or SBI instruction.
* It may be most practical if USB is dynamically enabled and disabled, as the above constraints apply only while the device is connected and acting as a USB device (notice how a digispark disappears from device manager when a non-USB sketch is run).
* Some research will be required to realize that sort of intermittent USB. I believe it to be possible, but don't know how to implement it. What I do know is that if you wish to to intermittently run VUSB in interrupt driven mode, you must disable all interrupts *except* the USB one. My approach would be, after all interrupts are in place but this is not yet done, compile and export the hex (which gets you the assembly listing and memory map too). Open the memory map in your favorite text editor (see , and make note of all `_vector` functions with non-zero size. Cross-reference with the interrupt list at the bottom of this document, and write down a list of the interrupt routines present, and hence which interrupts the sketch uses, and cross-reference that with the datasheet to find the bit which enables said interrupt.
* The libraries you find may require modification as many of them were written for AVR-GCC 4.3 (not 7.3). Among other things, since AVR-GCC 5, a variable declared PROGMEM must also be declared const. If trying to compile gives errors about that, you've got a very old version of the library - try to find a newer library version to work from. There are also problems with mismatches between versions of VUSB and the libraries that make use of it (typically they supply their own copy of VUSB, all of which have forkitis where countless forks of an open source project have been made at different times, and are all mutually incompatible).

##### Background and hope for the future
The crux of the problem is that USB requires that the chip be able to respond within a very very short time (a number of clock cycles in the low double digits) to certain USB events. This is easy in polled mode micronucleus uses - it is a bootloader fit into the gaps between when USB has to be serviced, written with an intimate knowledge of USB. This barely scrapes by in interrupt mode if there are no other interrupts that fire and interrupts are never disabled when USB comms has to happen. Now, on the newer modern AVR devices, those have a two level interrupt controller - thus, all that should be necessary is to run at a sufficiently high clock speed (which those parts are eminantly capable of, especially if you take advantage of the overclocking headroom) - I don't know the current status of ports to that platform, I don't think there's a ready micronucleus bootloader available, let alone VUSB libraries - but my assessment is that an arduino-y environment that admits VUSB would be entirely viable on modern AVRs, with the sacrifice of a single port's worth of pin interrupts to service USB, and with that interrupt given LVL1 priority - However, I dont expect this is high on anyone's priority pile:

### LED_BUILTIN is on PB1
Both optiboot and micronucleus will try to blink it to indicate bootloader status.

## Features

### PLL Clock
The ATtiny x5-family parts have an on-chip PLL. This is clocked off the internal oscillator and nominally runs at 64 MHz when enabled. It is possible to clock the chip off 1/4th of the PLL clock speed, providing a 16MHz clock option without a crystal (this has the same accuracy problems as the internal oscillator driving it). Alternately, or in addition to using it to derive the system clock, Timer1 can be clocked off the PLL. See below. For use with USB libraries, a 16.5 MHz clock option is available; with the Micronucleus bootloader, a tuned value calculated from the USB clock is used, and this is the default clock option, otherwise, a heuristic is used to determine the tuned speed.

### Timer1 is a high speed timer
This means it can be clocked at 64 MHz from the on-chip PLL. In the past a menu option was provided to configure this. It never worked, and in any event is insufficient to do much of practical use with. It was eliminated for 2.0.0. Instead, see the [ATTinyCore library](../libraries/ATTinyCore/README.md)

### PWM configuration
The core configures PWM output as described below
* The inverted outputs are not used, as the hardware provides no means to use them without also using the corresponding non-inverted output.
* Timer0 is the bog standard timer0 that the vast majority of classic AVRs use. It is an 8 bit timer that can generatre PWM on PB0 and PB1 with independent duty cycles.
* Timer1 is a unique timer which is more complicated to configure but also more capable.
  * Inverted outputs are not supported by the core (you must configure it manually by register writes, and refrain from using digitalWrite() or analogWrite() on any associated pin as those may have undesired effects on the configuration of the timer.
  * The timer by default is clocked from the the main peripheral clock (F_CPU), but it can be configured to use the PLL instead using the included [ATTinyCore library](../libraries/ATTinyCore/README.md). The PLL clock is generated from the internal oscillator kicked up by a factor of 8 by the on-chip PLL, for a nominal frequency of 64 MHz, or 65 MHz with the 16.5 MHz tuned oscillator option (sometimes used by digispark-like configurations)

### PWM frequency
TC0 is always run in Fast PWM mode: We use TC0 for millis, and phase correct mode can't be used on the millis timer - you need to read the count to get micros, but that doesn't tell you the time in phase correct mode because you don't know if it's upcounting or downcounting in phase correct mode. TC1 has no phase correct mode. However because of the flexible prescaler, we can always keep the output from TC1 between 488 and 977 Hz, our target range.

| F_CPU  | F_PWM<sub>TC0</sub> | F_PWM<sub>TC1</sub>   | Notes                        |
|--------|---------------------|-----------------------|------------------------------|
| 1  MHz | 1/8/256=     488 Hz |  1/8/256=      488 Hz |                              |
| 2  MHz | 2/8/256=     977 Hz |  2/16/256=     488 Hz |                              |
| <4 MHz | x/8/256= 488 * x Hz |  x/16/256= 244 * x Hz |                              |
| 4  MHz | 4/8/256=    1960 Hz |  4/16/256=     977 Hz |                              |
| <8 MHz | x/64/256= 61 * x Hz |  x/32/256= 122 * x Hz |                              |
| 8  MHz | 8/64/256=    488 Hz |  8/64/256=     488 Hz |                              |
| >8 MHz | x/64/256= 61 * x Hz |  x/64/256=  61 * x Hz |                              |
| 12 MHz | 12/64/256=   735 Hz | 12/64/256=     735 Hz |                              |
| 16 MHz | 16/64/256=   977 Hz | 16/64/256=     977 Hz |                              |
|>16 MHz | x/64/256= 61 * x Hz | x/128/256=  31 * x Hz |                              |
| 20 MHz | 20/64/256=  1220 Hz | 20/128/256=    610 Hz |                              |

Where speeds above or below a certain speed are specified, it's implied that the other end of the range is the next marked value. So >16 in that table is for 16-20 MHz clocks. The formula is given as a constant times x where x is expressed as MHz (the division above gets the time in megahertz - in the interest of readability I did not include the MHz to Hz conversion - I'm sure you all know how to multiply by a million)
For more information see the [Changing PWM Frequency](Ref_ChangePWMFreq.md) reference.

### I2C support
There is no hardware I2C peripheral. I2C functionality can be achieved with the hardware USI. This is handled transparently via the special version of the Wire library included with this core. **You must have external pullup resistors installed** in order for I2C functionality to work at all. We only support use of the builtin universal Wire.h library. If you try to use other libraries and encounter issues, please contact the author or maintainer of that library - there are too many of these poorly written libraries for us to provide technical support for.

### SPI Support
There is no hardware SPI peripheral. SPI functionality can be achieved with the hardware USI. This should be handled transparently via the SPI library. Take care to note that the USI does not have MISO/MOSI, it has DI/DO; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common, and the only mode that the SPI library has ever supported). As with I2C, we only support SPI through the included universal SPI library, not through any other libraries that may exist, and can provide no support for third party SPI libraries.

### UART (Serial) Support
There is no hardware UART. If running off the internal oscillator, you may need to calibrate it to get the speed close enough to the correct speed for UART communication to work. The core incorporates a built-in software serial named Serial - this uses the analog comparator pins, in order to use the Analog Comparator's interrupt, so that it doesn't conflict with libraries and applications that require PCINTs.  TX is AIN0, RX is AIN1. Although it is named Serial, it is still a software implementation, so you cannot send or receive at the same time. The SoftwareSerial library may be used; if it is used at the same time as the built-in software Serial, only one of them can send *or* receive at a time (if you need to be able to use both at the same time, or send and receive at the same time, you must use a device with a hardware UART). While one should not attempt to particularly high baud rates out of the software serial port, [there is also a minimum baud rate as well](Ref_TinySoftSerial.md)

Though TX defaults to AIN0, it can be moved to any pin using Serial.setTxBit(n) where n is the number in the pin name using Pxn notation (in this case, also the arduino pin number) (2.0.0+ only - was broken in earlier versions)..

To disable the RX channel (to use only TX), select "TX only" from the Builtin SoftSerial tools menu. To disable the TX channel, simply don't print anything to it, and set it to the desired pinMode after Serial.begin()

### Tone Support
Tone() uses Timer1. If the high speed functionality of Timer1 has been enabled (see link above), tone() will not produce the expected frequencies, but rather ones 2 or 4 times higher. For best results, use pin 1 or 4 for tone - this will use Timer1's output compare unit to generate the tone, rather than generating an interrupt to toggle the pin. In this way, "tones" can be generated up into the MHz range.  If using SoftwareSerial or the builtin software serial "Serial", tone() will work on pin 1 or 4 while the software serial is active but not on any other pins. Tone will disable PWM on pins 1 and 4.

### Servo Support
Although the timers are quite different, and historically there have been issues with the Servo library, we include a builtin Servo library that supports the Tiny x5 series. As always, while a software serial port is receiving or transmitting, the servo signal will glitch (this includes the builtin software serial "Serial).

### Servo and Tone break PB4 (and possibly PB1 for PWM)
The servo library and the tone function require full control of timer1.  This has two unfortunate consequences:
* There is no PWM available on the Timer1 pins (PB4 and - by default - PB1) if either tone() is outputting a tone, or the servo library is used.
* The Servo library cannot be used at the same time as tone

I realize that this is sometimes painful, but you're using an 8-pin tinyAVR that's over a decade old! Really, what do you expect? (this is not an issue on modern tinyAVRs (the ones supported by megaTinyCore))

## ADC Features
The ATtiny85 has a differential ADC, unlike even some ATmega parts, but like many other ATtiny devices. Gain of 1x or 20x is available, and two differential pairs are available. The ADC supports both bipolar mode (-512 to 511) and unipolar mode (0-1023) when taking differential measurements; you can set this using `setADCBipolarMode(true or false)`. On many AVR devices with a differential ADC, only bipolar mode is available. All of the channels can have the positive and negative inputs swapped; they advise taking a measurement in bipolar mode, and then swapping the direction if needed and switching to unipolar mode to double the effective resoluition.

### ADC Reference options
The ATtiny85 has both the 2.56 and 1.1V reference options, a rarity among the classic tinyAVR parts. It even supports an external reference, not that you can usually spare the pins to use one.

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
| ADC_GROUND      | Measures ground (offset cal?)          |
| ADC_TEMPERATURE | Internal temperature sensor            |

### Differential Channels
The ADC on the x5-series can act as a differential ADC with selectable 1x or 20x gain. It can operate in either unipolar or bipolar mode, and the polarity of the two differential pairs can be inverted in order to maximize the resolution available in unipolar mode (they envision users making a measurement in bipolar mode, reversing the input polarity if needed before taking a more accurate measurement in unipolar mode.) ATTinyCore wraps the IPR bit into the name of the differential ADC channel. Note the organization of the channels - there are two differential pairs A0/A1, and A2/A3 - but there is no way to take a differential measurement between A0 and A2 or A3 for example.

| Positive   | Negative   |Gain | Name            | Notes            |
|------------|------------|-----|-----------------|------------------|
| ADC0 (PB5) | ADC0 (PB5) | 1x  | DIFF_A0_A0_1X   | For offset cal.  |
| ADC0 (PB5) | ADC0 (PB5) | 20x | DIFF_A0_A0_20X  | For offset cal.  |
| ADC0 (PB5) | ADC1 (PB2) | 1x  | DIFF_A0_A1_1X   |                  |
| ADC0 (PB5) | ADC1 (PB2) | 20x | DIFF_A0_A1_20X  |                  |
| ADC1 (PB2) | ADC0 (PB5) | 1x  | DIFF_A1_A0_1X   | Input Reversed   |
| ADC1 (PB2) | ADC0 (PB5) | 20x | DIFF_A1_A0_20X  | Input Reversed   |
| ADC2 (PB4) | ADC2 (PB4) | 1x  | DIFF_A2_A2_1X   | For offset cal.  |
| ADC2 (PB4) | ADC2 (PB4) | 20x | DIFF_A2_A2_20X  | For offset cal.  |
| ADC2 (PB4) | ADC3 (PB3) | 1x  | DIFF_A2_A3_1X   |                  |
| ADC2 (PB4) | ADC3 (PB3) | 20x | DIFF_A2_A3_20X  |                  |
| ADC3 (PB3) | ADC2 (PB4) | 1x  | DIFF_A3_A2_1X   | Input Reversed   |
| ADC3 (PB3) | ADC2 (PB4) | 20x | DIFF_A3_A2_20X  | Input Reversed   |

### Temperature Measurement
To measure the temperature, select the 1.1v internal voltage reference, and analogRead(ADC_TEMPERATURE); This value changes by approximately 1 LSB per degree C. This requires calibration on a per-chip basis to translate to an actual temperature, as the offset is not tightly controlled - take the measurement at a known temperature (we recommend 25C - though it should be close to the nominal operating temperature, since the closer to the single point calibration temperature the measured temperature is, the more accurate that calibration will be without doing a more complicated two-point calibration (which would also give an approximate value for the slope)) and store it in EEPROM (make sure that `EESAVE` fuse is set first, otherwise it will be lost when new code is uploaded via ISP) if programming via ISP, or at the end of the flash if programming via a bootloader (same area where oscillator tuning values are stored). See the section below for the recommended locations for these.

### Tuning Constant Locations
These are the recommended locations to store tuning constants. In the case of OSCCAL, they are what are checked during startup when a tuned configuration is selected. They are not otherwiseused by the core.

ISP programming: Make sure to have EESAVE fuse set, stored in EEPROM

Optiboot used: Saved between end of bootloader and end of flash.

| Tuning Constant        | Location EEPROM | Location Flash |
|------------------------|-----------------|----------------|
| Temperature Offset     | E2END - 4       | FLASHEND - 7   |
| Temperature Slope      | E2END - 3       | FLASHEND - 6   |
| Unspecified            | N/A             | FLASHEND - 5   |
| Tuned OSCCAL 12 MHz    | E2END   2       | FLASHEND - 4   |
| Tuned OSCCAL 8.25 MHz  | E2END - 1       | FLASHEND - 3   |
| Tuned OSCCAL 8 MHz     | E2END           | FLASHEND - 2   |
| Bootloader Signature 1 | Not Used        | FLASHEND - 1   |
| Bootloader Signature 2 | Not Used        | FLASHEND       |

Mironucleus used: Micronucleus boards store a tuning value to the application section, but a separate sketch could also use a different means of calibration and store a value in the flash. The recommended locationsare shown below.

| Tuning Constant        | Location Flash         |
|------------------------|------------------------|
| Tuned OSCCAL 8.25 MHz  | BOOTLOADER_ADDRESS - 4 |
| Temperature Offset     | FLASHEND - 3           |
| Temperature Slope      | FLASHEND - 2           |
| Tuned OSCCAL 8.25 MHz  | FLASHEND - 1           |
| Tuned OSCCAL 8 MHz     | FLASHEND               |

## Purchasing ATtiny85 Boards
As the ATtiny85 is available in an easy-to-solder through-hole DIP package, a board can be easily made by just soldering the part into prototyping board.
I (Spence Konde) sell a specialized prototyping board that combines an ISP header with prototyping space and outlines to fit common SMD parts.
* [ATtiny85 prototyping board](https://www.tindie.com/products/drazzy/attiny85-project-board/)
* Micronucleus boards, and socketed boards for chips bootloaded with Micronucleus are readily available all over the internet, very cheaply, in several models. Search for things like "Digispark ATtiny85", "ATtiny85 USB" and so on.

### Do not buy bare ATtiny85 parts on aliexpress/ebay
While assembled boards, even socketed ones, have not been reported as impacted, there are a large number of reports of counterfeit bare chips being sold on international marketplace sites. In some cases, the parts are ATtiny13 chips, with the markings ground off and replaced with ones identifying it as an ATtiny85.

### Despite having only 8 pins, there are 5 different packages used x5-family parts
The manufacturer part numbers (ordering codes) indicate, in the final non-decimal characters, several properties of the part.
* If it ends in R, the part is being sold in tape and reel form, as opposed to tubes. This is not marked on the chip, ever. This is very important if you hope to feed it into your pick and place machine (some machines can deal with trays, some can even deal with SMT parts in tubes (utilizing vibration and a slight incline to nudge the parts down the tube and out, where it can pick them up), but some cannot, and all have constraints on these (either limited number of parts like that, or the physical impact of occupying a large amount of space that could be full of your boards with a tray of parts instead). If you do not have a pick and place machine, or are not planning to use it with these parts, the R just means that instead of a reusable tube, you will be removing parts from non-reusable plastic tape.
* After removing the R if present, what is left indicates three things: The package, the temperature range, and what treatment was applied to the pins. The termination treatment and temp range use the last letter (after removing the R): The Atmel lettering scheme may have a different letter to indicate NiPdAu and extended temperature ranges - these letters saw use throughout the non-automotive AVR parts (those used a different set of letters). As far as I can tell, the primary criteria for selecting the letters was to maximize the difficulty of clandestinly remarking the chips to convincingly appear to be a different (higher, of course) speed grade convincingly without grinding the markings off entirely like they do to tiny13's. Automotive parts used different letters, but they had the same properties w/regards to resistance to modification. You also couldn't change whether a chip was marked as an automotive temp grade without both erasing and adding lines. I think that in order to erase lines without their work being plainly obvious, they have no choice but to grind off the markings entirely and reinscribe, while it's plausible that someone could add lines quite easily.
  * U indicates -40 through 85 C operating range, and matte tin.
  * H indicates the same temperatures, but NiPdAu treatment (likely analogous to the PCB ENEPIG process)
  * N indicates -40 through 105 C operating range, and matte tin.
  * F indicates -40 through 125 C operating range, and matte tin.
* The first 1 or 2 characters indicate what type of package it is in:
  * P = PDIP-8
  * S = Wide SOIC-8 - Was more common in the past, presumably when the same parts were made on older process technology and hence with a physically larger die.
  * SS = Narrow SOIC-8/SOP-8 - By far the most commonly seen today
  * X = TSSOP-8 (not available for tiny85, only smaller parts - presumably for die size considerations - this package is actually smaller than the QFN in one dimension, which was likely the determining factor of whether they could offer the t85 in that package - While surely it would be no problem now, few companies like to revisit old designs to add package variants, and Microchip is no exception. I know of [only one case](ATtiny_x7.md) where this happened for a classic AVR post Microchip acquisition.
  * M = 4mm x 4mm QFN-20 0.5mm pitch. Only 8 of the contacts are electrically connected, the other 12 are just for decoration and to promote successful soldering via self-centering. If you ever, for some crazy reason, end up using these, and after-reflow inspection reveals visible solder bridge(s), only those that involve the 8 used pins must be corrected.

## Interrupt Vectors
This table lists all of the interrupt vectors available on the ATtiny x8-family, as well as the name you refer to them as when using the `ISR()` macro. Be aware that a non-existent vector is just a "warning" not an "error" (for example, if you misspell a vector name) - however, when that interrupt is triggered, the device will (at best) immediately reset (and not cleanly - I refer to this as a "dirty reset") The catastrophic nature of the failure often makes debugging challenging.

Note: The shown addresses below are "byte addressed" as that has proven more readily recognizable. The vector number is the number you are shown in the event of a duplicate vector error, as well as the interrupt priority (lower number = higher priority), if, for example, several interrupt flags are set while interrupts are disabled, the lowest numbered one would run first. Notice that INT0 is (as always) the highest priority interrupt. All of the parts  in this family are 8k or less flash, so they do not need to use 4-byte vectors.

|vect_num | Address| Vector Name        | Interrupt Definition                |
|---------|--------|--------------------|-------------------------------------|
|       0 | 0x0000 | RESET_vect         | Not an interrupt - this is a jump to the start of your code.  |
|       1 | 0x0002 | INT0_vect          | External Interrupt Request 0        |
|       2 | 0x0004 | PCINT0_vect        | Pin Change Interrupt 0              |
|       3 | 0x0006 | TIMER1_COMPA_vect  | Timer/Counter1 Compare Match A      |
|       4 | 0x0008 | TIMER1_OVF_vect    | Timer/Counter1 Overflow             |
|       5 | 0x000A | TIMER0_OVF_vect    | Timer/Counter0 Overflow             |
|       6 | 0x000C | EE_RDY_vect        | EEPROM Ready                        |
|       7 | 0x000E | ANA_COMP_vect      | Analog Comparator                   |
|       8 | 0x0000 | ADC_vect           | ADC Conversion Complete             |
|       9 | 0x0002 | TIMER1_COMPB_vect  | Timer/Counter1 Compare Match B      |
|      10 | 0x0004 | TIMER0_COMPA_vect  | Timer/Counter0 Compare Match A      |
|      11 | 0x0006 | TIMER0_COMPB_vect  | Timer/Counter0 Compare Match B      |
|      12 | 0x0008 | WDT_vect           | Watchdog Time-out (Interrupt Mode)  |
|      13 | 0x000A | USI_START_vect     | USI START                           |
|      14 | 0x000C | USI_OVF_vect       | USI Overflow                        |
