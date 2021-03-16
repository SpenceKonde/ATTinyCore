Readme additions
* Document DIFF_Ax_Ay_gX defines
* Document ADC_GROUND, ADC_INTERNAL, etc
To read from an analog pin, pass either the An constant, OR the digital pin number. As of 2.0.0, we have dropped support for an unadulterated analog channel number; analog channel numbers are distinguished by
In addition to the analog pins, we now have proper support for reading from differential ADC channels, and whatever other channels the chip supports; these vary between parts - refer to the part-specific documentation for the chip you are working with for the list of names. Differential channels are named as DIFF_Ap_An_gX where p is the positive channel, n is the negative one, and g is the gain. The options vary wildly between parts - the ATtiny85 has 12 options while the ATtiny861 has 81 combinations of gain and pins (though 10 are duplicates). There is an exception to this convention for the ATtiny441/841 (there are too many options) with 46 differential pairs each with 3 gain options and 16 single-ended channels. See the part-specific documentation for details.

* Document existence of enhanced compiletime error checking.
As I have learned new techniques, it is now possible for me to generate compile errors when functions are called in contexts where they cannot produce meaningful or coherent results, or where constant arguments are passed to them which will not give valid results. Now, instead of compiling and faiiling to work as intended (since errors can't be reported at runtime unless your code checks for them), these will now produce errors at compile time with a description of the problem so that it can be corrected. Two types of errors are shown, badArg() and badCall() - the former indicates that one of the arguments you are passing to it is both compile-time known and guaranteed not to produce meaningful results, while the latter indicates that the API function being called is inappropriate considering the part and options selected. Previously, these would generally result in an error that the function wasn't defined; if its defined for other parts, just not this one that doesn't have the support for that feature, now we tell you that.

For example:
* digitalWrite(10,HIGH) on an ATtiny85 - it only has 6 I/O pins. You're probably adapting code for another part and missed a digitalWrite() - before this would compile, but the offending digitalWrite would have no effect.
* analogRead(1) on an ATtiny4313, which has no ADC. This is now a compile error, previously it would.... attempt to look up the digital pin for that analog channel (and of course get NOT_A_PIN), digitalRead() the not-pin, and return 0 because it got a LOW back because we also didn't detect runtime bad pins carefully there either.

This doesn't stop you from using invalid values determined at runtime though Things like this will still compile - but this will catch a lot of mistakes while compiling, when we can show errors, instead of at runtime, when we can't.
```
for (byte i = 4;i < x; i++) {
  digitalWrite(i,HIGH)
}
// where x may exceed the number of pins
```


* Document ADC_NO_CHECK_STATUS option flash saver
