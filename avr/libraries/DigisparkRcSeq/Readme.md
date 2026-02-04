RcSeq library
=============

**RcSeq** is an asynchronous library for ATmega328P (**UNO**), ATmega2560 (**MEGA**), ATtiny84, ATtiny85 (**Digispark**) and ATtiny167 (**Digispark pro**) to easily create **servo's sequences** and/or to execute **short actions** from RC commands, from a digital input, or from a launch function called in the sketch.

The **A**pplication **P**rogramming **I**nterface (**API**) makes **RcSeq** library very easy to use.

Some definitions:
----------------
	
* **Sequence**: is used to sequence one or several servos (sequence is defined in a structure in the user's sketch to be performed when the RC command rises). The Sequence table (structure) may contain some servo motions and some short actions to call at a predefined time. For each servo, start angle, end angle and speed are tunable.

* **Short Action**:   is used to perform a quick action (action is a short function defined in the user's sketch to be called when the RC command rises). The duration must be less than 20ms to not disturb the servo commands.

Some examples of use cases:
--------------------------
* **A landing gear retract:**
	* Lock, door and leg servos sequenced with a single RC channel
		* from a predefined position of the stick on the transmitter
		* from the 2 positions "Aux Channel" of the transmitter

* **Navigation lights for aircraft:**
	* Anticollision, beacon, landing lights commanded:
		* from predefined positions of the stick on the transmitter
		* from push-buttons in place of the stick potentiometer
		

* **Multi-switch:**
	* Up to 8 digital pins driven from a single RC channel
		* using the stick of the transmitter
		* using 8 push-buttons in place of the stick potentiometer

* **Zodiac animation:**
	* A pneumatic Zodiac dropped at sea and lifted back to the deck of a ship. Drop and lift sequences commanded:
		* from predefined positions of the stick on the transmitter
		* from a regular ON/OFF switch (for demo on table without RC set)

* **Animatronics sequences:**

	* leg motion,
	* mouth motion,
	* eyes motion, 
	* etc.

Triggers:
--------

**Sequences** and **short actions** can be trigged by:

* a RC signal (eg: RC receiver output)

	* from one or several **predefined positions of a stick** of the transmitter

	* from one or several **push-button** (keyboard) replacing a a stick of the transmitter. (**RcSeq** assumes Push-Buttons associated Pulse duration are equidistant).

	* from **Custom Keyboard** replacing a stick of the the transmitter. (The pulse durations can be defined independently for each Push-Button).

	* from **Multi position switch** (2 pos switch, 3 pos switch, or more, eg. rotactor) replacing a stick of the the transmitter.

* a regular ON/OFF switch (no RC set required).

* a launch function call in the sketch.

API/methods:
-----------
* RcSeq_Init()
* RcSeq_DeclareSignal()
* RcSeq_DeclareStick()
* RcSeq_DeclareKeyboard()
* RcSeq_DeclareCustomKeyboard()
* RcSeq_DeclareMultiPosSwitch()
* RcSeq_SignalTimeout()
* RcSeq_DeclareServo()
* RcSeq_DeclareCommandAndSequence()
* RcSeq_DeclareCommandAndShortAction()
* RcSeq_LaunchSequence()
* RcSeq_LaunchShortAction()
* RcSeq_Refresh()
* RcSeq_LibVersion()
* RcSeq_LibRevision()
* RcSeq_LibTextVersionRevision()

Macros and constants:
--------------------
* const SequenceSt_t
* const KeyMap_t
* RC_SEQUENCE()
* RC_CUSTOM_KEYBOARD()
* SHORT_ACTION_TO_PERFORM()
* MOTION_WITH_SOFT_START_AND_STOP()
* MOTION_WITHOUT_SOFT_START_AND_STOP()
* CENTER_VALUE_US
* RC_SEQ_START_CONDITION
* RC_SEQ_END_OF_SEQ

Design considerations:
---------------------

The **RcSeq** library requires 3 other libraries written by the same author:

 1. **TinyPinChange**:  a library to catch asynchronously the input change using Pin Change Interruption capability of the AVR microcontroller.

 2. **SoftRcPulseIn**:  a library to catch asynchronously the input pulses using **TinyPinChange** library.

 3. **SoftRcPulseOut**: a library mainly based on the **SoftwareServo** library, but with a better pulse generation to limit jitter and with some other enhancements.

CAUTION:
-------
The end user shall also use asynchronous programmation method in the loop() function (no blocking functions such as delay() or pulseIn()).

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).


