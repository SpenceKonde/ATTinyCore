SoftSerial library
==================

The **SoftSerial** library is exactly the same as the **SoftwareSerial** library but used with the **TinyPinChange** library which allows to share
the "Pin Change Interrupt" Vector.

**SoftwareSerial** monopolizes the Pin Change Interrupt Vector and do not allow sharing.

With **SoftSerial**, it's possible. Don't forget to #include **TinyPinChange** in your sketch!

Additionally, for small devices such as **ATtiny85** (Digispark), it's possible to declare **the same pin for TX and RX**.
Data direction is set by using the new **txMode()** and **rxMode()** methods.

Some examples of use cases:
-------------------------
* **half-duplex bi-directional serial port on a single wire for debuging purpose**
* **half-duplex serial port to interface with Bluetooth module**
* **half-duplex serial port to interconnect an arduino with another one**

Supported Arduinos:
------------------
* **ATmega368 (UNO)**
* **ATmega2560 (MEGA)**
* **ATtiny84 (Standalone)**
* **ATtiny85 (Standalone or Digispark)**
* **ATtiny167 (Digispark pro)**

Tip and Tricks:
--------------
Develop your project on an arduino UNO or MEGA, and then shrink it by loading the sketch in an ATtiny or Digispark (pro).

API/methods:
-----------
* The **SoftSerial** library uses the same API as the regular **SoftwareSerial** library:
	* begin()
	* end()
	* available()
	* read()
	* listen()
	* isListening()
	* overflow()
	* flush()

* Two additional methods are used to manage the serial port on a single pin:
	* txMode()
	* rxMode()

Design considerations:
---------------------
The **SoftSerial** library relies the **TinyPinChange** library **for the RX pin**. This one shall be included in the sketch as well.

On the arduino MEGA, as all the pins do not support "pin change interrupt", only the following pins are supported **for the RX pin**:

* 10 -> 15
* 50 -> 53
* A8 -> A15

On other devices (ATmega328, ATtiny84, ATtiny85 and ATtiny167), all the pins are usable.

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

