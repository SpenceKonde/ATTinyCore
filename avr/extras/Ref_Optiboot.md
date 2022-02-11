# Optiboot bootloader
ATTinyCore includes Optiboot for all boards with at least 4k of flash as long as they are available in a version with at least 8k of flash (so no bootloaders for the 43 or 4313). This will use software serial on the same pins as the software serial port is available on, and which are shown on the pinout chart. Support for arbitrary remapping of TX pin is not provided.

## Baud Rates used

| System Clock | Baud (UART) | Baud (softser) | Baud (LIN - 87/167 only) |
|--------------|-------------|----------------|--------------------------|
|       20 MHz |      115200 |          57600 |                   115200 |
|     16.5 MHz |         n/a |            n/a |                      n/a |
|       16 MHz |       76800 |          57600 |                   115200 |
|     12.8 MHz |         n/a |            n/a |                      n/a |
|       12 MHz |      115200 |          57600 |                   115200 |
|        8 MHz |       76800 |          38400 |                   115200 |
|        6 MHz |       57600 |          28800 |                    76800 |
|   4 MHz xtal |       38400 |          14400 |                    56700 |
|   4 MHz int. |       9600* |          4800* |                    9600* |
|   2 MHz int. |       9600* |          4800* |                    9600* |
|        1 MHz |        9600 |           4800 |                     9600 |
|    3.686 MHz |       56700 |        14400** |                    56700 |
|    7.372 MHz |      115200 |        38400** |                   115200 |
|    9.216 MHz |      115200 |        38400** |                   115200 |
|  11.0592 MHz |      115200 |        38400** |                   115200 |
|  14.7456 MHz |      115200 |        57600** |                   115200 |
|   18.432 MHz |      115200 |        57600** |                   115200 |

Because each clock speed requires it;s own bootloader binary, the number of bootloaders required borders on the absurd: ATTinyCore 2.0.0 includes no less than 586 hex

## Entering the Bootloader
Two versions of every configuration are supplied - one for use with auto-reset, which waits 1 second for commuinication and does not run at power on reset, and one which waits 8 seconds and does run on power on reset, for use with manual reset schemes.

## Optiboot and MCUSR (reset cause)
In order to make the entry modes work correctly - regardless of sketch behavior - Optiboot for ATTinyCore must reset MCUSR prior to exiting. It stashes the value of MCUSR in the GPIOR0 register - in the unlikely event that your sketch needs to know the reset cause, the most efficient wat to do this is shown below.

```
uint8_t resetcause = GPIOR0;  // in reg, GPIOR0
GPIOR0 = 0;                   // out GPIOR0, __zero_reg__
if(resetcause != 0) {         // cpse reg, __zero_reg__
  resetcause = MCUSR;         // in reg, MCUSR
}
MCUSR=0;                      // out MCUSR, __zero_reg__
// 10 bytes, 5 words. Putting MCUSR = 0 inside the conditional is unnecessary, and costs an extra instruction or two.
```
If both GPIOR0 and MCUSR are 0 at the start of setup, and you use code like that shown above in your setup, that means that MCUSR has been cleared since the last reset and not stored to GPIOR. This likely happened because code execution ended up back at 0x0000 either intentionally or unintentionally (it commonly is the end result when the stack is corrupted leading to code attempting to 'return' to a bogus address - this is frequently what causes the chip to get into a bad state from which it needs a manual reset to fix, since no reset cycle occurs, so the peripheral registers are not in the expected state at startup after the "dirty reset". People also sometimes use that as a method of software reset (not recommended, because it doesn't actually reset the current state.).

In the even more unlikely event that your sketch uses GPIOR0, be sure to set it to 0 in setup. [What is GPIOR0?](https://github.com/SpenceKonde/AVR-Best-Practices/blob/master/LowLevelNotes.md)
