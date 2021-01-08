# 1 "baudcheck.c"
# 1 "C:\\arduino-1.0.6-5.4.0compiler\\hardware\\arduino\\bootloaders\\optiboot_bld//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "baudcheck.c"
# 24 "baudcheck.c"
bpsx=2400
bps=${bpsx/L/}
bps=${bps/U/}
fcpux=1000000L
fcpu=${fcpux/L/}
fcpu=${fcpu/U/}
# 38 "baudcheck.c"
BAUD_SETTING=$(( ( ($fcpu + $bps * 4) / (($bps * 8))) - 1 ))
# 50 "baudcheck.c"
BAUD_ACTUAL=$(( ($fcpu/(8 * (($BAUD_SETTING)+1))) ))

BAUD_ERROR=$(( (( 100*($BAUD_ACTUAL - $bps) ) / $bps) ))
ERR_TS=$(( ((( 1000*($BAUD_ACTUAL - $bps) ) / $bps) - $BAUD_ERROR * 10) ))
ERR_TENTHS=$(( ERR_TS > 0 ? ERR_TS: -ERR_TS ))




echo BAUD RATE CHECK: Desired: $bps, Real: $BAUD_ACTUAL, UBRRL = $BAUD_SETTING, Difference=$BAUD_ERROR.$ERR_TENTHS\%
