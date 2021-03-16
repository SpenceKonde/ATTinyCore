#Conventions in Part Docs


## Tables
All part specific documentation contains a table like this.


Specification         |    ATtiny88    |       ATtiny88 |    ATtiny88    |      ATtiny48  |       ATtiny48 |
----------------------|----------------|----------------|----------------|----------------|----------------|
.                     |  No Bootloader |       Optiboot |  Micronucleus  |  No Bootloader |       Optiboot |
Uploading uses        |   ISP/SPI pins | Serial Adapter | USB (directly) |   ISP/SPI pins | Serial Adapter |
Flash available       |     8192 bytes |     8192 bytes |     6550 bytes |     4096 bytes |     3456 bytes |
RAM                   |      512 bytes |      512 bytes |      512 bytes |      256 bytes |      256 bytes |
EEPROM                |       64 bytes |       64 bytes |       64 bytes |       64 bytes |       64 bytes |
GPIO Pins             |     26 + RESET |     26 + RESET |     25 + RESET |     26 + RESET |     26 + RESET |
ADC Channels          |   8 (6 in DIP) |   8 (6 in DIP) |              8 |   8 (6 in DIP) |   8 (6 in DIP) |
PWM Channels          |      2 (9, 10) |      2 (9, 10) |      2 (9, 10) |      2 (9, 10) |      2 (9, 10) |
Interfaces            |       SPI, I2C |       SPI, I2C | vUSB, SPI, I2C |       SPI, I2C |       SPI, I2C |
Clocking Options      |         in MHz |         in MHz |         in MHz |         in MHz |         in MHz |
Int. Oscillator       |     8, 4, 2, 1 |     8, 4, 2, 1 |  Not supported |     8, 4, 2, 1 |     8, 4, 2, 1 |
Internal, with tuning |    8, 12, 12.8 |    8, 12, 12.8 |  Not supported |    8, 12, 12.8 |    8, 12, 12.8 |
External Crystal      |  Not supported |  Not supported |  Not supported |  Not supported |  Not supported |
Internal Clock        |   All Standard | 16,12,10,8,4,1 | **16**,8,4,2,1 |   All Standard | 16,12,10,8,4,1 |

* "Uploading Uses" lists the protocol used to upload sketches. All parts supported by ATTinyCore can ONLY have fuses written using ISP programming. Note that while most tinies *can* use a self-programming sketch like the Micronucleus Updater which erases the bootloader and puts in a different one, this has only been done for Micronucleus (this was done on account from which the programming command must be issued. That is the only time this core provides for updateing the bootloader
* Flash Available - usable flash for user code after the overhead of the bootloader. This assumes that the newest bootloader shipped with ATTinyCore is used. Micronucleus boards should have bootloader upgraded, as the stock bootloaders are often much worse.
* "With Tuning"  There is an option to upload a tuner when bootloading with optiboot, or "bootloading" to set fuses and clear flash in a no-bootloader configuration. If using ArduinoAsISP++ (included with this core) this is done automatically if it detects the tuner sketch on the target. (wait until the light on the programmer stops blinking). Micronucleus using the internal oscillator uses a different method that retunes it from the USB clock every time it is started on USB; this "boot tuning" is by default carried over to the sketch, but the core may be set to switch back to factory cal - or even the results of the tuning sketch, if either greater accuracy or a different tuned speed is required.
* Interfaces does not count software/bitbanged interfaces like software serial.
*
