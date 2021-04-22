# Micronucleus bootloader

ATTinyCore now includes support for the popular Micronucleus bootloader. This is typically found on small boards such as the "Digispark" and "Digispark Pro", as well as less common boards based on the ATtiny84, 841, and 88. Micronucleus uses "VUSB", a bitbang USB implementation, to provide USB connectivity on parts which do not have hardware USB support. This is used to upload new code directly over USB as well as by sketches, allowing them to act as a USB mouse, keyboard, serial port, or other device, like a native USB device! In a future version, these USB libraries will be provided as part of the core.

This is currently supported only on parts for which readily available boards with the USB connector and associated hardware are available - this means the ATtiny85 (Digispark), ATtiny167 (Digispark Pro), ATtiny841 (Nanite and others), ATtiny84a, and ATtiny88 (MH-Tiny, MH-ET, HW-Tiny - seems to be known by many names). As this is the form that people are most likely to interact with these parts, the board definitions are designed to work with the available breakout boards.

In the future, the remaining boards will get the Micronucleus treatment, and I will make boards with that functionality available for purchase as well.

On Windows, Micronucleus requires drivers to be installed; this can be done by running the included batch file - but the Arduino IDE will not do that for you. When you install it, the path to the batch file will be printed in the console window. Alternately, the drivers can also be downloaded separately from https://github.com/digistump/DigistumpArduino/releases/download/1.6.7/Digistump.Drivers.zip - unzip, run installer.

### Uploading to a Micronucleus board
When you upload to a Micronucleus board, you will be prompted to "Please plug in the device" (there is a 60-second timeout on this). At this point, the board must be reset and/or plugged in. Depending on the bootloader version it is running, only certain types of reset will enter the bootloader (see below); almost all will enter the bootloader on a power-on reset when it detects that it is connected to a USB port (as opposed to just power). About a second after this reset, the computer will recognize the device, and the upload will commence. By scrolling up in the console output, you can also see the maximum sketch size that the bootloader reported was available. Often, this is not the same as what the ATTinyCore part-specific documentation specifies or what the compiler tests against. This is expected because have a different version of the bootloader; as Micronucleus development has progressed, improvements have been made to reduce the size of the bootloader. The bootloader entry mode also has an impact on flash usage. Additionally, "aggressive" versions of the bootloader have been made - these use optimizations to reduce the size of the bootloader which come at the cost of it doing a worse job at USB, which may cause it to malunction on specific chips, or with specific USB hosts (currently we do not supply such firmware with this core). While, as supplied, most of these boards come with recent versions of Micronucleus with (nearly) the same amount of usable space as the ones included with this package *many ATtiny85 based boards in the wild have a primitive version* that leaves only around 6k free for your sketch! Fortunately, upgrading is easy, even if reset has been disabled.

Currently, where multiple versions of the bootloader are included with different amounts of free space, the currently selected one is used (and if the bootloader actually on the board has less, Micronucleus will stop you); I may revisit this decision in a future version

