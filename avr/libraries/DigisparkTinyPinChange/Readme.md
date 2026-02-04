TinyPinChange library
=====================

**TinyPinChange** is an asynchronous (interrupt-driven) library designed to detect event (rising or falling edege) on pins.

Very often in the arduino world, users complain about conflicts between libraries.

This **TinyPinChange** library allows to share the "pin change interrupt" vector for several "clients".

For example, it's possible to use the **SoftRcPulseIn** library whilst using the **SoftSerial** library: both libraries rely on the **TinyPinChange** library.

Some examples of use cases:
-------------------------
* **Event detector** (on pins)
* **Frequency meter**
* **Pulse width meter**
* **Tachometer**
* **Duty cycle measurement**
* **Software serial port** (see **SoftSerial** library which relies on TinyPinChange)

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
* TinyPinChange_Init()
* TinyPinChange_RegisterIsr()
* TinyPinChange_EnablePin()
* TinyPinChange_DisablePin()
* TinyPinChange_GetPortEvent()
* TinyPinChange_GetCurPortSt()
* TinyPinChange_PinToMsk()
* TinyPinChange_Edge()
* TinyPinChange_RisingEdge
* TinyPinChange_FallingEdge

Design considerations:
---------------------
On the arduino MEGA, as all the pins do not support "pin change interrupt", only the following pins are supported:

* 10 -> 15
* 50 -> 53
* A8 -> A15

On other devices (ATmega328, ATtiny84, ATtiny85 and ATtiny167), all the pins are usable.

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

