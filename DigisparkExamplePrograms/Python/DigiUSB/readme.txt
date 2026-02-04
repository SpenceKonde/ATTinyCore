For use with DigiUSB library - see the DigiUSB->Echo sketch for an example of use.

Monitor:
GUI with a send and recieve box. Plug in your Digispark with DigiUSB first.

Read and Write:
Run Read to read from the Digispark
Run Write to write to the Digispark

You probably want to run both at once.


Send and Receive:
Send will send the first argument to the Digispark - 
example:
send "This is a test"
would send "This is a test" without the quotes to the Digispark and sending a newline character at the end

Receive will recieve from the Digispark until a newline is reached and output all that it received

These should help provide quick and dirty integration with other programs/scripts.


Windows users should install the windows driver first.

Released into public domain
-Erik Kettenburg

Python source requires libusb