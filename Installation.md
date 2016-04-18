Supported IDE versions
============
ATtinyCore (Universal) works with all versions of the official IDE (from arduino.cc ) from version 1.6.3 onwards. For board manager installation, 1.6.4 or later is required. Due to serious bugs introduced in 1.6.6, 1.6.7, and 1.6.8, version 1.6.5r5 of the IDE is recommended - though these bugs do not specifically impact this core.

Board Manager Installation
============

This core can be installed using the board manager. The board manager URL is:

`http://drazzy.com/package_drazzy.com_index.json`

1. File -> Preferences, enter the above URL in "Additional Board Manager URLs"
2. Tools -> Boards -> Board Manager...
  *If using 1.6.6, close board manager and re-open it (see below)
3. Select ATtinyCore (Universal) and click "Install". 

Due to [a bug](https://github.com/arduino/Arduino/issues/3795) in 1.6.6 of the Arduino IDE, new board manager entries are not visible the first time Board Manager is opened after adding a new board manager URL. 

Manual Installation
============
Option 1: Download the .zip, extract, and place in the hardware folder inside arduino in your documents folder. (if there is no (documents)/arduino/hardware, create it) 

Option 2: Download the github client, and sync this repo to (documents)/arduino/hardware. 


![core installation](http://drazzy.com/e/img/coreinstall.jpg "You want it to look like this")



