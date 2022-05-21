/* This should only ever be included from Arduino.h
 * No include guard here - because it is only included from one file that has one
 */

#define LOW              0    /* used for digitalWrite(), digitalRead(), openDrain() and attachInterrupt() */
#define HIGH             1    /* used for digitalWrite(), digitalRead(). There is no option for HIGH level interrupt provided by the hardware */
#define FLOATING         HIGH /* Not in official API - used for openDrain, while making clear what you're doing */

#define CHANGE           4    /* used for attachInterrupt() */
#define FALLING          2    /* used for attachInterrupt() */
#define RISING           3    /* used for attachInterrupt() */

#define INPUT            0    /* used for pinMode() */
#define OUTPUT           1    /* used for pinMode() */
#define INPUT_PULLUP     2    /* used for pinMode() */

#define LSBFIRST         0    /* used for shiftIn/shiftOut */
#define MSBFIRST         1    /* used for shiftIn/shiftOut */


/* Features ganked from megaTinyCore. */

#ifndef _NOP
  #define _NOP()    __asm__ __volatile__ ("nop");
#endif
#ifndef _NOP2
  #define _NOP2()   __asm__ __volatile__ ("rjmp .+0");
#endif
#ifndef _NOPNOP
  #define _NOPNOP() __asm__ __volatile__ ("rjmp .+0");
#endif
/* instead of 8 like AVRxt */
#ifndef _NOP9
  #define _NOP9()   __asm__ __volatile__ ("rjmp .+2"  "\n\t" \
                                          "ret"       "\n\t" \
                                          "rcall .-4" "\n\t");

/* instead of 14 like AVRxt */
#endif
#ifndef _NOP16
  #define _NOP16()  __asm__ __volatile__ ("rjmp .+2"  "\n\t" \
                                          "ret"       "\n\t" \
                                          "rcall .-4" "\n\t" \
                                          "rcall .-6" "\n\t" );
#endif

#ifndef _SWAP
  #define _SWAP(n) __asm__ __volatile__ ("swap %0"  "\n\t" :"+r"((uint8_t)(n)));
#endif



#if (F_CPU < 1000000L)
  //Prevent a divide by 0 is
  #warning "Clocks per microsecond < 1. To prevent divide by 0, it is rounded up to 1."
  #define clockCyclesPerMicrosecond() 1UL
#else
  #define clockCyclesPerMicrosecond() (F_CPU / 1000000UL)
#endif

//#define clockCyclesToMicroseconds(a) (((a) * 1000L) / (F_CPU / 1000L))
//#define microsecondsToClockCycles(a) (((a) * (F_CPU / 1000L)) / 1000L)

#define clockCyclesToMicroseconds(a) ((a) / clockCyclesPerMicrosecond())
#define microsecondsToClockCycles(a) ((a) * clockCyclesPerMicrosecond())

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define interrupts() sei()
#define noInterrupts() cli()

#define bit(b) (1UL << (b))
#define NOT_AN_INTERRUPT -1

/* Mathematic stuff */


#define PI            3.1415926535897932384626433832795
#define HALF_PI       1.5707963267948966192313216916398
#define TWO_PI        6.283185307179586476925286766559
#define DEG_TO_RAD    0.017453292519943295769236907684886
#define RAD_TO_DEG    57.295779513082320876798154814105
#define EULER         2.718281828459045235360287471352

#define SERIAL        0x00
#define DISPLAY       0x01

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#ifndef constrain
#define constrain(x,low,high)   ({  \
  __typeof__ (x)    _x = (x);           \
  __typeof__ (low)  _l = (low);         \
  __typeof__ (high) _h = (high);        \
  _x < _l ? _l : _x > _h ? _h :_x ;})
#endif

#ifndef radians
#define radians(deg) ((deg)*DEG_TO_RAD)
#endif

#ifndef degrees
#define degrees(rad) ((rad)*RAD_TO_DEG)
#endif

#ifndef sq
#define sq(x)        ({ __typeof__ (x) _x = (x); _x * _x; })
#endif

#ifndef round
#define round(x)     ({ __typeof__ (x) _x = (x);  _x >= 0.5 ? (long)(x + 0.5) : (long)(x - 0.5) ;})
#endif
