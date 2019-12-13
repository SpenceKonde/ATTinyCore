## Programming the ATtiny parts

All parts supported by ATTinyCore can be programmed using ISP (also called ICSP or SPI programming), using the SCK, MISO, MOSI and Reset pins. Most breakout boards (including all the ones I sell) provide a 6-pin (2x3) ISP programming header that matches the 6-pin connector used on common programmers. You will need an ISP programmer - the USBAsp and USBTinyISP programmers are readily available from ebay, amazon, aliexpress and many other vendors, typically for prices of around $3. If you get a USBAsp, be sure to get one with the 10-pin to 6-pin adapter. You can also use another Arduino (I know the AVR-based ones work, eg, nano, uno, pro mini - I can't make any promises about the fancier, non-AVR boards) using the Arduino as ISP sketch; upload this sketch to the board, and then place a ~10uF capacitor between reset and ground on that Arduino (this disables autoreset - it must be removed before you try to upload a different sketch).

If a bootloader has already been loaded on the board (this must be done using ISP programming, unless you bought the board pre-bootloaded), it can be programmed via a Serial adapter. Be sure the serial adapter has a DTR pin broken out. I recommend the version available on ebay by searching for 'CH340G 6-pin' on ebay, and looking for the black boards with the little 3.3v/5v voltage switch (the black ones with no switch don't break out DTR), or the green boards with the microUSB connector and 3.3v/5v voltage switch. Both of those have the standard FTDI pinout used on on Pro Mini and on my boards, and I have used both with great success.

In the following sections, the "programmer" is the ISP programmer or Arduino running Arduino as ISP. The "target" is the part you are programming.

We recommend that everyone enable Verbose Uploads - in Preferences, under "Show Verbose Output During", check "Upload" (we recommend unchecking "Compiling" unless chasing down a mysterious compile failure, as it produces a lot of output that usually isn't relevant).

### Programming via ISP (for burning bootloader, and uploading to non-optiboot boards)
Ensure that you have connected all necessary external components (see [Wiring Guide](Wiring.md). If you have an ISP programmer with the 6-pin header, and a 6-pin header on the board, connect it to the board. If you are using Arduino as ISP, the connections are:

* Vcc of programmer to Vcc of target
* Gnd of programmer to Gnd of target
* SCK of programmer (pin 13 on Uno/Nano/ProMini) to SCK of target
* MISO of programmer (pin 12 on Uno/Nano/ProMini) to MISO of target
* MOSI of programmer (pin 11 on Uno/Nano/ProMini) to MOSI of target
* Pin 10 (pin 10 on Uno/Nano/ProMini, or pin "RST" pin on the ISP connector of a dedicated programer) to RST of target

Be sure that you do not have anything on any of these pins that would load them down, including LEDs (if the series resistor is large - think >1k ohm, this is generally not a problem), these need to be disconnected while programming. If you have any SPI devices connected to the target, you must either disconnect them while programming, or connect a 10k resistor between their CS line and Gnd.

From the IDE, select your programmer from the Tools -> Programmer menu. Be sure to select the version with (ATTinyCore) after the name of the programmer. From the Tools -> Board menu, select the board that you are using. Do not select a board definition marked Optiboot unless you are planning to "Burn Bootloader" to prepare it for serial programming. From the Tools -> Chip menu, select the specific part you are using (if applicable). Select the desired BOD settings and clock source. If you are using an external crystal, you must have that crystal, and it's associated loading capacitors (see [Wiring Guide](Wiring.md)) connected, otherwise the chip cannot be reprogrammed until these are installed. If you are using a 48/88/828 with the External Clock option, you must have the external clock connected to the XTAL1 pin. If you are enabling BOD, you must select a voltage that is significantly less than the supply voltage - it should be the lowest voltage that the chip will plausibly run at; if you set BOD to a voltage higher than the supply voltage, it cannot be reprogrammed until a higher voltage is supplied.

If this is the first time you are using a brand new chip, or if you need to change the clock source or BOD settings, you must do "Burn Bootloader" to set the fuses. If you previously used optiboot, and wish to program with ISP instead, or if you want to install optiboot on the chip, you must do "burn bootloader" first as well.

If burning the bootloader, do Tools -> Burn Bootloader. You should see output on the console reporting that it was successfully written.

If uploading a sketch, click upload. You should see output on the console reporting that the sketch was successfully uploaded.

### Programming via Serial (Optiboot)

Ensure that you have connected all necessary external components (see [Wiring Guide](Wiring.md)), including the autoreset circuit. Unless you are using a breakout/development board where the bootloader was preinstalled, you must use an ISP programmer to "burn bootloader" as described above before you can reprogram it via serial. If you have a board with the 1x6 pin "FTDI" serial header, and a programmer with the same pinout, they can be connected directly, otherwise, make the following connections:

* Gnd of serial adapter to Gnd of target
* Vcc of serial adapter to Gnd of target (many adapters have a switch to select the voltage)
* Tx of serial adapter to Rx of target
* Rx of serial adapter to Tx of target
* DTR of serial adapter to DTR of target, or if using self-build board, the other side of the 0.1uF autoreset capacitor

From the IDE, select the board that you are using from the Tools -> Board menu; be sure to select the version marked (Optiboot). From the Tools -> Chip menu, select the specific part you are using (if applicable). From Tools -> Clock Source, select the clock and source. Then, just click "upload" and in the console, the IDE should report a successful upload.

### Tips and Tricks
* If you have a breakout board with holes for the 6-pin header, but don't want to solder pin header onto it, no problem. Just put a plug a piece of pin header into the connector on the programmer, insert it in the holes, and hold it at an angle, applying gentle pressure to make sure the pins make contact while you run the programming command. I use this technique routinely - it's how I used to bootload the boards I sell; you can also buy a "pogo-pin adapter" which has 6 spring loaded pins, which is a bit easier to handle (this is what I use now).

* Be sure you are plugging in the connector on the top of the board - if it is plugged in backwards, but on the correct side, it will not damage the board; if you don't know which way to orient the connector, you can try them both, and the one that works is correct. However, if you were to put the header in on the wrong side of the board, it would likely damage the board.

* For figuring out the correct orientation of the connector - if you look closely at a circuit board under good light, you can see the traces. Almost all boards have a "ground plane" on one side of the board - the ground pin will be connected to that ground plane by short traces - between one and four, each one pointing towards one edge of the board. All of the other pins will have a trace going off to some more distant location on the board.

* For using Arduino as ISP, it is easy to get a connection wrong. If you have a spare nano (clones are under $3), cut one end off of 6 dupont jumpers, and solder that end to the appropriate pin on the Nano. Secure these wires in place with hot-glue, otherwise they will break off very easily with normal handling. Then, arrange the other ends in a 2x3 arrangement with the usual ISP pinout, and tape them together with scotch tape. The result looks unprofessional, but it works. For a more professional look, remove the black housing from the end of the dupont jumpers (use the end of a pin to pull back the latch that holds them in place) and use a 2x3 pin Dupont Housing (available for cheap on ebay/aliexpress). It is much easier to use dupont line that already has the connectors on the end than to attempt crimping them yourself - crimping dupont connectors takes a lot of practice to do consistently.
