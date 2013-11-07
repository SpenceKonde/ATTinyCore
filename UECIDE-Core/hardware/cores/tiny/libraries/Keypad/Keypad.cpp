/*
||
|| @file Keypad.cpp
|| @version 3.0
|| @author Mark Stanley, Alexander Brevig
|| @contact mstanley@technologist.com, alexanderbrevig@gmail.com
||
|| @description
|| | This library provides a simple interface for using matrix
|| | keypads. It supports the use of multiple keypads with the
|| | same or different sets of keys.  It also supports user
|| | selectable pins and definable keymaps.
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/
#include <Keypad.h>

// <<constructor>> Allows custom keymap, pin configuration, and keypad sizes.
Keypad::Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols) {
    rowPins = row;
    columnPins = col;
    sizeKpd.rows = numRows;
    sizeKpd.columns = numCols;

    begin(userKeymap);

    setDebounceTime(10);
    setHoldTime(500);
	keypadEventListener = 0;

    initializePins();

	startTime = 0;
}

// Let the user define a keymap - assume the same row/column count as defined in constructor
void Keypad::begin(char *userKeymap) {
    keymap = userKeymap;
}

// Returns a single key only. Retained for backwards compatibility.
char Keypad::getKey() {
	if (scanKeys() && (key[0].kstate==PRESSED) && key[0].stateChanged)
		return key[0].kchar;
	return NO_KEY;
}

bool Keypad::getKeys() {
	if (scanKeys()) {
		for (byte i=0; i<LIST_MAX; i++) {
			if (key[i].stateChanged) {
				return true;
			}
		}
	}
	return false;
}

// Private
// Scan the keypad.  Rerturn whether a key has been pressed or not.
bool Keypad::scanKeys() {
	boolean anyKey=false;

	// Scan keypad once every XX mS. This makes the loop() count
	// go from about 4,000 loops per second to about 40,000.
	if ( (millis()-startTime)>debounceTime ) {

		// When sharing row pins with other hardware they may need to be re-intialized.
		for (byte r=0; r<sizeKpd.rows; r++) {
			pin_mode(rowPins[r],INPUT_PULLUP);
			pin_write(rowPins[r],HIGH);	// Enable the internal 20K pullup resistors. (Arduino<101)
		}

		// Scan the entire keypad/keyboard and provide a key pressed status to
		// setKeyState().  Also, determine which keys are being pressed.
		for (byte c=0; c<sizeKpd.columns; c++) {
			pin_mode(columnPins[c],OUTPUT);
			pin_write(columnPins[c], LOW);	// Begin column pulse output.
			for (byte r=0; r<sizeKpd.rows; r++) {
				bitWrite(bitMap[r], c, !pin_read(rowPins[r]));  // keypress is active low but invert to high.
				if (bitRead(bitMap[r], c) == 1)
					anyKey = true;
			}
			// Set pin to high impedance input. Effectively ends column pulse.
			pin_write(columnPins[c],HIGH);
			pin_mode(columnPins[c],INPUT);
		}
		updateList();	// Manage the Active-Key list. Adding/Removing active keys.

		// Reset debounceTime delay.
		startTime = millis();
	}
	return anyKey;	// Report if any keys are active.
}

// Manage the key list. Any keys already on the list stay in their current slot
// and empty slots get filled with new keys.
void Keypad::updateList() {

	// Delete Idle keys
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kstate==IDLE) {
			key[i].kchar = NO_KEY;
			key[i].stateChanged = false;
		}
	}

	// Fill the empty slots in key[] list with new CLOSED keys.
	for (byte r=0; r<sizeKpd.rows; r++) {
		for (byte c=0; c<sizeKpd.columns; c++) {
			boolean button = bitRead(bitMap[r],c);
			char keyChar = keymap[r * sizeKpd.columns + c];
			int idx = findInList (keyChar);				// -1 = not on list, else returns key index
			// Update any key that was found on the list.
			if (idx>=0)
				setKeyState (idx, button);
			// Add a key to the list when the button is CLOSED.
			if ((idx==-1) && button) {
				// Check the whole list for an empty key slot and put it there.
				for (byte i=0; i<LIST_MAX; i++) {
					if (key[i].kchar==NO_KEY) {
						key[i].kchar = keyChar;
						key[i].kstate = IDLE;
						// All keys not on the list are considered IDLE. Using setKeyState
						// on new key will transition the key to the PRESSED state as expected.
						setKeyState (i, button);
						break;	// only fill the first empty slot.
					}
				}
			}
		}
	}
}

// Private
void Keypad::setKeyState(byte idx, boolean button) {
	// Clear stateChanged.
	key[idx].stateChanged = false;

	switch (key[idx].kstate) {
		case IDLE:		// Waiting for a keypress.
			if (button==CLOSED) {
				transitionTo (idx, PRESSED);
				holdTimer = millis(); }		// Get ready for next HOLD state.
			break;
		case PRESSED:
			if ((millis()-holdTimer)>holdTime)	// Waiting for a key HOLD...
				transitionTo (idx, HOLD);
			else if (button==OPEN)				// or for a key to be RELEASED.
				transitionTo (idx, RELEASED);
			break;
		case HOLD:
			// Waiting for a key to be RELEASED.
			if (button==OPEN)
				transitionTo (idx, RELEASED);
			break;
		case RELEASED:
			transitionTo (idx, IDLE);
			break;
	}
}

// New in 2.1
bool Keypad::isPressed(char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if ( (key[i].kchar == keyChar) && (key[i].kstate == PRESSED) )
			return true;
	}
	return false;	// Not pressed.
}

// New in 2.0
char Keypad::waitForKey() {
	char waitKey = NO_KEY;
	while( (waitKey = getKey()) == NO_KEY );	// Block everything while waiting for a keypress.
	return waitKey;
}

KeyState Keypad::getState() {
	return key[0].kstate;
}

// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
// Useful for keypads/keyboards returning a single key only.
bool Keypad::keyStateChanged() {
	return key[0].stateChanged;
}

// The number of keys on the key list, key[LIST_MAX], equals the number
// of bytes in the key list divided by the number of bytes in a key.
byte Keypad::numKeys() {
	return sizeof(key)/sizeof(Key);
}

// Minimum debounceTime is 10 mS. Any lower *will* slow down the loop().
void Keypad::setDebounceTime(uint debounce) {
	debounce<1 ? debounceTime=1 : debounceTime=debounce;
}

void Keypad::setHoldTime(uint hold) {
    holdTime = hold;
}

void Keypad::addEventListener(void (*listener)(char)){
	keypadEventListener = listener;
}

void Keypad::transitionTo(byte n, KeyState nextState) {
	key[n].kstate = nextState;
	key[n].stateChanged = true;

	if (keypadEventListener!=NULL)
		keypadEventListener(key[0].kchar);
}

void Keypad::initializePins() {
    // Configure column pin modes and states. Row pins get configured
    // in scanKeys(). See explanation there.
    // See http://arduino.cc/forum/index.php/topic,95027.0.html for an explanation
    // of why changing the column pins to INPUTs prevents inter-column shorts.
    for (byte C=0; C<sizeKpd.columns; C++) {
        pin_mode(columnPins[C],INPUT);
        pin_write(columnPins[C],HIGH);
    }
}

// Search for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kchar == keyChar) {
			return i;
		}
	}
	return -1;	// No matching key in list.
}


/*
|| @changelog
|| | 3.0 2012-07-12 - Mark Stanley     : Made library multi-keypress by default. (Backwards compatible)
|| | 3.0 2012-07-12 - Mark Stanley     : Modified pin functions to support Keypad_I2C
|| | 3.0 2012-07-12 - Stanley & Young  : Removed static variables. Fix for multiple keypad objects.
|| | 3.0 2012-07-12 - Mark Stanley     : Fixed bug that caused shorted pins when pressing multiple keys.
|| | 2.0 2011-12-29 - Mark Stanley     : Added waitForKey().
|| | 2.0 2011-12-23 - Mark Stanley     : Added the public function keyStateChanged().
|| | 2.0 2011-12-23 - Mark Stanley     : Added the private function scanKeys().
|| | 2.0 2011-12-23 - Mark Stanley     : Moved the Finite State Machine into the function getKeyState().
|| | 2.0 2011-12-23 - Mark Stanley     : Removed the member variable lastUdate. Not needed after rewrite.
|| | 1.8 2011-11-21 - Mark Stanley     : Added decision logic to compile WProgram.h or Arduino.h
|| | 1.8 2009-07-08 - Alexander Brevig : No longer uses arrays
|| | 1.7 2009-06-18 - Alexander Brevig : Every time a state changes the keypadEventListener will trigger, if set.
|| | 1.7 2009-06-18 - Alexander Brevig : Added setDebounceTime. setHoldTime specifies the amount of
|| |                                          microseconds before a HOLD state triggers
|| | 1.7 2009-06-18 - Alexander Brevig : Added transitionTo
|| | 1.6 2009-06-15 - Alexander Brevig : Added getState() and state variable
|| | 1.5 2009-05-19 - Alexander Brevig : Added setHoldTime()
|| | 1.4 2009-05-15 - Alexander Brevig : Added addEventListener
|| | 1.3 2009-05-12 - Alexander Brevig : Added lastUdate, in order to do simple debouncing
|| | 1.2 2009-05-09 - Alexander Brevig : Changed getKey()
|| | 1.1 2009-04-28 - Alexander Brevig : Modified API, and made variables private
|| | 1.0 2007-XX-XX - Mark Stanley : Initial Release
|| #
*/
