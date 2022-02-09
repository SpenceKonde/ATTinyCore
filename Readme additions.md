| Part Family     |   x4-series |  x41-series |   x5-series | ATtiny26 |    x61-series |  x7-series | x8-series | x313-series | ATtiny1634 | ATtiny828 | ATtiny43 |
|-----------------|-------------|-------------|-------------|----------|---------------|------------|-----------|-------------|------------|-----------|----------|
| Flash Size      |    2k/4k/8k |       4k/8k |    2k/4k/8k |     2048 |      2k/4k/8k |     8k/16k |     4k/8k |       2k/4k |      16384 |      8192 |     4096 |
| EEPROM Size     | 128/256/512 |     256/512 | 128/256/512 |      128 |   128/256/512 |        512 |           |             |        256 |       512 |       64 |
| RAM size        | 128/256/512 |     256/512 | 128/256/512 |      128 |   128/256/512 |        512 |           |             |       1024 |       512 |      256 |
| Internal 16 MHz |          No |  via tuning |    Yes, PLL | Yes, PLL |      Yes, PLL |         No |        No |          No |         No |        No |       No |
| Ext. Crystal    |         Yes |         Yes |         Yes |      Yes |           Yes |        Yes |        No |         Yes |        Yes |        No |       No |
| Clock Switching |          No |         Yes |          No |       No |            No | yes, buggy |        No |          No |        Yes |       Yes |       No |
| HV programming  |        HVSP |        HVSP |        HVSP | parallel |      parallel |   parallel |  parallel |    parallel |   parallel |  parallel | parallel |
| I/O pins        |          12 |          12 |           6 |       16 |            16 |         16 |        28 |          18 |         18 |        28 |       16 |
| Optiboot        |         Yes |         Yes |         Yes |       No |           Yes |        Yes |       Yes |          No |        Yes |       Yes |       No |
| Micronucleus    |         Yes |         Yes |         Yes |       No |           Yes |        Yes |       Yes |          No |        Yes |        No |       No |
| AREF Pin        |         Yes |         Yes |         Yes |          |           Yes |        Yes |        No |          No |        Yes |        No |       No |
| PWM pins        |           4 |       6 (8) |           3 |        2 |             3 |       3(9) |         2 |           4 |          4 |     4 (8) |        4 |
| Internal Refs   | 1V1         | 1V1,2V2,4V1 |   1V1, 2V56 |     2V56 |   1V1, 2V56   |   1V1,2V56 |       1V1 |         1V1 |        1V1 |       1V1 |      1V1 |
| Analog Pins     |           8 |          12 |           4 |       11 |            11 |         11 |    6 or 8 |        none |         12 |        28 |        4 |
| Diff. ADC pairs |          12 |  "46"* (18) |           2 |        8 |     "16" (10) |          8 |      none |        none |       none |      none |     none |
| Diff. ADC gain  |     1x, 20x | 1x,20x,100x |     1x, 20x |  1x, 20x | 1, 8, 20, 32x |    8x, 20x |      none |        none |       none |      none |     none |

`* Number in quotes is from the Atmel marketing material which are inconsistent with the counting methods used for other parts and the most basic of mathematcal concepts. Ex: on x41, for differential pairs, they counted every pair of pins twice (since you can reverse them) plus the 10 channels where the same input is used as both positive and negative, for offset calibration and that added up to 46. But historically, they only counted unique pairs, and didn't count channels that would read 0 except for offset error. That gets 18 pairs. Hence: "46" (18)`

I/O pins *includes* reset in the count; reset can only be used as GPIO if you disable reset (in which case you need to have an HV programmer to reprogram the part).

PWM pins where second number is shown in (parenthesis), the first number is the number of simultaneous, independent duty cycles that can be generated, and the one in parenethesis is the number of pins on which those can be output. See the part specific documentation for details, as the implementation and core integration (if any) varies.

Clock source switching is NEVER supported by ATTinyCore. The x7-series is impacted by scary errata with a very specific workaround.



Readme additions
* Document DIFF_Ax_Ay_gX defines
* Document ADC_GROUND, ADC_INTERNAL, etc
To read from an analog pin, pass either the An constant, OR the digital pin number. As of 2.0.0, we have dropped support for an unadulterated analog channel number; analog channel numbers are distinguished by
In addition to the analog pins, we now have proper support for reading from differential ADC channels, and whatever other channels the chip supports; these vary between parts - refer to the part-specific documentation for the chip you are working with for the list of names. Differential channels are named as DIFF_Ap_An_gX where p is the positive channel, n is the negative one, and g is the gain. The options vary wildly between parts - the ATtiny85 has 12 options while the ATtiny861 has 81 combinations of gain and pins (though 10 are duplicates). There is an exception to this convention for the ATtiny441/841 (there are too many options) with 46 differential pairs each with 3 gain options and 16 single-ended channels. See the part-specific documentation for details.

* Document existence of enhanced compile time error checking.
As I have learned new techniques, it is now possible for me to generate compile errors when functions are called in contexts where they cannot produce meaningful or coherent results, or where constant arguments are passed to them which will not give valid results. Now, instead of compiling and faiiling to work as intended (since errors can't be reported at runtime unless your code checks for them), these will now produce errors at compile time with a description of the problem so that it can be corrected. Two types of errors are shown, badArg() and badCall() - the former indicates that one of the arguments you are passing to it is both compile-time known and guaranteed not to produce meaningful results, while the latter indicates that the API function being called is inappropriate considering the part and options selected. Previously, these would generally result in an error that the function wasn't defined; if its defined for other parts, just not this one that doesn't have the support for that feature, now we tell you that.

For example:
* digitalWrite(10,HIGH) on an ATtiny85 - it only has 6 I/O pins at most. You're probably adapting code for another part and missed a digitalWrite() - before this would compile, but the offending digitalWrite would have no effect.
* analogRead(1) on an ATtiny4313, which has no ADC. This is now a compile error, previously it would.... attempt to look up the digital pin for that analog channel (and of course get NOT_A_PIN), digitalRead() the not-pin, and return 0 because it got a LOW back because we also didn't detect runtime bad pins carefully there either.

This doesn't stop you from using invalid values determined at runtime though Things like this will still compile - but this will catch a lot of mistakes while compiling, when we can show errors, instead of at runtime, when we can't.
```
for (byte i = 4;i < x; i++) {
  digitalWrite(i,HIGH)
}
// where x may exceed the number of pins
```


* Document ADC_NO_CHECK_STATUS option flash saver



### Reading Temperature and Vcc voltage

**Voltage**

Set reference to `DEFAULT` and read the voltage reference (`ADC_INTERNAL1V1` is generally the only option on most parts). The result will be (1.1 / Vcc) * 1023
