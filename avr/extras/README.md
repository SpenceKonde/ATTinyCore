# Additional more detailed sections of documentation for ATTinyCore

* [ATtiny441, 841](ATtiny_x41.md) (With or without Optiboot or Micronucleus bootloader)
* [ATtiny1634](ATtiny_1634.md)  (With or without Optiboot bootloader)
* [ATtiny87, 167](ATtiny_x7.md) (with or without Optiboot or Micronucleus bootloader)
* [ATtiny25, 45, 85](ATtiny_x5.md) (With or without Optiboot or Micronucleus bootloader)
* [ATtiny24, 44, 84](ATtiny_x4.md) (With or without Optiboot or Micronucleus bootloader)
* [ATtiny261, 461, 861](ATtiny_x61.md) (With or without Optiboot bootloader)
* [ATtiny48, 88](ATtiny_x8.md) (With or without Optiboot or Micronucleus bootloader)
* [ATtiny828](ATtiny_828.md) (With or without Optiboot bootloader)
* [ATtiny2313, 4313](ATtiny_x313.md) (no bootloader)
* [ATtiny43](ATtiny_43.md) (no bootloader)

Variants of these are also supported (such as the ATtiny1634R, ATtiny2313A or ATtiny85V)

## Quick Comparison of supported parts, as a table
| Part Family     |   x4-series |  x41-series |   x5-series | ATtiny26 |    x61-series |  x7-series | x8-series | x313-series | ATtiny1634 | ATtiny828 | ATtiny43 |
|-----------------|-------------|-------------|-------------|----------|---------------|------------|-----------|-------------|------------|-----------|----------|
| Flash Size      |    2k/4k/8k |       4k/8k |    2k/4k/8k |     2048 |      2k/4k/8k |     8k/16k |     4k/8k |       2k/4k |      16384 |      8192 |     4096 |
| EEPROM Size     | 128/256/512 |     256/512 | 128/256/512 |      128 |   128/256/512 |        512 |   256/512 |     128/256 |        256 |       512 |       64 |
| RAM size        | 128/256/512 |     256/512 | 128/256/512 |      128 |   128/256/512 |        512 |   256/512 |     128/256 |       1024 |       512 |      256 |
| Internal 16 MHz |          No |  via tuning |    Yes, PLL | Yes, PLL |      Yes, PLL |         No |        No |          No |         No |        No |       No |
| Ext. Crystal    |         Yes |         Yes |         Yes |      Yes |           Yes |        Yes |        No |         Yes |        Yes |        No |       No |
| Clock Switching |          No |         Yes |          No |       No |            No | yes, buggy |        No |          No |        Yes |       Yes |       No |
| HV programming  |        HVSP |        HVSP |        HVSP | parallel |      parallel |   parallel |  parallel |    parallel |   parallel |  parallel | parallel |
| I/O pins        |          12 |          12 |           6 |       16 |            16 |         16 |        28 |          18 |         18 |        28 |       16 |
| Optiboot        |         Yes |         Yes |         Yes |       No |           Yes |        Yes |       Yes |          No |        Yes |       Yes |       No |
| Micronucleus    |         Yes |         Yes |         Yes |       No |           Yes |        Yes |       Yes |          No |        Yes |        No |       No |
| AREF Pin        |         Yes |         Yes |         Yes |          |           Yes |        Yes |        No |          No |        Yes |        No |       No |
| PWM pins        |           4 |       6 (8) |           3 |        2 |             3 |       3(9) |         2 |           4 |          4 |     4 (8) |        4 |
| Internal Refs   | 1V1         | 1V1,2V2,4V1 |   1V1, 2V56 |     2V56 |   1V1, 2V56   |   1V1,2V56 |       1V1 |         1V1 |        1V1 |       1V1 |      1V1 |
| Analog Pins     |           8 |          12 |           4 |       11 |            11 |         11 |    6 or 8 |        none |         12 |        28 |        4 |
| Diff. ADC pairs |          12 |  "46"* (18) |           2 |        8 |     "16" (10) |          8 |      none |        none |       none |      none |     none |
| Diff. ADC gain  |     1x, 20x | 1x,20x,100x |     1x, 20x |  1x, 20x | 1, 8, 20, 32x |    8x, 20x |      none |        none |       none |      none |     none |
| Gimmick/trick   | Just pretty | 3 timer,ADC |     8 pins! |  Ancient | ADC, BLDC PWM | Automotive |   SPI+TWI |  Old, USART |   2x USART | Most bugs |Int. Boost|
| (contd)         | normal.     | 2xUSART     |   PLL clock | not much | PLL clock     | Lin, SPI   | like 328p |     No ADC! |    1k SRAM | ADC gimped|runs at 1.5v or less|

`* Number in quotes is from the Atmel marketing material which are inconsistent with the counting methods used for other parts and the most basic of mathematcal concepts. Ex: on x41, for differential pairs, they counted every pair of pins twice (since you can reverse them) plus the 10 channels where the same input is used as both positive and negative, for offset calibration and that added up to 46. But historically, they only counted unique pairs, and didn't count channels that would read 0 except for offset error. That gets 18 pairs. Hence: "46" (18)`

I/O pins *includes* reset in the count; reset can only be used as GPIO if you disable reset (in which case you need to have an HV programmer to reprogram the part).

