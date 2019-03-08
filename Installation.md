Supported IDE versions
============
ATtinyCore (Universal) works with all versions of the official IDE (from arduino.cc ) from version 1.6.3 onwards. For boards manager installation, 1.6.4 or later is required. Due to serious bugs, 1.6.6, 1.6.7, 1.6.8, and 1.6.10 are not recommended. Version 1.8.5 is recommended.

Boards Manager Installation
============

This core can be installed using the boards manager. The boards manager URL is:

`http://drazzy.com/package_drazzy.com_index.json`

1. File -> Preferences, enter the above URL in "Additional Boards Manager URLs"
2. Tools -> Boards -> Boards Manager...
  *If using 1.6.6, close boards manager and re-open it (see below)
3. Select "ATTinyCore by Spence Konde" and click "Install".

Due to [a bug](https://github.com/arduino/Arduino/issues/3795) in 1.6.6 of the Arduino IDE, new boards manager entries are not visible the first time Boards Manager is opened after adding a new boards manager URL.

Manual Installation
============
Option 1: Download the .zip, extract, and place in the hardware folder inside your sketchbook folder (if there is no hardware folder, create it). You can find/set the location of the sketchbook folder in the Arduino IDE at File > Preferences -> Sketchbook location.

Option 2: Download the github client, and sync this repo to the hardware subfolder of your sketchbook folder.


![core installation](http://drazzy.com/e/img/coreinstall.jpg "You want it to look like this")
