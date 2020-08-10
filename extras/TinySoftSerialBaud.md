### TinySoftSerial ("Serial" on parts that don't have one) and baud rates

The builtin software serial implementation named Serial on the parts without a hardware UART, the x4, x5, x61, x8, and 43, uses a a bunch of hand tuned assembly to receive and transmit serial data. While one should not try to push it by running at particularly high baud rates (maximum practical baud rates were not tested - it may be better than we give it credit for), the "delay" loop used for timing also has a lower bound on the baud rate that it can generate, because it uses a byte to count down it's iterations. As there is no means of error reporting, if you try to use a baud rate that is too low, nothing will be received, and data will be printed at a speed around or slightly below the minimum baud rate shown in the below table. The theoretical maximum baud rate is likely significantly higher than could ever be achieved, however, I couldn't say what the practical limits are. In any event, attempting to exceed the theoretical limit will have the same effect as trying to use a baud rate lower that the minimum.
Clock Speed  | Minimum Baud | Theoretical maximum baud
------------ | -------------|---------------------------
0.5 MHz|161|9803
1 MHz|322|19607
4 MHz|1290|78431
6 MHz|1935|117647
7.3728 MHz|2378|144564
8 MHz|2580|156862
9.216 MHz|2972|180705
11.0592 MHz|3567|216847
12 MHz|3870|235294
14.7456 MHz|4756|289129
16 MHz|5161|313725
16.5 MHz|5322|323529
18.432 MHz|5945|361411
20 MHz|6451|392156



Note that there is no particular advantage to the "UART Clocks" when using any software serial implementation; they are included here for completeness.