HV programming is required if you disable eiher reset or ISP progrmming. HVSP is relatively easy and there are many plans for "fuse doctors" online that will fix the fuses. HVPP (parallel) is much more complicated, requiring a minimum of 18 wires connecteed to the target (forget about in system reprogramming). As HVPP programmers are not readily available to hobby programmers, it would cause more problems than it solved to allow the tools menu to disable reset there, so we don't. That is not the case for HVSP parts (the 85, 84, and 841 and their smaller versions)

PWM pins where second number is shown in (parenthesis), the first number is the number of simultaneous, independent duty cycles that can be generated, and the one in parenethesis is the number of pins on which those can be output. See the part specific documentation for details, as the implementation and core integration (if any) varies.

Clock source switching is NEVER supported by ATTinyCore. The x7-series is impacted by scary errata with a very specific workaround.

The "Gimmick" section lists the most prominent unique features of this part. These are what set it apart from other devives and may force you to go with that part even though you'd rather a different one. If you need to maximize battery life from a single alkaline battery cell, the tiny43 is your only choice without designing a very low quiescent current boost converter yourself.

The 828 deserves a bit of explanation - it was one of the last parts that was released as classic AVR, and I think they set a hard deadline. The part was going to have a super snazzy differential ADC like the 841. But the silicon came back, and in addition to the bugs listed in the errata (one of which is quite nasty) the differential functionality of the ADC was hosed. Management woldn't approve a respin. And so it was released in the sorry half done state that it is. I suspect that some digging around in that area of registers would find a differential ADC that didn't work very well hiding.

