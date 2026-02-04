IRLib – an Arduino library for infrared encoding and decoding
Version 1.1 april 2013
Copyright 2013 by Chris Young http://cyborg5.com
 
This library is a major rewrite of IRemote by Ken Shirriff which was covered 
by GNU LESSER GENERAL PUBLIC LICENSE which as I read it allows me to make 
modified versions. That same license applies to this modified version. See 
his original copyright below. 

The latest Ken Shirriff code can be found at 
https://github.com/shirriff/Arduino-IRremote

My purpose was to reorganize the code to make it easier to add or remove 
protocols. As a result I have separated the act of receiving a set of raw timing 
codes from the act of decoding them by making them separate classes. That way 
the receiving aspect can be more black box and implementers of decoders and 
senders can just deal with the decoding of protocols.

Also added provisions to make the classes base classes that could be extended 
with new protocols which would not require recompiling of the original library nor 
understanding of its detailed contents. Some of the changes were made to reduce 
code size such as unnecessary use of long versus bool. Some changes were just my 
weird programming style. Also extended debugging information added.

IRremote
Version 0.1 July, 2009
Copyright 2009 Ken Shirriff
For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm 
http://arcfn.com

Interrupt code based on NECIRrcv by Joe Knapp
http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/

****************************************************
The package contains:
IRLib.cpp	Code for the library written in object C++
IRLib.h		Header file which you will include in your sketch
IRLibMatch.h	Match macros used internally. Need not include this unless you implement
		your own protocols
iRLibTimer.h	Attempts to detect type of Arduino board and allows you to modify which 
		interrupt timer you will use. Defaults to timer 2 as did the original KS 
		library. Alternate board and timer information based on a fork of the 
		original KS library. That for can be found here.
		https://github.com/TKJElectronics/Arduino-IRremote

Note: there is no "IRremoteInt.h" header as in the original library. Those values were 
	moved elsewhere.

The examples directory contains:
IRhashdecode	Demonstrates hash decoder.
IRrecord	Recording incoming signal and play it back when a character is sent 
		through the serial console. By using the console you no longer need 
		to wire up a pushbutton to run this code.
IRrecvDump	Receives a code, attempts to decode it, produces well formatted 
		output of the results using the new "dump" method.
IRsendDemo	Simplistic demo to send a Sony DVD power signal every time a 
		character is received from the serial monitor.
IRsendJVC	Demonstrates sending a code using JVC protocol which is tricky.
Samsung36	Demonstrates how to expand the library without recompiling it. 
		Also demonstrates how to handle codes that are longer than 32 bits.
IRservo		Demonstrates controlling a servo motor using an IR remote
IRserial_remote	Demonstrates a Python application that runs on your PC and sends
		serial data to Arduino which in turn sends IR remote signals.
Note: I did not port any of the other demo sketches although I may add IRTest later.
****************************************************
The library handles the following protocols:
NEC, Sony, RC5, RC6, Raw all of which were supported in the KS version.
Additionally added Panasonic_Old, JVC, NECx.
Also added KS hash code routines which he released separately.
