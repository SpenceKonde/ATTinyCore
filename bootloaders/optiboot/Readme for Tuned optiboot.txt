The tuned optiboot (precompiled in hex files and used by default) includes the ability to calibrate the internal oscillator of the host processor prior to the bootloader being enabled.

Once burnt using ISP, connect the bootloader up to the Serial port using:

Tiny84:
TX = PA1
RX = PA2
LED = PB2

Tiny85:
TX = PB0
RX = PB1
LED = PB2

(Connect DTR to Reset pin via 100nF capacitor, along with 10k resistor from Reset to VCC)

Open your favourite Serial/Termial program and set to 9600 baud.

You should see the LED blink several times (I believe it is 10).

In the Serial monitor you should see a Welcome message printed. If you don't, or you just get gibberish, don't worry, this just means the oscillator is uncalibrated.

Slowly keep sending the lower case letter 'x' with no line ending (no enter, carriage return, line feed etc.). Each time you send it you should see the LED blink.
Eventually you should see legible text being printed onscreen telling you the current calibration data.

Once calibration is complete you will be informed over the serial monitor (at this point text should be readable for sure). The calibration will then be stored in the flash memory and the chip will reset.

After this you can use optiboot normally via the upload button in Arduino. The stored calibration value will be loaded each time the chip resets by the bootloader so you don't have to do this in your program.

The tuning sketch takes up 4kB, however this is overwritten by any program you upload, which means the total bootloader size is only 576 bytes, which is the same as the bootloader without the tuning option, but the tuned bootloader is much more reliable.