## Non-supported parts
* [tinyAVR 0/1/2-series](https://github.com/SpenceKonde/megaTinyCore/) Modern tinyAVR (with 0, 1, or 2 as next-to-last digit) are supported by my megaTinyCore instead. They are totally different in every way except the "t-word" in the name, and the fact that they're great parts and work well with Arduino.
* [ATtiny13/13A](https://github.com/MCUdude/MicroCore/) are supported by MicroCore by @MCUdude
* ATtiny26 are not supported by any Arduino core. They are the obsolete predecessor to the '261, which itself is ancient). I will accept a PR to add support but will not use my own limited development time for such old and uninspiring parts.
* ATtiny 4/5/10/11 and any other "AVRrc" (reduced core) parts. [Try this core](https://github.com/technoblogy/attiny10core)

## Quick Gotcha list - having trouble? READ THESE

**Windows users must install Micronucleus drivers manually**
If you want to use Micronucleus (VUSB) boards on Windows, you must manually install the drivers - Arduino does not run "post-install" tasks for third party libraries (though I am told they recognized how nonsensical this is - a malicious core could still run whatever commands it wanted to by telling the IDE that was how to compile sketches for these boards - and will be changing this in the future. Note also that the 1.5.0 release does not include working micronucleus upload tool for Windows, this is resolved in 1.5.2.

During the install process it will print the path of a post_install.bat that it skipped running. Running that will install the drivers - it's easiest if you copy/paste it, as after installation the drivers will be located in `C:\Users\YourUserName\AppData\Local\Arduino15\packages\ATTinyCore\tools\micronucleus\2.5-azd1\`  Or they can be downloaded from: [https://azduino.com/bin/micronucleus/Drivers-Digistump(win).zip](https://azduino.com/bin/micronucleus/Drivers-Digistump%28win%29.zip). Unzip, run the installation batch file.


**avrdude: error: could not find USB device with vid=0x16c0 pid=0x5dc vendor=`www.fischl.de` product='USBasp'**
Vaguely recent versions of AVRdude appear to have dropped a few versions of the firmware. If this is the first time you'd installed something that triggered an update here, you may have problems with AVRdude. The solution, thankfully, is straightforward..
.
1. Download [Zadig from http://zadig.akeo.ie](http://zadig.akeo.ie)
2. Plug in USBasp
3. Start zadig
4. Options > List all devices
5. Select USBasp from the drop down menu
6. Select libusbK(v3.0.7.0) driver
7. Click Install

**This core includes part specific documentation - click the links above for your family of chips and READ IT** These describe issues and "gotchas" specific to certain chips. Be sure to review this documentation!

**problems dynamically linking libusb-0.1 on linux** can occur if Arduino was installed through the Snap package manager. The Arduino IDE should always be installed from the tarball available from [http://arduino.cc](http://arduino.cc), never from a package manager.

**There is a bug in the IDE that causes compilation errors with some combinations of boards and submenu options** on 1.8.5 and earlier on Windows. The symptom of this is an error ending in something like C:\Users\yourusername\AppData\Local\Temp\arduino_build_131884/..\arduino_cache_186227\core\core_ATTinyCore_avr_attinyx4_LTO_enable,chip_84,clock_8internal,eesave_aenable,bod_disable,pinmapping_anew,millis_enabled,neopixelport_porta_1bc5d2d7fe299bbd4d4a668366e76c74.a: No such file or directory - this is caused by the way it includes all options in the name of the file; where there are many menu options, this will exceed the maximum file name length under windows. This issue impacts many parts on ATTinyCore 1.3.0 and later, and the 841/441 on ATTinyCore 1.2.2 and later. To fix this issue without downgrading ATTinyCore, update to 1.8.6 or later of the IDE - 1.8.13 is STRONGLY recommended

**Windows store version sometimes experiences strange issues**. The windows store issues are difficult to reproduce on other systems, and no reliable solutions to them are currently known. We recommend using the .zip package or standard installer version of the IDE, not the Windows Store version.

**On IDEs prior to 1.8.13, you need to select the correct version of the programmer. In 1.8.13 only these programmers are shown. As of 1.4.2, the (ATTinyCore) note is removed, as 1.8.13 has had excellent uptake (because it lacks new bugs and greatly improves UX)**

**When using a chip for the first time, or after changing the clock speed, EESAVE or BOD settings, you must do "burn bootloader" to set the fuses, even if you are not using the chip with a bootloader**

**Problems programming some parts for first time, especially ATtiny841/441** These parts are less forgiving of the SCK clock rate. I'm not sure why USBAsp doesn't seem to be working anymore (it used to, and I haven't changed anything), looking into options there. Arduino as ISP or USBTinyISP SLOW will program without issue.

**Chips sold as "ATtiny85" with wrong signature** - enable verbose upload output, and it will tell you what sig it actually saw. **0x1e9005** means you got scammed (all 0's or all F's is wiring problem, or bricked chip from choosing clock source that isn't present). Apparently one or more foreign sellers have been remarking the much cheaper ATtiny12 as an 85 and ripping people off on ebay/etc.

**free(): invalid next size (normal) error** This error is due to a bug in [AVRdude](https://savannah.nongnu.org/bugs/?48776) - and it's a spurious error, as when it is displayed, the programming operation has actually completed successfully (you can see for yourself by enabling verbose upload, and noting the successful write before this error is shown. It is unknown under what conditions this error appears, though it has been recorded on a USBTinyISP on Linux when bootloading an attiny88 with optiboot.

**When using analogRead(), use the A# constant to refer to the pin, not the digital pin number.** Analog channel number (see table in datasheet entry for ADMUX register) can also be used - unlike the official core, you can use analogRead() with the differential ADC channels (for example).

**When using I2C on anything other than the ATtiny48/88** you **must** use an I2C pullup resistor on SCL and SDA (if there isn't already one on the I2C device you're working with - many breakout boards include them). 4.7k or 10k is a good default value. On parts with real hardware I2C, the internal pullups are used, and this is sometimes good enough to work without external pullups; this is not the case for devices without hardware I2C (all devices supported by this core except 48/88) - the internal pullups can't be used here, so you must use external ones. **That said, for maximum reliability, you should always use external pullups, even on the t48/88**, as the internal pullups are not as strong as the specification requires.

**You cannot use the Pxn notation (ie, PB2, PA1, etc) to refer to pins** - these are defined by the compiler-supplied headers, and not to what an arduino user would expect. To refer to pins by port and bit, use PIN_Pxn (ex, PIN_PB2); these are #defined to the Arduino pin number for the pin in question, and can be used wherever digital pin numbers can be used. We recommend this method of referring to pins, especially on parts with multiple pinmapping options

**All ATtiny chips (as well as the vast majority of digital integrated circuits) require a 0.1uF ceramic capacitor** between Vcc and Gnd for decoupling; this should be located as close to the chip as possible (minimize length of wires to cap). Devices with multiple Vcc pins, or an AVcc pin, should use a cap on those pins too. Do not be fooled by poorly written tutorials or guides that omit these. Yes, I know that in some cases (ex, the x5-family) the datasheet doesn't mention these - but other users as well as myself have had problems when it was omitted on a t85.

**For low power applications, before entering sleep, remember to turn off the ADC (ADCSRA&=(~(1<<ADEN))) - otherwise it will waste ~270uA**

**When in power down sleep mode, the clock is stopped - using a slower clock speed does not save power while sleeping** I am asked very frequently about using very low clock speeds in low-power applications. Almost invariably, upon further interrogation, the user reveals that they plan to have the part in power-down sleep mode almost all the time. In this case, there is often little to be gained from running at a lower clock speed, since it only effects power use in the brief moments that the chip is awake - even more so because these moments may well be shorter when running at a higher clock speed. Clock speeds below 1MHz can present problems while programming due to the programmer using an SCK clock speed too fast for the target chip running at such a low system clock speed.

**When using the WDT as a reset source and NOT using a bootloader** remember that after reset the WDT will be enabled with minimum timeout. The very first thing your application must do upon restart is reset the WDT (`wdt_reset()`), clear WDRF flag in MCUSR (`MCUSR&=~(1<<WDRF)`) and then turn off or configure the WDT for your desired settings. If using the Optiboot bootloader, this is already done for you by the bootloader.

## ATTinyCore Features

### Optiboot Support - serial bootloader for ATtiny441/841, 44/84, 45/85, 461/861, 48/88, 828, 1634, 87, 167 only)

The Optiboot bootloader is included for the ATtiny441, 841, 44, 84, 45, 85, 461, 861, 48, 88, 1634, 828, 87 and 167. For boards with a hardware UART, this runs at 57600 baud at 8 MHz and slower, and  115200 baud above that; for boards without hardware UART, it runs at 19200 baud (on these boards TX/RX pins are the same as the builtin software serial, the ones marked AIN0 (TX) and AIN1(RX)). On boards with hardware serial, the bootloader uses UART0 or the LIN/UART as appropriate (bootloaders that use UART1 for devices that have a second UART are included, prefixed with "ser1" - you must flash them manually or modify boards.txt if you wish to use them). Once the bootloader is programmed, the target can be programmed over serial; the bootloader will run after reset, just like on a normal Arduino. Bootloaders are not included for the 2k-flash versions of these parts - more than a quarter of the available flash would go to the bootloader, and the Arduino abstraction is already severely constrained with just 2k of flash. For these parts, we recommend ISP programming. Clock speeds lower than 1MHz are not supported because the resulting baud rates would be so low that it some serial adapters start having problems talking to them. At 1MHz, uploads are painfully slow. We recommend using the bootloader only at 8MHz or higher. If you are trying to save power, it is usually better to run at 8MHz and put the chip to sleep for most of the time.

The ATtiny441/841, ATtiny1634, ATtiny44/84, ATtiny45/85, ATtiny461/861, ATtiny48/88 and the ATtiny x7-family do not have hardware bootloader support. To make the bootloader work, the "Virtual Boot" functionality of Optiboot is used. Because of this, another vector is used to point to point to the start of the applications - this interrupt cannot be used by the application - under the hood, the bootloader rewrites the reset and "save" interrupt vectors, pointing the save vector at the start of the program (where the reset vector would have pointed), and the reset vector to the bootloader (as there is no BOOTRST fuse). Up until version 1.2.0 of this core, the WDT vector was used for this purpose. In 1.2.0 and later, the EE_RDY vector (which is not used by anything in Arduino-land - the EEPROM library uses a busy-wait) is used instead. **If the bootloader was burned with 1.1.5 or earlier of this core, the WDT cannot be used to generate an interrupt** (WDT as reset source is fine) - re-burning bootloader with 1.2.0 or later will resolve this.

#### A warning about Virtual Boot
Virtual boot relies on rewriting the vector table, such that the RESET vector points to the bootloader. This presents a potential issue: If the bootloader starts to write the first page, but then - for some reason - fails (such as a poorly timed reset right after the programming process begins), the page containing the reset vectors will be erased but not rewritten, with the result being that both the sketch and bootloader are hosed resulting in ISP programming being required to revive the chip. See #398

See the [Programming Guide](Ref_Programming.md) for more information on programming parts using Optiboot.

### Micronucleus - VUSB bootloader for 841, 167, 85, 88 and 84/84a
It's finally here! As of 1.4.0, we now offer Micronucleus (aka Digispark) support for some of the more popular boards for these bootloaders. This allows sketches to be uploaded directly via USB, which many users find highly convenient. This comes at a cost in terms of flash - they typically use around 1.5k of flash, and they sometimes have problems connecting to specific USB ports. These boards are available from various vendors; see the part-specific documentation pages for more information on the implementation used on specific parts. For more information on using Micronucleus, see the [usage documentation](Ref_Micronucleus.md).

### Changing the ATtiny clock speed and other settings

Changing the ATtiny clock speed, B.O.D. settings etc, is easy. When an ATTinyCore board is selected from the Tools -> Board menu, there will appear extra submenus under Tools menu where we can set several ATtiny properties:

* Tools > Save EEPROM: (Boards without bootloader only - controls whether EEPROM is erased during a chip erase cycle)
* Tools > Timer 1 clock: (ATtiny25/45/85 only - allows Timer1 to be clocked off the PLL for higher frequency PWM)
* Tools > LTO: (Link Time Optimization makes sketches smaller, but requires AVR boards 1.6.11 or later)
* Tools > B.O.D Level: (trigger voltage for Brown Out Detection - below this voltage, chip will be held in reset)
* Tools > Chip: (Select the part being used )
* Tools > Clock:  (Select the desired clock speed)
* Tools > B.O.D. Mode (active): (441, 841, 1634, 828 only - see B. O. D. section below)
* Tools > B.O.D. Mode (sleep): (441, 841, 1634, 828 only - see B. O. D. section below)

After changing the clock source, BOD settings, or whether to save EEPROM on chip erase), you must do "Burn Bootloader" with an ISP programmer. See [Programming Guide](Ref_Programming.md)

### Supported clock speeds
Supported clock speeds are shown in the menus in descending order of usefulness, ie, the popular clock speeds/sources are at the top, and the weird ones are at the bottom. See the notes for caveats specific to certain clock speeds.

Internal:
* 8 MHz
* 1 MHz
* 16 MHz (PLL clock, x5, x61 only)
* 16 MHz ‡ (aggressively configured 441/841 only)
* 4 MHz*** (except on x313, starts up at 1MHz and immediately switches to 4MHz before setup() is run)
* 16.5MHz † ! (PLL clock, tweaked, x5, x61 Micronucleus only, for USB support)
* 12.8MHz † ‡ (Internal, tweaked hardcore, Micronucleus only, for USB support)
* 0.5 MHz** ‼ (x313 only)
* 512 kHz** ‼ (ULP - x41 only)
* 256 kHz** ‼ (ULP - x41 only)
* 128 kHz** ‼ (watchdog or ULP, all except 1643, 828)
* 64 kHz** ‼ (ULP - x41 only)
* 32 kHz** ‼ (ULP - 1634, 828, x41 only)

External crystal (all except 828, 43 and x8-family):
* 20 MHz !
* 18.432 MHz* !
* 16 MHz
* 14.7456 MHz* !
* 12 MHz !
* 11.0592 MHz* !
* 9.216 MHz* !
* 8 MHz
* 7.3728 MHz* !
* 6 MHz !
* 4 MHz


All available clock options for the selected processor will be shown in the Tools -> Clock menu.

`*` These weird crystals are "UART frequencies" - these divide evenly to common baud rates, so you can get a perfect match if this is required for your application - typical UART use cases do not require running at one of these UART frequencies, the usual caveats about software serial and baud rate accuracy on classic (pre-2016) AVR designs notwithstanding. These crystal options are not available for chips without a UART in versions of ATTinyCore older than 1.1.5, and are never recommended for parts without a hardware serial port.

`**` These options are slow enough that many ISP programmers may not be able to program them. Depending on the ISP programmer (and in some cases the firmware on it), there may be a setting or jumper to slow the SCK frequency down for programming these parts, or it may automatically figure it out. The SCK frequency must be less than 1/6th of the system clock for ISP programming. Before using a such a low clock speed, consider whether you might be able to get lower power consumption by running at a higher base clock while staying in sleep most of the time - this results in fewer programming headaches, and in many (but not all) use cases results in comparable or lower power consumption.

`***` The 4MHz internal option is useful if you are running near the minimum voltage - the lowest voltage for most of these parts is 1.8v, and at that voltage, they are only rated for 4MHz maximum. This starts up at 1 MHz and then switches to 4 MHz. Hence, bootloader performance is very slow, as it doesn't (yet) do this itself.

`†` These speeds support USB when used with the internal clock. Available on Micronucleus boards only.

`‡` Aggressively tuned internal oscillators have to be slowed down when writing to EEPROM, and then sped back up. We attempt to correct millis timekeeping for this, but you may still see timing glitches. Please report any hangs or other bad behavior you observe with these, as this is a very new feature, and more aggressive measures to prevent CPU glitches may be required.

`!` Micros takes longer to return on these clocks (64/clock cycles per microsecond is not an integer).

`‼` `micros()` and `delayMicroseconds()` require a system clock of 1 MHz or higher.

**Warning** Particularly in versions prior to 1.5.0, When using weird clock frequencies (those other than 16MHz, 8MHz, 4MHz, 2MHz, 1MHz, 0.5MHz), micros() is significantly slower (~ 110 clocks) (It reports the time at the point when it was called, not the end, however, and the time it gives is pretty close to reality). This combination of performance and accuracy is the result of hand tuning for these clock speeds. For other clock speeds (for example, if you add your own), it will be slower still - hundreds of clock cycles - though the numbers will be reasonably accurate, and reflect the time when it was called. millis() is not effected, only micros() and delay().

This differs from the behavior of official Arduino core - the "stock" micros() executes equally fast at all clock speeds, and instead returns wrong values for "weird" clock speeds (64/(clock speed in microseconds, rounded down to integer), rounded down to integer. 12.8 MHz is a special case and is handled exactly.

Thanks to @cburstedde for his work on this.

#### Using external CLOCK (new in 1.3.3 for on 48, 88, and 828, expanded everywhere for 2.0.0)
These parts do not support using an external crystal. External Clock, however, is supported - this requires an external clock source (not just a crystal) connected to the CLKI pin. **DANGER** if this clock source is not present, you must supply a clock source to CLKI pin before it can be reprogrammed, including to use a different clock source. The external CLOCK option is available through the IDE; in previous versions this was limited to parts which didn't support a crystal. With greater recognition that unbricking the parts is straightforward (we include an appropriate sketch in the examples), as of 2.0.0, we now support this on all parts. **This is not the same as external crystal - do not use this option if you are unsure about the difference between external clock and external crystal!** External clock sources are commonly sold as "oscillators", we would recommend the KC5032A-series for it's low cost and wide operating voltage range of 1.6-5.5v (ie, the entire operating range of these parts - but they're discontinued and there is no longer any external clock for sale from any western manufacturer or subsidiary thereof that is openly marketing an external oscillator with such  a wide voltage range). Every other oscillator available from Digikey has a narrower voltage range (often 3.3v or 5v +/- 10%, though some work from wider ranges). Through-hole units are available, but expensive, and all have the restrictive supply voltage requirements. If selecting your own oscillator, you want an "XO" type.

#### Using external CLOCK (instead of crystal) on other parts
The use of an external clock - that is, a single wire with an appropriate clock signal is supplied to the XTAL1 pin from an external source, is possible using this core. This is an advanced feature, and is not supported directly through the IDE (except as noted above) to reduce the risk of people confusing it with external crystal and bricking their chips (if external clock is set as clock source when actually using external crystal, you must supply a clock signal on XTAL1 to program the chip again, including to set it to use a crystal again). To use an external clock:
1. Connect the chip, and ensure that the external clock source is present.
2. Select the desired clock speed with the external crystal option.
3. From File -> Preferences, ensure that "Show verbose output" is enabled for "uploading".
4. Connect your ISP programmer.
6. Choose "Burn Bootloader". This will set all the selected fuses, only it will be set to use a crystal (this is fine, the external clock will substitute this). However, at this point the XTAL2 pin will not be usable as GPIO.
7. Scroll up in the bottom pane of the IDE to the top of the upload output. The first line will be the avrdude command used to burn the bootloader. Copy this into a text editor. If the command includes -e, remove -e and everything after it. If it just points to the bootloader hex file, remove "-Uflash:w:(path to bootloader .hex file):i" from the end of the command.
8. Add -Ulfuse:w:0xF0:m to the end of the command.
9. Copy/paste this command into a command window, and hit return. Your chip should now be running on the external clock, and the XTAL2 pin (but not the XTAL1 pin) will be available for use normally.
10. When compiling for and uploading to your now-externally-clocked part, use the external crystal option corresponding to the clock speed you are using.

#### Determining clock speed and source from within the sketch
The clock speed is made available via the F_CPU #define - you can test this using #if macro

In version 1.3.3 and later, the clock source is also made available via the CLOCK_SOURCE #define. CLOCK_SOURCE can take one of the following values (as of 1.4.0, it is expanded to cover a few weird clocking situations: the low 4 bits identify the source, and high 4 bits identify special things regarding it:

```text
0 - Internal 8MHz oscillator, not prescaled, or prescaled to 1 MHz (ie, fully set by fuses)
1 - External Crystal
2 - External Clock (only available within the core on the 48, 88 and 828, as described above - note that above steps to use external clock on other parts, this will still be 1, not 2; as far as the core knows, it's a crystal)
3 - Internal WDT oscillator  (not available on the x41, 1634, and 828)
4 - Internal ULP oscillator (available only on the x41, 1634, and 828)
5 - Internal 4MHz oscillator (present only on the x313 parts - if the 8MHz internal oscillator is prescaled to 4MHz, CLOCK_SOURCE will be 0, not 5)
6 - Internal PLL (x5 and x61 only)
15 or 0x10 (ie, 0x10 | 0) - Internal oscillator with prescaling not set by fuses (ie, not 1 MHz or 8 MHz - ie, 2 or 4 MHz)
17 or 0x11 (ie, 0x10 | 1) - External crystal at 16MHz, which may be prescaled to get lower frequencies (for Digispark Pro ATtiny167)
18 or 0x12 (ie, 0x10 | 2) - External clock  at 16MHz, which may be prescaled to get lower frequencies (for MH Tiny ATtiny88)
```


### How to refer to pins
There are several ways to refer to pins.
1. **Recommended: `PIN_PA0` - PIN_Pxn notation** - the Pxn constants are defined in the headers
  a. For all Pxn that corresponds to a pin, the headers define as n, eg, #define PA0 0, #define PA1 1 ... #define PB7 7). As it is unprecedented and generally agreed to be a mortal sin, we do not undefine anything that the Microchip IO headers supply - that road leads to hell (or at least, to users thinking they're in hell) - and that in turn is why there are so many programmers down there. We add to them (when they change the spelling of things, which they've been doing lately to newer parts, we add compatibility #defines with the name of the old constant), but we don't take away.
2. **Recommended: MISO, MOSI, SCK, SS, SCL, SDA** - Regardless of whether your device uses a USI, has hardware SPI, even hardware I2C, these defines are provided with the appropriate pins (though SS is the only one you need to know, since you need to keep it output)
3. **Deprecated: Numeric pin numbers** - these put a pin-mapping dependency on your code.
### Assembler Listing generation
Sketch -> Export compiled binary will generate an assembly listing in the sketch folder; this is particularly useful when attempting to reduce flash usage, as you can see how much flash is used by different functions. The file is named with abbreviations for all options selected from a tools menu, so you can export, change a tools submenu, export again, and then find two sets of listings waiting for you (if it's not added as an abbreviation, it does not change the binary output - or it shouldn't).

You should also have two .map files - these are memory maps. They are fucking ugly to read, and it's very difficult to get them into excel, for example, when it really shouldn't be. That is because the format that the linker uses is so evil that it is surely the work of the devil or a minion of his (All of the formats it uses suck. The one I use sucks the worst, but it also is the one that displays the most complete information. I've been working on python scripts to clean them up to be more readable. It's over in [AVR Research: Maps and Listings](https://github.com/SpenceKonde/AVR_Research/MapsAndListings)

### Link-time Optimization (LTO) support
As of 2.0.0, we no longer support use without Link Time Optimization. It is required for some of the tricks I use to ensure that useful compile errors appear instead of mysterious broken behavior at runtime when we know at compile time that it won't work, and link time optimization is a stunning reduction in sketch size as well.

### Makefile Support
For those who prefer to compile with a makefile instead of the IDE, sketches can be compiled with [https://github.com/sudar/Arduino-Makefile](https://github.com/sudar/Arduino-Makefile) - See the [makefile documentation](Ref_Makefile.md) for more information on specific steps needed for this process.

### I2C support
**You must have external pullup resistors installed** - unlike devices with a real hardware TWI port, the internal pullups cannot be used with USI-based I2C to make simple cases (short wires, small number of tolerant slave devices) work. In all cases, including parts with hardware I2C where it may work sometimes, you should **always** use external pullup resistors, as the internal ones are far weaker than the I2C standard requires for reliable operation.

On the following chips, I2C functionality can be achieved with the hardware USI. As of version 1.1.3 this is handled transparently via the special version of the Wire library included with this core. Be aware that USI-based I2C is not available when USI-based SPI is in use.
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (262/461/861)
* ATtiny x7 (87/167)
* ATtiny x313 (2313/4313)
* ATtiny1634

On the following chips, slave I2C functionality is provided in hardware, but a software implementation must be used for master functionality. This is done automatically with the included Wire library. In versions prior to 1.2.2, USI on these devices is a flash-hog; this has been greatly improved in 1.2.2.
* ATtiny828
* ATtiny x41 (441/841)

On the following chips, full master/slave I2C functionality is provided in hardware and the Wire library uses it:
* ATtiny x8 (48, 88)

### SPI support

On the following chips, full SPI functionality is provided in hardware, and works identically to SPI on Atmega chips:
* ATtiny828
* ATtiny x7 (87/167) (it has both a USI and full SPI, but the SPI library will use the SPI hardware)
* ATtiny x41 (441/841)
* ATtiny x8 (48, 88)

On the following chips, SPI functionality can be achieved with the hardware USI - as of version 1.1.3 of this core, this should be handled transparently via the SPI library. Take care to note that the **USI does not have MISO/MOSI, it has DI/DO**; when operating in master mode, DI is MISO, and DO is MOSI. When operating in slave mode, DI is MOSI and DO is MISO. The #defines for MISO and MOSI assume master mode (as this is much more common). Clock dividers 2, 4, 8 and >=14 are implemented as separate routines; **call `SPISettings` or `setClockDivider` with a constant value to use less program space**, otherwise, all routines will be included along with 32-bit math. Clock dividers larger than 14 are only approximate because the routine is optimized for size, not exactness. Also, interrupts are not disabled during data transfer as SPI clock doesn't need to be precise in most cases. If you use long interrupt routines or require consistent clocking, wrap calls to `transfer` in `ATOMIC_BLOCK`. Be aware that USI-based I2C is not available when USI-based SPI is in use.
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (262/461/861)
* ATtiny x7 (87/167)
* ATtiny x313 (2313/4313)
* ATtiny1634

### Serial Support

On the following chips, full serial (UART) support is provided in hardware, as Serial (and Serial1 for parts with two serial ports):
* ATtiny x313 (2313/4313)
* ATtiny x7 (87/167 - LIN support, including a very fancy baud rate generator)
* ATtiny x41 (441/841 - two UARTs)
* ATtiny1634 (two UARTs)
* ATtiny828

On the following chips, **no hardware serial is available**, however, a built-in software serial named `Serial` is provided to maximize compatibility. This uses the analog comparator pins (to take advantage of the interrupt, since very few sketches/libraries use it, while lots of sketches/libraries use PCINTs). **TX is AIN0, RX is AIN1** -  This is a software implementation - as such, you cannot receive and send at the same time. If you try, you'll get gibberish, just like using SoftwareSerial. [See also the discussion of baud rates.](Ref_TinySoftSerial.md)
* ATtiny x5 (25/45/85)
* ATtiny x4 (24/44/84)
* ATtiny x61 (261/461/861)
* ATtiny x8 (48/88)
* ATtiny43

This core is also fully compatible with the usual SoftwareSerial library if you want a software serial port on a different pin - however, you can still only transmit or receive on a single software serial instance at a time - on parts without hardware serial, this includes the builtin software serial "Serial" implementation noted above.

**Warning: Internal oscillator and Serial**
Note that when using the internal oscillator or pll clock, you may need to tune the chip (using one of many tiny tuning sketches) and set OSCCAL to the value the tuner gives you on startup in order to make serial (software or hardware) work at all - the internal clock is only calibrated to +/- 10% in most cases, while serial communication requires it to be within just a few percent. However, in practice, a larger portion of parts work without tuning than would be expected from the spec. That said, for the ATtiny x4, x5, x8, and x61-family I have yet to encounter a chip that was not close enough for serial using the internal oscillator at 3.3-5v at room temperature - This is consistent with the Typical Characteristics section of the datasheet, which indicates that the oscillator is fairly stable w/respect to voltage, but highly dependent on temperature.

The ATtiny x41-family, 1634R, and 828R have an internal oscillator factory calibrated to +/- 2% - but only at operating voltage below 4v. Above 4v, the oscillator gets significantly faster, and is no longer good enough for UART communications. The 1634 and 828 (non-R) are not as tightly calibrated (so they may need tuning even at 3.3v) and are a few cents less expensive, but suffer from the same problem at higher voltages. Due to these complexities, **it is recommended that those planning to use serial (except on a x41, 1634R or 828R at 2.5~3.3v, or with a Micronucleus bootloader) use an external crystal** until a tuning solution is available.

A tuning sketch is planned for a future version of this core. Progress on this has been made - slowly and fitfully. The current plan is for Optiboot bootloader to be optionally burnable with a "tuning" sketch - this will write the calibration value to the space between the end of the bootloader and the end of flash, and the bootloader will load this value if it is set.  ( #139 ) Micronucleus using internal oscillator already does this using the USB clock as a timebase, and loads it before the sketch

### ADC Support

All of the supported parts except for the x313-family have an Analog to Digital converter on chip. **Single-ended ADC inputs can be read using the ADC channel number or the Ax constant (they can NOT be read using the digital pin number)**. In addition to the single-ended input channels marked on the pinout diagrams, many also support differential ADC input channels. To use these, simply call analogRead() with the appropriate ADC channel number. To get the ADC channel number, refer to the datasheet - it is listed in the Register Description section of the chapter on the ADC, under the ADMUX register.''

### Timers and PWM
All of the supported parts have hardware PWM (timer with output compare functionality) on at least one pin. See the part-specific documentation pages for a chart showing which pins have PWM. In addition to PWM, the on-chip timers are also used for millis() (and other timekeeping functions) and tone() - as well as by many libraries to achieve other functionality. Typically, a timer can only be used for one purpose at a time.

On all supported parts, timekeeping functions are on timer0. On all parts except the tiny841/441 tone() is on timer1; on 841/441, in version 1.1.6 and later, tone() is on Timer2 to improve compatibility (on 1.1.5 and earlier, tone() is on Timer1 on all parts). This means that reconfiguring timer0 by manipulating it's registers will break millis() and delay(). Using tone() will prevent PWM from working on PWM pins controlled by Timer1 (Timer2 for 841/441), and manipulating it's registers will break tone(). Because tone() is now on Timer2 on the 841/441, you can use tone() at the same time as other libraries that use Timer1 (such as Servo, TimerOne, and many others).

Most of the ATtiny parts only have two timers. The attiny841 has a third timer - but be aware that it's Timer2 is very different from the Timer2 on the atmega328p and most other atmega parts - the '841 has a second 16-bit timer (identical to timer1), while the atmega parts usually have an 8-bit asynchronous timer. This means that libraries designed to use Timer2 on the usual Arduino boards (ex, ServoTimer2) cannot be used with the 841.

### Built-in tinyNeoPixel library

The standard NeoPixel (WS2812/etc) libraries do not support all the clock speeds that this core supports, and some of them only support certain ports. This core includes two libraries for this, both of which are tightly based on the Adafruit_NeoPixel library, tinyNeoPixel and tinyNeoPixel_Static - the latter has a few differences from the standard library (beyond supporting more clocks speeds and ports), in order to save flash. At speeds below 16MHz, you must select the port containing the pin you wish to use from the Tools -> tinyNeoPixel Port menu. This code is not fully tested at "odd" clock speeds, but definitely works at 8/10/12/16/20 MHz, and will probably work at other speeds, as long as they are 7.3728 MHz or higher. See the [tinyNeoPixel documentation](tinyNeoPixel.md) and included examples for more information.

### Retain EEPROM configuration option
All non-bootloader board definitions have a menu option to control whether the contents of the EEPROM are erased when programming. This only applies to ISP programming, and you must "burn bootloader" to set the fuses to apply this. Because it only applies to ISP programming, it is not available for Bootloader board definitions. On those, EEPROM is never retained, on the reasoning that if you are burning the bootloader to a chip, you are trying to restore it to a "fresh" state.

### B. O. D. (brown out detect) configuration option

Brown-out detection continuously monitors Vcc, and holds the chip in reset state (BOR) if the applied voltage is below a certain threshold. This is a good idea with slow-rising power supplies or where it is expected that the supply voltage could droop below the required operating voltage for the frequency it is running at (see the speed grade specification for the part you're using) - without BOD enabled, this can put the chip into a hung state until manually reset. However, BOD increases power consumption slightly, and hence may be inappropriate in low power applications.

The BOD voltage trigger level can be chosen from the tools -> BOD menu. The ATtiny441, 841, 828, and 1634 support independently configuring the BOD mode (active, sampled, disabled) for active and sleep modes (see the applicable datasheet for details). These are configured via the Tools -> BOD Mode (sleep) and Tools -> BOD Mode (active) menus.

In all cases, the selected BOD option(s) is/are configured by the fuses, so after changing these, you must "burn bootloader" to set the fuses.

For Micronucleus boards, only options with are vaguely close to manufacturer's spec and "disabled" are available - since with those parts, we know without a doubt what that the clock speed will be, we know what BOD levels would be appropriate. There's not much point to using BOD if the BOD level is so far below spec that the chip would stop running before it was reached.

### Option to disable millis()/micros()

The Tools -> millis()/micros() allows you to enable or disable the millis() and micros() timers. If set to enable (the default), millis(), micros() will be available. If set to disable, these will not be available, Serial methods which take a timeout as an argument will not have an accurate timeout (though the actual time will be proportional to the timeout supplied); delay will still work. Disabling millis() and micros() saves flash, and eliminates the millis interrupt every 1-2ms; this is especially useful on parts with very limited flash, as it saves a few hundred bytes.

## Memory Lock Bits, disabling Reset

ATTinyCore will never set lock bits automatically, nor will it set fuses to disable ISP programming (it is intentionally not made available as an option, since after doing that, an HVSP programmer is needed to further reprogram the chip, and inexperienced users would be at risk of bricking their chips this way). The usual workflow when these bits are in use is Set other fuses -> Upload -> Test -> set the lockbits and/or fuses. This can be done from the command line using AVRdude. To expedite the process, you can enable "Verbose Upload" in preferences, do "burn bootloader" (the board and/or programmer does not need to be present), scroll to the top of the output window - the first line is the avrdude command used to burn the bootloader, including the paths to all the relevant files. It can be used as a template for the command you execute to set the fuse/lock bits. Disabling of reset is currently not an option, either - but for bootloader boards, this may change - VUSB bootloaders which disable reset are in widespread use, seemingly without issue. It will never be an option for non-bootloader boards because of the convoluted workflow required.
