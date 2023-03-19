# Micronucleus bootloader

ATTinyCore now includes support for the popular Micronucleus bootloader. This is typically found on small boards such as the "Digispark" and "Digispark Pro", as well as less common boards based on the ATtiny84, 841, and 88. Micronucleus uses "VUSB", a bitbang USB implementation, to provide USB connectivity on parts which do not have hardware USB support.

## Warning: This doesn't get you USB from within your sketch
In theory VUSB could be used to make a sketch act as a mouse, keyboard, MIDI device, or other USB device. In practice, this doesn't really work. The original digispark core bent over backwards to try to make it work, and it still didn't really work very well,  Your best bet would be to disable millis, and make sure that nothing that disables interrupts can occur while you expect USB communication. There are also considerable obstacles to VUSB drivers on Windows platforms. I was only able to get two examples to compile. One example uploaded and gave "this device has malfunctioned", the other, which pretended to be a mouse, made the cursor go crazy for a few seconds before crashing. After discussions with individuals who know more about the USB protocol than I, I concluded that it would require throwing out too much of the Arduino API functionality to enable such a niche use.  when there are always the u2 and u4 series, and the announced-and-retracted-hours-later DU-series (take a DD-series part, dedicate TCD0 to the PLL required for USB )

## Windows drivers
On Windows, Micronucleus requires drivers to be installed; this can be done by running the included batch file - but the Arduino IDE will not do that for you. When you install it, the path to the batch file will be printed in the console window. Alternately, the drivers can also be downloaded separately from [https://github.com/digistump/DigistumpArduino/releases/download/1.6.7/Digistump.Drivers.zip](https://github.com/digistump/DigistumpArduino/releases/download/1.6.7/Digistump.Drivers.zip) - unzip, run installer.

## Uploading to a Micronucleus board
When you upload to a Micronucleus board, you will be prompted to "Please plug in the device" (there is a 60-second timeout on this). At this point, the board must be reset and/or plugged in. Depending on the bootloader version it is running, only certain types of reset will enter the bootloader (see below); almost all will enter the bootloader on a power-on reset when it detects that it is connected to a USB port (as opposed to just power). About a second after this reset, the computer will recognize the device, and the upload will commence. By scrolling up in the console output, you can also see the maximum sketch size that the bootloader reported was available. Often, this is not the same as what the ATTinyCore part-specific documentation specifies or what the compiler tests against. This is expected because have a different version of the bootloader; as Micronucleus development has progressed, improvements have been made to reduce the size of the bootloader. The bootloader entry mode also has an impact on flash usage. Additionally, "aggressive" versions of the bootloader have been made - these use optimizations to reduce the size of the bootloader which come at the cost of it doing a worse job at USB, which may cause it to malunction on specific chips, or with specific USB hosts (currently we do not supply such firmware with this core). While, as supplied, most of these boards come with recent versions of Micronucleus with (nearly) the same amount of usable space as the ones included with this package *many ATtiny85 based boards in the wild have a primitive version* that leaves only around 6k free for your sketch! Fortunately, upgrading is easy, even if reset has been disabled.

Currently, where multiple versions of the bootloader are included with different amounts of free space, the currently selected one is used (and if the bootloader actually on the board has less, Micronucleus will stop you); I may revisit this decision in a future version

## Entering the Bootloader
The Micronucleus bootloader supports a number of methods for entering the bootloader, and several were added during the course of adding support for it to ATTinyCore; the best choice will depend on your development methodology, end use, and personal preferences (many people seem to favor running at power so they can disable reset, others prefer to not run on power on so there isn't that 6 second delay between when it's connected and when the sketch runs, etc). The following are supplied as prebuilt binaries for all applicable parts in 2.0.0:
* Power-on only
* Reset-pin and power-on
* Reset-pin alone if reset is enabled - but - if reset is set as an I/O pin, and held high during an reset, it will also enter the bootloader so you can unbrick the board.
* Any sort of clean reset, but not direct jumps.
* "Jumper" - Any time execution reaches the reset vector (by any reset, OR by a "dirty reset"/jmp 0x0000) while a specified pin is LOW.
* "Always" - Any time execution reaches the reset vector *including by direct jump*.
* Reset pin reset - even if other reset causes are present, with the same safeguard as described above.
* Only on a Watchdog Reset (so if your app doesn't trigger the WDT timeout, it won't enter the bootloader, and I hope you didn't disable reset)

All are built with fast exit if no USB, `START_WITHOUT_PULLUP` and `ENABLE_SAFE_OPTIMIZATIONS` (except for the ones listed as permitting jump), and all that depend on reset cause will clear MCUSR after reading it and copying it's contents to the `GPIOR0` register so that user code can access it, while the bootloader can clear the flags so that they do not get confused by them after the next reset.

## Configuration Summary
This means there are a total of 288 bootloader binaries, since 1 part comes with 2 pinmaps, 2 parts come with 3, and one of those two has the 12 MHz and 12.8 MHz (because the Micronucleus documentation swears up and down that 12 MHz won't work on the internal oscillator - the 841 and 1634 have a much nicer internal oscillator, so they are more plausible), and each one has fresh install version, and a usb "upgrade" version.  In about half of cases, either the "always" entry mode leaves one additional page of flash for user code, or the "jumper" mode takes up an extra page.

| Part     | D+  | D-  | LED | Jumper | Clock Source   | Boot Clock | Sketch Clock  | OSCCAL | Flash | Notes                                | D+ on INT0? |
|----------|-----|-----|-----|--------|----------------|------------|---------------|--------|-------|--------------------------------------|-------------|
| t84@12   |PB0-2|PB0-1|PB2/0| PA0    | USB Tuned int. | 12 MHz     | 12 MHz, 8 MHz | Store  |  6588 | 12 MHz is really pushing it w/int. * | Sometimes   |
| t84@12.8 |PB0-2|PB0-1|PB2/0| PA0    | USB Tuned int. | 12.8 MHz   | 12.8 MHz,8 MHz| Keep   |  6268 | 12.8 MHz may work better, but flash  | Sometimes   |
| t841  †! |PB0-2|PB0-1|PB2/0| PA0    | USB Tuned int. | 12 MHz     | 12 MHz, 8 MHz | Store  |  6524 | Wattuino Nanite, Bitboss, etc     ** | Sometimes   |
| t85   †‡ | PB4 | PB3 | PB1 | PB0    | USB Tuned PLL  | 16.5 MHz   | 16.5, 16 MHz  | Keep   |  6588 | Digispark and clones              ** | Yes         |
| t88    † | PD2 | PD1 | PD0 | PC7    | Ext. Clock     | 16.0 MHz   |       16 MHz  | N/A    |  6588 | MH-Tiny                            * | Yes         |
| t167   † | PB3 | PB6 | PB1 | PA7    | Crystal        | 16.0 MJz   |       16 MHz  | N/A    | 14844 | Digispark Pro                      * | Yes         |
| t167     | PB3 | PB6 | PA6 | PA7    | Crystal        | 16.0 MHz   |       16 MHz  | N/A    | 14844 | Azduino USB 167                    * | Yes         |
| t87      | PB3 | PB6 | *** | PA7    | Crystal        | 16.0 MHz   |       16 MHz  | N/A    |  6654 | Same LED pin options as '167         | Yes         |
| t861     | PA4 | PA5 | PB6 | PA3    | USB Tuned PLL  | 16.5 MHz   | 16.5, 16 MHz  | Keep   |  6654 | USB861 prototype (won't be in 2.0.0) | No          |
| t861     | PB6 | PB4 | PB5 | PA3    | USB Tuned PLL  | 16.5 MHz   | 16.5, 16 MHz  | Keep   |  6654 | Azduino USB 861                   ** | Yes         |
| t1634    | PC5 | PC4 | PC2 | PA0    | USB Tuned Int  | 12.0 MHz   |    12, 8 MHz  | Keep   | 14714 | Azduino USB 1634                     | No          |

`*` - One page of flash (64 or 128b) less is available with the jumper entry mode option on these parts.
`**` - One page of additional flash (64b) is available with the "always" entry mode option on these parts.
`†` - Boards based on these parts are commercially available from people other than me (let me know if I missed any).
`!` - These boards are known to be in current or recent production with more than one set of USB bins used. If reset is disabled, extreme care must be used to
`‡` - As the t85 comes in a through-hole version, the board can have a socket. Unfortunately, dishonest vendors often represent such boards as "programmers", leading to consumers loading them with a virgin tiny85, and wondering why it won't upload. While these boards can be an excellent choice, you d0 still need to bootload the chip via an ISP programmer or start with a bootloaded chip.

Note that here a page refers to the units that flash is erased in. On the 841 and 1634, they made the "pages" 1/4th the size.... (for smaller page buffer? No clue why...) but they can only be erased in blocks of 4. And they turned around and claimed that was a feature.


## Pin configurations
Three parts have multiple different pin configurations: The ATtiny84, the 841, and the 167.

In the case of the 167, it is merely cosmetic: the LED is on a different pin (required to make the routing work on a 0.425" wide board).

The matter of the 84 and 841 is more complicated... All of the products that have been described to me  use two of three pins for USB, and the LED is almost always used on the the third of those three pins; these are the pins of PORTB, PB0 through PB2. (PORTB on these parts is rather strange to begin with, as the on the chip are arranged in the order PA0 - PA7, PB2, PB3 (reset), PB1, PB0). PB0 and PB1 are the crystal pins (neither of these parts use a crystal for VUSB - they crank up the internal to 12 MHz). It is observed that the D+ pin is almost always on INT0 (even though the VUSB implementation in micronucleus uses PCINTs, typically). This is likely because when accessed from the sketch, the interrupt response time matters greatly, and INT0 is the highest priority interrupt - and also because you want to use an INTn interrupt, rather than occupying a whole port of PCINTs for the VUSB - though in the case of the 14-pin parts, by using the pins of PORTB for USB, the PORTA PCINTs (PCINT0) would still be free. Inexplicably, INT0 was moved from PB2 on the t84 to PB1 on the 841 (meaning the 841 can't use INT0 if clocked from a crystal, either). That explains why D+ pn PB2 and D- on PB1 has been used at times for the tiny84, and the pinout with D+ on PB1 as used on the Bitboss (why the Wattuino did it the other way around is harder to explain) - I suppose we're lucky that the other three possible combinations of those pins haven't been used in production boards too...

### Jumper option
The pin used for the jumper entry mode was only established for the 8-pin parts, for the others, it was chosen more or less arbitrarily, with an eye towards selecting a pin that was not of critical importance and which was preferably located next to a ground pin.

## Clock Speeds
Micronucleus is built with a specific clock speed in mind; only a few are supported by the codebase, and generally the fastest that is an option is used: 12 MHz, 12.8 MHz, 15 MHz, 16 MHz, 16.5 MHz or 20 MHz. The 12.8 and 16.5 configurations are only used with the internal oscillator - and this extra speed gives the bootloader some room to correct for the lower clock accuracy, though the 12.8 MHz speed generates significantly larger binaries (by about 320 bytes). 16.5 is only available on parts that can be clocked from an on-chip PLL (the ATtiny85 and ATtiny861) and is within spec above 4.5v for those parts, while 12.8 MHz can be reached by the internal oscillator on other parts (although it is far outside spec for the internal oscillator). It is worth noting that the internal oscillator on some (not all) ATtiny841's could at very close to the maximum, probably do 16.5, however this would be out of spec not only for the internal oscillator, but for the chip in general, and we do not provide binaries for this. Where the internal oscillator is used, it is "tuned" to the needed frequency based on the USB clock.

In the case of parts that run the bootloader at 16.5 or 12.8 MHz, the calibration needed for that is retained and the  the internal oscillator is (by default) left at the 16.5 MHz setting so that USB will also work in the sketch. On the ATtiny84 and ATtiny841, this is not done; in the current version of ATTinyCore, USB cannot be used within the sketch on these parts. When running at 16.5 MHz, be aware that timing related code may be be less accurate - this has not been tested carefully, but I believe that millis() will be correct, but micros - and possibly delay - may not be)

In order to support low-power operation, ATTinyCore provides an option to prescale the system clock on the more Micronucleus boards (as well as most others) - as soon as the sketch starts, the oscillator will bw prescaled to run at the specified frequency. On the ATtiny85, an option for 12 MHz operation is also provided - this will reset `OSCCAL` to the factory setting

## Disabling Reset
Because a board running the Micronucleus bootloader can be reprogrammed without the use of the RESET pin, it is practical to use them with reset configured to act as an I/O pin. This is, obviously, of particular utility on the ATtiny85, where available pins are at a premium, but may be used on any part (except the ATtiny841, in the current version of this core). Because this requires changing the fuses, it can only be enabled with an ISP programmer. Once reset as been disabled, it can no longer be programmed via ISP. Only HVSP or parallel programming (depending on the part - each part supports one or the other, never both), wherein 12v is applied to the reset pin under specific conditions, can be used to unset this fuse or to unbrick a part where reset is disabled and the bootloader is no longer functioning. Note that the bootloader can still be upgraded over USB - including to a version that only runs upon external reset!

## Installing Micronucleus via ISP
This is used to install Micronucleus on a device without a different version already installed, or if it is necessary to change the fuse settings (such as to alter the selected board's BOD settings, disable reset, etc). Connect an ISP programmer normally (as if programming it without a bootloader). From the Tools -> Burn Bootloader Mode, choose "Fresh Install", from Tools -> Programmer, ensure that the correct programmer is selected (on versions prior to 1.8.13 of the Arduino IDE, programmers with names that do not end in `(ATTinyCore)` may not work correctly) and then Burn Bootloader. The console log will report the results of the attempt.

## Upgrading Micronucleus via USB
Micronucleus provides an "upgrade" function - by uploading a program which contains a copy of the updated binary, erases the existing one, and writes a new one in it's place. In this way, the bootloader can be replaced even if reset has been disabled. Even if it hasn't been, it's a lot more convenient. This method seems to be reliable - it does however present a serious hazard to users who have disabled reset: It is possible to upgrade to a non-usable bootloader. The obvious example is one for which no entry condition will ever be met (the external reset entry mode binaries we supply have a safeguard - hold reset high at power on and it should enter the bootloader). The other hazard is "upgrading" to a bootloader version that uses different pins for VUSB. Unfortunately the 841 and 84 have both made the rounds with multiple permutations of the PORTB pins. On the x7, while there are two versions of the binary, they differ only in the LED pin, and it can still be used to upload a more appropriate bootloader version. On the X4 and X41, you're S. O. L. Either break out your HVSP programming hardware or buy a new micronucleus board. The good news is that the only board that would require HVPP to recover and has two pin mappings is the 861 - but the prototype option is being used only for internal testing, it will not be in any release.

## Micronucleus and MCUSR (reset cause)
In order to make the entry modes work correctly - regardless of sketch behavior - Micronucleus must reset MCUSR prior to exiting for all entry conditions that depend in any way on whether or what kind of reset occurred. It stashes the value of MCUSR in the GPIOR0 register - in the unlikely event that your sketch needs to know the reset cause, the most efficient wat to do this is shown below.

```c
uint8_t resetcause = GPIOR0;  // in reg, GPIOR0
GPIOR0 = 0;                   // out GPIOR0, __zero_reg__
if(resetcause != 0) {         // cpse reg, __zero_reg__
  resetcause = MCUSR;         // in reg, MCUSR
}
MCUSR=0;                      // out MCUSR, __zero_reg__
// 10 bytes, 5 words. Putting MCUSR = 0 inside the conditional is unnecessary, and costs an extra instruction or two.
```
If both GPIOR0 and MCUSR are 0 at the start of setup, and you use code like that shown above in your setup, that means that MCUSR has been cleared since the last reset and not stored to GPIOR. This likely happened because code execution ended up back at 0x0000 either intentionally or unintentionally (it commonly is the end result when the stack is corrupted leading to code attempting to 'return' to a bogus address - this is frequently what causes the chip to get into a bad state from which it needs a manual reset to fix, since no reset cycle occurs, so the peripheral registers are not in the expected state at startup after the "dirty reset". People also sometimes use that as a method of software reset (not recommended, because it doesn't actually reset the current state. ))

In the even more unlikely event that your sketch uses GPIOR0, be sure to set it to 0 in setup. [What is GPIOR0?](https://github.com/SpenceKonde/AVR-Best-Practices/blob/master/LowLevelNotes.md)
