# Doc for ATtint85

<p align="center">
<img src="https://goo.gl/7cxEmm" width="500">
</p>

Follow those steps for use the Arduino as ISP:
- Load the sketch "ArduinoISP" on your Arduino
- Connect the Arduino with "ArduinoISP" to the ATtiny85. Use the schematic in this folder.
- Open your ATtiny85 program and select:
  * "tools" --> "board" --> "ATtiny"
  * "tools" --> "processor" --> "ATtiny85"
  * "tools" --> "programmer" --> "Arduino as ISP"

**IF** it's the **first time** you use the ATtiny OR you want to **change the frequency** (Mhz) OR **BOD settings**, you must burn the bootloader:
- "tools" --> Burn Bootloader

Now you can upload your sketch.


## The 0.1 uF capacitor
All ATTiny chips (as well as the vast majority of digital integrated circuits) require a 0.1uF ceramic capacitor between Vcc and Gnd for decoupling; this should be located as close to the chip as possible (minimize length of wires to cap). Devices with multiple Vcc pins, or an AVcc pin, should use a cap on those pins too. Do not be fooled by poorly written tutorials or guides that omit these.

## The 10 uF capacitor
The capacitor prevents the Arduino board from resetting (which starts the bootloader), thus ensuring that the Arduino IDE talks to the ArduinoISP (not the bootloader) during the upload of sketches. 4.7uF will also work, and may be preferred if the larger capacitor causes your USB hub to reset (which it will on many of the low quality USB hubs on the market).