### Entering the Bootloader
The Micronucleus bootloader supports a number of methods for entering the bootloader, and several were added during the course of adding support for it to ATTinyCore; the best choice will depend on your development methodology, end use, and personal preferences (many people seem to favor running only on power on so they can disable reset - I'm happy to sacrifice that pin for the convenience of pressing a reset button instead of having to plug and unplug something, especially considering how dismally far from the USBIF's design . On parts which have had the bootloaders updated, the following are supported:
* Always (any sort of reset will run the bootloader)
* External reset (or power on with reset tied to Vcc if-and-only-if reset disabled)
* External reset (as above) or WDT reset
* Power On
* Power On or WDT reset
* Any except WDT reset
* Always w/unsafe optimizations (only on parts where this gets an extra page)

All are built with fast exit if no USB, `START_WITHOUT_PULLUP` and `ENABLE_SAFE_OPTIMIZATIONS`, and all that depend on reset cause will clear MCUSR after reading it and copying it's contents to the `GPIOR0` register so that user code can access it, while the bootloader can clear the flags so that they do not get confused by them after the next reset.

The Micronucleus codebase also supports more tightly constrained options, including jumpered power-on only, reset only (don't "upgrade" a board with reset disabled to that...), watchdog only (if the sketch doesn't do what you expect with the watchdog, it'll require ISP reinstall to fix, or if reset disabled, brick the part).
In 1.4.0, each supported board had only a single entry option available. In a future version of ATTinyCore, more options for the entry mode will be provided.

The included firmware uses the following settings.
For the ATtiny85 (Digispark), ATtiny167 (Digispark Pro), ~and ATtiny88 (MH_Tiny)~ parts, the most widely "recommended" configuration is used - the bootloader is entered only upon power on reset.

For the ATtiny84, the bootloader is always entered.

For the ATtiny841, the bootloader is only entered upon external reset.

The entry behavior of the included versions of Micronucleus does not always match the version that the boards ship with - it looks like most of the boards ship with the "always" entry mode.

### Clock Speed
Micronucleus is built with a specific clock speed in mind; only a few are supported by the codebase, and generally the fastest that is an option is used: 12 MHz, 12.8 MHz, 15 MHz, 16 MHz, 16.5 MHz or 20 MHz. The 12.8 and 16.5 configurations are only used with the internal oscillator - and this extra speed gives the bootloader some room to correct for the lower clock accuracy. 16.5 is only possible on parts that can be clocked from an on-chip PLL (the ATtiny85 and ATtiny861 - though the internal oscillator on some (not all) ATtiny841 can reach those speeds as well - note that this is higher than the maximum spec for the chip, not just the internal oscillator!), and is within spec above 4.5v for those parts, while 12.8 MHz can be reached by the internal oscillator on other parts (although it is far outside spec for that). Where the internal oscillator is used, it is "tuned" to the needed frequency based on the USB clock.

In the case of the ATtiny85, the internal oscillator is (by default) left at the 16.5 MHz setting so that USB will also work in the sketch. On the ATtiny84 and ATtiny841, this is not done; in the current version of ATTinyCore, USB cannot be used within the sketch on these parts. When running at 16.5 MHz, be aware that timing related code may be be less accurate - this has not been tested carefully, but I believe that millis() will be correct, but micros - and possibly delay - may not be)

In order to support low-power operation, ATTinyCore provides an option to prescale the system clock on all Micronucleus boards (as well as most others) - as soon as the sketch starts, the oscillator will bw prescaled to run at the specified frequency. On the ATtiny85, an option for 12 MHz operation is also provided - this will reset `OSCCAL` to the factory setting

### Disabling Reset
Because a board running the Micronucleus bootloader can be reprogrammed without the use of the RESET pin, it is practical to use them with reset configured to act as an I/O pin. This is, obviously, of particular utility on the ATtiny85, where available pins are at a premium, but may be used on any part (except the ATtiny841, in the current version of this core). Because this requires changing the fuses, it can only be enabled with an ISP programmer. Once reset as been disabled, it can no longer be programmed via ISP. Only HVSP or parallel programming (depending on the part - each part supports one or the other, never both), wherein 12v is applied to the reset pin under specific conditions, can be used to unset this fuse or to unbrick a part where reset is disabled and the bootloader is no longer functioning. Note that the bootloader can still be upgraded over USB - including to a version that only runs upon external reset!

### Installing Micronucleus via ISP
This is used to install Micronucleus on a device without a different version already installed, or if it is necessary to change the fuse settings (such as to alter the selected board's BOD settings, reset configuration, etc). Connect an ISP programmer normally (as if programming it without a bootloader). From the Tools -> Burn Bootloader Mode, choose "Fresh Install", from Tools -> Programmer, ensure that the correct programmer is selected (on versions prior to 1.8.13 of the Arduino IDE, programmers with names that do not end in `(ATTinyCore)` may not work correctly) and then Burn Bootloader. The console log will report the results of the attempt.

### Upgrading Micronucleus via USB
Micronucleus provides an "upgrade" function - by uploading a program which contains a copy of the updated binary, erases the existing one, and writes a new one in it's place, the bootloader can be replaced even if reset has been disabled. Even if it hasn't been, it's a lot more convenient. This method seems to be reliable - the big danger would be uploading a bootloader version that only enters on reset, after disabling reset. It is planned that in a future version, when the entrty condition is *only* after external reset, such a bootloader will check verify that reset is not disabled, and ortherwise run bootloader under all conditions; sadly it looks like the overhead of this safety mechanism is going to be at least 20 bytes due to the clumsy process for reading the fuses from application on classic AVRs

### Micronucleus and MCUSR (reset cause)
In order to make the entry modes work correctly - regardless of sketch behavior - Micronucleus must reset MCUSR prior to exiting. It stashes the value of MCUSR in the GPIOR0 register - in the unlikely event that your sketch needs to know the reset cause, check GPIOR0, not MCUSR. This is currently done on the supplied bootloader for the ATtiny84, 85, and 88 parts; in a future release, it will be done for all parts. To ensure your sketch gets the correct reset cause whether or not the bootloader does this, know that one of these will be zero when the bootloader exits after any sort of reset, and the other will be non-zero. The non-zero one is the correct one. .
```
uint8_t resetcause = GPIOR0;
if(resetcause == 0) {
  resetcause = MCUSR;
  MCUSR = 0;
} 
// if you use GPIOR0 in your code
else {
  GPIOR0 = 0;
}

```

In the even more unlikely event that your sketch uses GPIOR0, be sure to set it to 0. [What is GPIOR0?](https://github.com/SpenceKonde/AVR-Best-Practices/blob/master/LowLevelNotes.md)
