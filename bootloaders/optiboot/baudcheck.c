/*
 * baudcheck.c
 * Mar, 2013 by Bill Westfield (WestfW@yahoo.com)
 * Exercises in executing arithmetic code on a system that we can't count
 * on having the usual languages or tools installed.
 *
 * This little "C program" is run through the C preprocessor using the same
 * arguments as our "real" target (which should assure that it gets the
 * same values for clock speed and desired baud rate), and produces as
 * output a shell script that can be run through bash, and THAT in turn
 * writes the desired output...
 *
 * Note that the C-style comments are stripped by the C preprocessor.
 */

/*
 * First strip any trailing "L" from the defined constants.  To do this
 * we need to make the constants into shell variables first.
 */
bpsx=BAUD_RATE
bps=${bpsx/L/}
fcpux=F_CPU
fcpu=${fcpux/L/}

// echo f_cpu = $fcpu, baud = $bps
/*
 * Compute the divisor
 */
BAUD_SETTING=$(( ( ($fcpu + $bps * 4) / (($bps * 8))) - 1 ))
// echo baud setting = $BAUD_SETTING

/*
 * Based on the computer divisor, calculate the actual bitrate,
 * And the error.  Since we're all integers, we have to calculate
 * the tenths part of the error separately.
 */
BAUD_ACTUAL=$(( ($fcpu/(8 * (($BAUD_SETTING)+1))) ))
BAUD_ERROR=$(( (( 100*($bps - $BAUD_ACTUAL) ) / $bps) ))
ERR_TS=$(( ((( 1000*($bps - $BAUD_ACTUAL) ) / $bps) - $BAUD_ERROR * 10) ))
ERR_TENTHS=$(( ERR_TS > 0 ? ERR_TS: -ERR_TS ))

/*
 * Print a nice message containing the info we've calculated
 */
echo BAUD RATE CHECK: Desired: $bps,  Real: $BAUD_ACTUAL, UBRRL = $BAUD_SETTING, Error=$BAUD_ERROR.$ERR_TENTHS\%





