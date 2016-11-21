### Minimal Wiring Requirements:

These are the bare minimum requirements to bring the chip to life and program it using the Arduino IDE. The 0.1uf capacitors are *not* optional - you must use these or the parts will behave erratically, and may not work at all; these should be placed as close to the chip as possible. 

* Vcc pin(s) connected to supply voltage (1.8~5v depending on part, clock speed, and fuse settings)
* Gnd pin(s) connected to Ground
* 0.1uf ceramic capacitor connected between Vcc and Gnd pins, as close to the part as practical. 
* If part has multiple Vcc pins, there should be 0.1uf cap from each one to ground, right next to the chip. 
* If part has an AVcc pin, that should be connected to Vcc. 
* If part has an AGnd pin, that should be connected to Gnd.
* If part has an AVcc pin and an AGnd pin, 0.1uf ceramic capacitor between AVcc and AGnd, right next to the chip. 
* if part has an AVcc pin but no AGnd pin, 0.1uf cerapic capacitor between AVcc and Gnd, right next to the chip. 

If the power supply is unstable, or the wires from the supply are long, it is highly recommended to put a larger capacitor between power and ground on the board - the appropriate size and type will depend on the loading, voltage, stability of the power supply, and available space; typical values might range from 10 uf to 1000uf. Aluminum electrolytic caps are commonly used for this purpose. 

### Additional requirements for programming over Serial (841,1634,828,87 and 167 only):
* 6-pin "FTDI" header, pin order is, Gnd, CTS, Vcc, Tx, Rx, DTR. 
* 6-pin header Vcc and Gnd pins connected to Vcc and Gnd
* Tx and Rx connected to Rx and Tx pins of Tiny (Tx to Rx, Rx to Tx)
* 10k resistor between Reset pin and Vcc
* 0.1uf capacitor between Reset pin and DTR on 6-pin header.
* Diode (any) between Reset and Vcc, band towards Vcc. Necessity of this part is debated.
