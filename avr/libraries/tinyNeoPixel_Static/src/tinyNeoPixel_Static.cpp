/*-------------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811- and WS2812-based RGB
  LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
  Currently handles 800 KHz bitstreams on 8, 10, 12, 16, and 20 MHz ATtiny
  MCUs used and ATTinyCore 1.30+ with LEDs wired for various color orders.

  The tinyAVR 0/1/2-series and other modernAVRs use the version that
  comes with megaTinyCore or DxCore, not this version - the instruction
  timing is different.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC, Michael Miller and other members of the open
  source community.

  Modification was carried out between 2016 and 2021 by Spence Konde for
  ATTinyCore, and later megaTinyCore and DxCore. The latest modification
  eliminates the menu for the port - It uses ST instructions now, and
  while the assembly is uglier, the result is often smaller code that
  doesn't need a stupid menu option.

  This version also corrects improper constraint specifications that while
  silent were most certainly not legal. "r" is NOT a valid constraint if
  you're hoping to ldi the register, writing something that a pointer
  register is pointing AT isn't a "write" to that register, while
  reading something the pointer is pointing at with postincrement is.

  This is the "static allocation" version of this library. You must
  pass a pointer to a suitable array to use as the frame buffer, and
  you must ensure that the pin being used is set output. We don't do
  that here, so that you can use direct port writes to set it output
  while using less flash.

  -------------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "tinyNeoPixel_Static.h"

// Constructor when length, pin and type are known at compile-time:
tinyNeoPixel::tinyNeoPixel(uint16_t n, uint8_t p, neoPixelType t, uint8_t *pxl) :
  brightness(0), pixels(pxl), endTime(0)
{
  //boolean oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

  wOffset = (t >> 6) & 0b11; // See notes in header file
  rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  gOffset = (t >> 2) & 0b11;
  bOffset =  t       & 0b11;
  numBytes = n * ((wOffset == rOffset) ? 3 : 4);
  numLEDs=n;
  if(p >= 0) {
    pin = p;
#ifdef __AVR__
    port    = portOutputRegister(digitalPinToPort(p));
    pinMask = digitalPinToBitMask(p);
#endif
  }
}
// Cases where all three are not known at compile time are not supported by
// the statically allocated version of the library.



tinyNeoPixel::~tinyNeoPixel() {
}


void tinyNeoPixel::show(void) {
  if(!pixels) return;
  while(!canShow());
  /* Data latch = 50+ microsecond pause in the output stream.  Rather than
   * put a delay at the end of the function, the ending time is noted and
   * the function will simply hold off (if needed) on issuing the
   * subsequent round of data until the latch time has elapsed.  This
   * allows the mainline code to start generating the next frame of data
   * rather than stalling for the latch.
   * endTime is a private member (rather than global var) so that multiple
   * instances on different pins can be quickly issued in succession (each
   * instance doesn't delay the next).

   * In order to make this code runtime-configurable to work with any pin,
   * SBI/CBI instructions are eschewed in favor of full PORT writes via the
   * OUT or ST instructions.  It relies on two facts: that peripheral
   * functions (such as PWM) take precedence on output pins, so our PORT-
   * wide writes won't interfere, and that interrupts are globally disabled
   * while data is being issued to the LEDs, so no other code will be
   * accessing the PORT.  The code takes an initial 'snapshot' of the PORT
   * state, computes 'pin high' and 'pin low' values, and writes these back
   * to the PORT register as needed.
   */

  noInterrupts(); // Need 100% focus on instruction timing

// AVR MCUs --  ATtiny and megaAVR ATtiny -------------------------------

  volatile uint16_t
    i   = numBytes; // Loop counter
  volatile uint8_t
   *ptr = pixels,   // Pointer to next byte
    b   = *ptr++,   // Current byte value
    hi,             // PORT w/output bit set high
    lo;             // PORT w/output bit set low

  /* Hand-tuned assembly code issues data to the LED drivers at a specific
   * rate.  There's separate code for different CPU speeds (8, 12, 16 MHz).
   * The 400 kHz WS2811 code has been removed. Assembld LEDs configured that
   * way are entirely absent from the market and have been for years. The
   * datastream timing for the LED drivers allows a little wiggle room each
   * way (listed in the datasheets), so the conditions for compiling each
   * case are set up for a range of frequencies rather than just the exact
   * 8, 12 or 16 MHz values, permitting use with some close-but-not-spot-on
   * devices (e.g. 16.5 MHz DigiSpark).  The ranges were arrived at based
   * on the datasheet figures and have not been extensively tested outside
   * the canonical 8/12/16 MHz speeds; there's no guarantee these will work
   * close to the extremes (or possibly they could be pushed further).
   *
   * Research well documented online has shown that these controllers have
   * substantially more margin than the datasheet indicates on most of the
   * timing parameters.
   */


  /* 1/2022 changes for ATTinyCore 2.0.0:
   * There were a few issues here that were corrected.
   * 1. First, places where the code was port-specific were re-examined
   * with more indepth knowledge of AVR assembly than I had in the past.
   * This revealed that it is possible to make port-independent code which
   * relies on st, rather than out: No more tinyNeoPixel port submenu!!!
   *
   * 2. It was observed that the constraints were incorrect
   *   2a. [port] was given as an input/output variable. This is incorrect.
   *      We do not change port. We change the value of the memory that it
   *      points to but [port] itself remains unchanged. Thus, it is input only.
   *   2b. [ptr] was given as an input only. This is incorrect. While we do
   *      not modify the memory pointed to by ptr, we read it using ld with.
   *      postincrement and hence we DO modify ptr. Hence it is an
   *      input/output.
   *
   *      It looks like neither of those issues were causing problems here, because
   *      the variables get reinitialized every time show is called, but we should
   *      not demonstrate incorrect assembly. I was burned bt the postincrement
   *      thing while writing Flash.cpp for DxCore, and the broken behavior was
   *      a right pain in the backside to figure out.
   *
   *    2c. In implementations that use a bit counter, the constraint for bit was
   *      +r. The compiler preferentially used the call-used upper registers here
   *      so that worked, but it could legally have given it any register. But we
   *      initialize it every byte using ldi. Like all the other immediates, this
   *      requires an upper register pair. It's only because the link time optimization
   *      is flummoxed by classes and hence never inlines show() that this worked.
   *
   * 3. The new implementation required changes to the constraints in some places
   *    Specifically, [count] for some routines needs to be a "special upper register
   *    pair" (w) because we use sbiw (subtract immediate from word), while in others
   *    it need only be an upper register (d) because we can't fit 2 clocks for an
   *    sbiw anywhere, and need to use subi/sbci (subtract [with carry] immediate)
   *    which only works on upper registers (r16-r31)
   *
   * 4. Some of the routines had additional room for space optimization. We now take
   *    advantage of this to improve code size.
   */

// 8 MHz(ish) AVRe/AVRe+ ---------------------------------------------------------
#if (F_CPU >= 7370000UL) && (F_CPU <= 9500000UL)
  /* This is a NEW implementation! It lets us drop the port-specific
   * code. The timing is off a little at the end, but it's known that
   * the small timing inaccuracies at those points in the signal are okay.
   * Notice how we set up the next bytes bit7 in the previous bit's
   * low. The decrement was switched from a 2-clock SBIW to an SUBI
   * in place of the 5th NOP and SBCI  in place of the 6th. Then
   * we load the new bit in place of the 7th NOP since we're done with
   * it by then, and can do the rest with a normal body and a BRNE,
   * so the last 2 bits are 1 clock over, and all others are on target
   * The extra time is inserted into a low, which should be safe.
   * especially since it's only 125ns.
   * Zero is 2 clocks/250ns.
   * One is 7 clocks/875ns
   * low is 3 clocks/375ns, sometimes 4/500ns
   * 10 instruction clocks per bit:LLHHxxxxxL
   * ST instructions:              ^ ^    ^   (T=0,2,7)
   * Diagram assumes ST takes effect at end of the 2 clocks, but it
   * doesn't matter, as long as they're all the same.
   * And as if all that wasn't good enough, we also save 8 words of flash!
   */
  volatile uint8_t n1, n2 = 0;  // First, next bits out
  hi = (*port) |  pinMask;
  lo = (*port) & ~pinMask;
  n1 = lo;
  if(b & 0x80) n1 = hi;
  // And as if all that wasn't good enough, we also save 8 words of flash!

  asm volatile(
   "headD:"                   "\n\t" // Clk  Pseudocode
    // Bit 7: 1.25 us                // on target
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 6"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "nop"                     "\n\t" // 2   T = 10   nop
    // Bit 6: 1.25 us                // on target
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n2]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 5"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "nop"                     "\n\t" // 2   T = 10   nop
    // Bit 5: 1.25 us                // on target
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 4"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "nop"                     "\n\t" // 2   T = 10   nop
    // Bit 4: 1.25 us                // on target
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n2]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 3"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "nop"                     "\n\t" // 2   T = 10   nop
    // Bit 3: 1.25 us                // on target
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 2"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "subi %A[count], 1"       "\n\t" // 1   T = 10  i-- part 1
    // Bit 2: 1.25 us                // on target
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n2]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 1"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "sbci %B[count], 0"       "\n\t" // 1   T = 10  i-- part 2 - carrying clears 0 flag unless this is also 0. (don't act on Z flag yet)
    // Bit 1: 1.375 us               // 1 clock over
    "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 0"        "\n\t" // 1-2 T = 6   if(b & 0x40)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
    "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
    "ld   %[byte] , %a[ptr]+" "\n\t" // 2   T = 11  b = *ptr++ load next byte
    // Bit 0: 1.375 us               // 1 clock over
    "st   %a[port] , %[hi]"   "\n\t" // 2   T = 2   PORT = hi   0 and 2
    "st   %a[port] , %[n2]"   "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
    "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
    "sbrc %[byte] , 7"        "\n\t" // 1-2 T = 6   if(b & 0x80)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n1 = hi
    "st   %a[port] , %[lo]"   "\n\t" // 2   T = 9   PORT = lo
    "brne headD"              "\n"   // 2   T = 11  while(i) (Z flag set above)
  : [byte]  "+r" (b),
    [n1]    "+r" (n1),
    [n2]    "+r" (n2),
    [count] "+d" (i),
    [ptr]   "+e" (ptr)
  : [port]   "e" (port),
    [hi]     "r" (hi),
    [lo]     "r" (lo));

// 10 MHz(ish) AVRe/AVRe+ ---------------------------------------------------
#elif (F_CPU >= 9500000UL) && (F_CPU <= 11100000UL)
    /* In the 10 MHz case, an optimized 800 KHz datastream (no dead time
     * between bytes) is possible using ST... but requires unrolling the loop like 8
     * AND we can't directly use a relative branch, we have to jump over the rjmp.
     *
     * At least it is perfectly on target!
     *
     * 13 instruction clocks per bit: LLHHHHxxxxLLL
     * ST  instructions:              ^   ^   ^   (T=0,3,8)
     * 3 clocks  = 300ns zero
     * 8 clocks =  800ns one
     * 5 clocks  = 50ns low
     * 13 clocks = 1.30us total
     *
     * 73 instructions instruction words - largest of all.
     */
    volatile uint8_t next, bit;
    hi = (*port) |  pinMask;
    lo = (*port) & ~pinMask;
    n1 = lo;
    if(b & 0x80) n1 = hi;

    asm volatile(
     "headD:"                   "\n\t" // Clk  Pseudocode
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 6"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
      "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
      "nop"                     "\n\t" // 2   T = 13  nop
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n2]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 5"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
      "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
      "nop"                     "\n\t" // 2   T = 13  nop
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 4"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
      "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
      "nop"                     "\n\t" // 2   T = 13  nop
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n2]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 3"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
      "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
      "nop"                     "\n\t" // 2   T = 13  nop
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 2"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
      "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
      "nop"                     "\n\t" // 2   T = 13  nop
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n2]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 1"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10   PORT = lo
      "sbiw %[count], 1"        "\n\t" // 2   T = 12  i--
      "nop"                     "\n\t" // 2   T = 13  nop
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 0"        "\n\t" // 1-2 T = 7   if(b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "ld   %[byte] , %a[ptr]+" "\n\t" // 2   T = 11  b = *ptr++ load next byte
      "nop"                     "\n\t" // 2   T = 13   nop
      "st   %a[port] , %[hi]"   "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "nop"                     "\n\t" // 2   T = 3   nop
      "st   %a[port] , %[n2]"   "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
      "sbrc %[byte] , 7"        "\n\t" // 1-2 T = 7   if(b & 0x80)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n1 = hi
      "st   %a[port] , %[lo]"   "\n\t" // 2   T = 10   PORT = lo
      "breq .+2"                "\n\t" // 2   T = 11  while(i) (Z flag not set above)
      "rjmp headD"              "\n"   // 2   T = 13
    : [byte]  "+r" (b),
      [n1]    "+r" (n1),
      [n2]    "+r" (n2),
      [count] "+d" (i),
      [port]  "+e" (port)
    : [hi]     "r" (hi),
      [lo]     "r" (lo),
      [ptr]    "e" (ptr)); /* makes me nervous, we ARE modifying the value here with the postincrement */




// 12 MHz(ish) AVRe/AVRe+ ---------------------------------------------------
#elif (F_CPU >= 11100000UL) && (F_CPU <= 14300000UL)

    /* In the 12 MHz case, ST can be used easily. Everything stays on target
     * and we don't need to do anything particularly funky!
     * Only odd things:
     *  * It opens with a nop - because we brne 1 cycle early so we can
     * use a 2 clock instruction when we move on to the last bit.
     *  * We still set up next for first bit in previous bit's low.
     *  * We do a 7 cycle loop, and the last cycle is unrolled to cram in
     * loading the next byte
     *
     * 15 instruction clocks per bit: LLHHHHxxxxxxLLL
     * ST  instructions:              ^   ^     ^     (T=0,4,10)
     * 4 clocks  = 333ns zero
     * 10 clocks = 833ns one
     * 5 clocks  = 416ns low
     * 15 clocks = 1.25us total
     *
     * 21 instruction words! Booooyaaaah!!!
     */
    volatile uint8_t next, bit;

      hi   = *port |  pinMask;
      lo   = *port & ~pinMask;
      next = lo;
      if(b & 0x80) next = hi;
      bit  = 7;
      asm volatile(
        "headD:"                    "\n\t" // Clk  Pseudocode    (T = 14)
          "nop"                     "\n\t" // 1    nop,for last  (T = 15)
          "st  %a[port], %[hi]"     "\n\t" //                    (T =  2)
          "nop"                     "\n\t" // 1                  (T =  3)
          "rol  %[byte]"            "\n\t" // 1    b <<= 1       (T =  4)
          "st  %a[port], %[next]"   "\n\t" // 1    PORT = next   (T =  6)
          "mov  %[next], %[lo]"     "\n\t" // 1    next = lo     (T =  7)
          "sbrc %[byte], 7"         "\n\t" // 1-2  if(b & 0x80)  (T =  8)
           "mov %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  9)
          "dec  %[bit]"             "\n\t" // 1    bit--         (T = 10)
          "st  %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 12)
          "brne headD"              "\n\t" // 1-2  1 if false    (T = 13)
          "ld   %[byte] , %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 15)
          "st  %a[port], %[hi]"     "\n\t" //                     (T =  2)
          "sbiw %[count], 1"        "\n\t" // 2    i--            (T =  4)
          "st  %a[port] , %[next]"  "\n\t" // 2    PORT = hi      (T =  6)
          "mov  %[next] , %[lo]"    "\n\t" // 1    next = lo      (T =  7)
          "sbrc %[byte] , 7"        "\n\t" // 1-2  if(b & 0x80)   (T =  8)
           "mov %[next] , %[hi]"    "\n\t" // 0-1    next = hi    (T =  9)
          "ldi %[bit], 7"           "\n\t" // 1    7 loops        (T = 10)
          "st  %a[port] , %[lo]"    "\n\t" // 1    PORT = lo      (T = 12)
          "brne headD"              "\n"   // 1-2 to hea (T = 14 at head which starts with nop to finish this)
          : [byte]  "+r" (b),
            [next]  "+r" (next),
            [count] "+w" (i),
            [bit]   "+d" (bit), // the old implementation put this in "any register" but we need to LDI it! WTF!
            [port]  "+e" (port)
          : [ptr]    "e" (ptr) /* makes me nervous, we ARE modifying the value here with the postincrement */
            [hi]     "r" (hi),
            [lo]     "r" (lo));

// 14.7 MHz(ish) AVRe/AVRe+ --------------------------------------------------------
#elif (F_CPU >= 14300000UL) && (F_CPU <= 15400000UL)

    /* In the 14.7 MHz case, an optimized 800 KHz datastream (no dead time
     * between bytes) requires a PORT-specific loop similar to the 8 MHz
     * code (but a little more relaxed in this case).

     * 18 instruction clocks per bit: LLHHHHHxxxxxxxLLLL
     * ST  instructions:              ^    ^      ^     (T=0,5,12)
     * 5 clocks  = 340ns zero
     * 12 clocks = 816ns one
     * 6 clocks  = 406ns low
     * 18 clocks = 1.22us total
     * In the 12 MHz case, ST can be used easily. Everything stays on target
     * and we don't need to do anything particularly funky!
     * Only odd things:
     *  * It opens with a nop - because we brne 1 cycle early so we can
     * use a 2 clock instruction when we move on to the last bit.
     *  * We still set up next for first bit in previous bit's low.
     *  * We do a 7 cycle loop, and the last cycle is unrolled to cram in
     * loading the next byte
     *
     * 25 instruction words! Booooyaaaah!!!
     */
    volatile uint8_t next, bit;

      hi   = *port |  pinMask;
      lo   = *port & ~pinMask;
      next = lo;
      if(b & 0x80) next = hi;
      bit  = 7;

      asm volatile(
        "headD:"                    "\n\t" // Clk  Pseudocode    (T = 17)
          "nop"                     "\n\t" // 1    nop,for last  (T = 18)
          "st  %a[port], %[hi]"     "\n\t" //      PORT = hi     (T =  2)
          "rjmp .+0"                "\n\t" // 2    nop nop       (T =  4)
          "rol  %[byte]"            "\n\t" // 1    b <<= 1       (T =  5)
          "st  %a[port], %[next]"   "\n\t" // 1    PORT = next   (T =  7)
          "mov  %[next], %[lo]"     "\n\t" // 1    next = lo     (T =  8)
          "sbrc %[byte], 7"         "\n\t" // 1-2  if(b & 0x80)  (T =  9)
           "mov %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T = 10)
          "rjmp .+0"                "\n\t" // 2    nop nop       (T = 12)
          "st  %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 14)
          "dec  %[bit]"             "\n\t" // 1    bit--         (T = 15)
          "brne headD"              "\n\t" // 1-2  1 if false    (T = 16)
          "ld   %[byte] , %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 18)
          "st  %a[port], %[hi]"     "\n\t" //      PORT = hi      (T =  2)
          "nop"                     "\n\t" // 1    nop            (T =  3)
          "sbiw %[count], 1"        "\n\t" // 2    i--            (T =  5)
          "st  %a[port] , %[next]"  "\n\t" // 2    PORT = next    (T =  7)
          "mov  %[next] , %[lo]"    "\n\t" // 1    next = lo      (T =  7)
          "sbrc %[byte] , 7"        "\n\t" // 1-2  if(b & 0x80)   (T =  8)
           "mov %[next] , %[hi]"    "\n\t" // 0-1    next = hi    (T =  9)
          "ldi %[bit], 7"           "\n\t" // 1    7 loops        (T = 10)
          "rjmp .+0"                "\n\t" // 2    nop nop        (T = 12)
          "st  %a[port] , %[lo]"    "\n\t" // 1    PORT = lo      (T = 14)
          "nop"                     "\n\t" // 1    nop            (T = 15)
          "brne headD"              "\n"   // 1-2 to hea (T = 17 at head which starts with nop to finish this)
          : [byte]  "+r" (b),
            [next]  "+r" (next),
            [count] "+w" (i)
            [port]  "+e" (port),
          : [ptr]    "e" (ptr) /* makes me nervous, we ARE modifying the value here with the postincrement */
            [hi]     "r" (hi),
            [lo]     "r" (lo));


// 16 MHz(ish) AVRe/AVRe+ --------------------------------------------------
#elif (F_CPU >= 15400000UL) && (F_CPU <= 19000000L)
  // 20 inst. clocks per bit: HHHHHxxxxxxxxLLLLLLL
  // ST instructions:         ^    ^       ^       (T=0,5,13)
  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
     "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
    "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  7)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  8)
    "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 10)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 12)
    "nop"                      "\n\t" // 1    nop           (T = 13)
    "st   %a[port],  %[lo]"    "\n\t" // 2    PORT = lo     (T = 15)
    "nop"                      "\n\t" // 1    nop           (T = 16)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 18)
    "rjmp head20"              "\n\t" // 2    -> head20 (next bit out) (T=20)
   "nextbyte20:"               "\n\t" //                    (T = 10)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 11)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 13)
    "st   %a[port], %[lo]"     "\n\t" // 2    PORT = lo     (T = 15)
    "nop"                      "\n\t" // 1    nop           (T = 16)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 18)
     "brne head20"             "\n"   // 2    if(i != 0) -> (next byte) (T=20)
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

// 20 MHz(ish) AVRe/AVRe+ --------------------------------------------------
#elif (F_CPU >= 19000000UL) && (F_CPU <= 22000000L)
  // 25 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,7,15)
  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
     "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  7)
    "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  9)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  10)
    "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 12)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 14)
    "nop"                      "\n\t" // 1    nop           (T = 15)
    "st   %a[port],  %[lo]"    "\n\t" // 2    PORT = lo     (T = 17)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 19)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 21)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 23)
    "rjmp head20"              "\n\t" // 2    -> head20 (next bit out)
   "nextbyte20:"               "\n\t" //                    (T = 12)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 13)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 15)
    "st   %a[port], %[lo]"     "\n\t" // 2    PORT = lo     (T = 17)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 19)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 21)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 23)
     "brne head20"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

#else
 #error "CPU SPEED NOT SUPPORTED"
#endif

// END AVR ----------------------------------------------------------------

  interrupts();
  #ifndef DISABLEMILLIS
    endTime = micros(); // Save EOD time for latch on next call
  #else
    #warning "micros is not available based on timer settings. You must ensure at least 6 us, 50 us or 250us, depending on which of the identical looking LEDs you have."
  #endif
}

// Set the output pin number
void tinyNeoPixel::setPin(uint8_t p) {
    pin = p;
    port    = portOutputRegister(digitalPinToPort(p));
    pinMask = digitalPinToBitMask(p);
}

// Set pixel color from separate R,G,B components:
void tinyNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {

  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = 0;        // But only R,G,B passed -- set W to 0
    }
    p[rOffset] = r;          // R,G,B always stored
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

void tinyNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
      w = (w * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel (ignore W)
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = w;        // Store W
    }
    p[rOffset] = r;          // Store R,G,B
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void tinyNeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    uint8_t *p,
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    if(wOffset == rOffset) {
      p = &pixels[n * 3];
    } else {
      p = &pixels[n * 4];
      uint8_t w = (uint8_t)(c >> 24);
      p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
    }
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

void tinyNeoPixel::fill(uint32_t c, uint16_t first, uint16_t count) {
  uint16_t i, end;

  if(first >= numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if(count == 0) {
    // Fill to end of strip
    end = numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if(end > numLEDs) end = numLEDs;
  }

  for(i = first; i < end; i++) {
    this->setPixelColor(i, c);
  }
}


/*!
  @brief   Convert hue, saturation and value into a packed 32-bit RGB color
           that can be passed to setPixelColor() or other RGB-compatible
           functions.
  @param   hue  An unsigned 16-bit value, 0 to 65535, representing one full
                loop of the color wheel, which allows 16-bit hues to "roll
                over" while still doing the expected thing (and allowing
                more precision than the wheel() function that was common to
                prior NeoPixel examples).
  @param   sat  Saturation, 8-bit value, 0 (min or pure grayscale) to 255
                (max or pure hue). Default of 255 if unspecified.
  @param   val  Value (brightness), 8-bit value, 0 (min / black / off) to
                255 (max or full brightness). Default of 255 if unspecified.
  @return  Packed 32-bit RGB with the most significant byte set to 0 -- the
           white element of WRGB pixels is NOT utilized. Result is linearly
           but not perceptually correct, so you may want to pass the result
           through the gamma32() function (or your own gamma-correction
           operation) else colors may appear washed out. This is not done
           automatically by this function because coders may desire a more
           refined gamma-correction function than the simplified
           one-size-fits-all operation of gamma32(). Diffusing the LEDs also
           really seems to help when using low-saturation colors.
*/
uint32_t tinyNeoPixel::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {

  uint8_t r, g, b;

  /* Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
   * 0 is not the start of pure red, but the midpoint...a few values above
   * zero and a few below 65536 all yield pure red (similarly, 32768 is the
   * midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
   * each for red, green, blue) really only allows for 1530 distinct hues
   * (not 1536, more on that below), but the full unsigned 16-bit type was
   * chosen for hue so that one's code can easily handle a contiguous color
   * wheel by allowing hue to roll over in either direction.
   */
  hue = (hue * 1530L + 32768) / 65536;
  /* Because red is centered on the rollover point (the +32768 above,
   * essentially a fixed-point +0.5), the above actually yields 0 to 1530,
   * where 0 and 1530 would yield the same thing. Rather than apply a
   * costly modulo operator, 1530 is handled as a special case below.
   *
   * So you'd think that the color "hexcone" (the thing that ramps from
   * pure red, to pure yellow, to pure green and so forth back to red,
   * yielding six slices), and with each color component having 256
   * possible values (0-255), might have 1536 possible items (6*256),
   * but in reality there's 1530. This is because the last element in
   * each 256-element slice is equal to the first element of the next
   * slice, and keeping those in there this would create small
   * discontinuities in the color wheel. So the last element of each
   * slice is dropped...we regard only elements 0-254, with item 255
   * being picked up as element 0 of the next slice. Like this:
   * Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
   * Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
   * Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
   * and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
   * the constants below are not the multiples of 256 you might expect.
   */
  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if(hue < 510) {         // Red to Green-1
    b = 0;
    if(hue < 255) {       //   Red to Yellow-1
      r = 255;
      g = hue;            //     g = 0 to 254
    } else {              //   Yellow to Green-1
      r = 510 - hue;      //     r = 255 to 1
      g = 255;
    }
  } else if(hue < 1020) { // Green to Blue-1
    r = 0;
    if(hue <  765) {      //   Green to Cyan-1
      g = 255;
      b = hue - 510;      //     b = 0 to 254
    } else {              //   Cyan to Blue-1
      g = 1020 - hue;     //     g = 255 to 1
      b = 255;
    }
  } else if(hue < 1530) { // Blue to Red-1
    g = 0;
    if(hue < 1275) {      //   Blue to Magenta-1
      r = hue - 1020;     //     r = 0 to 254
      b = 255;
    } else {              //   Magenta to Red-1
      r = 255;
      b = 1530 - hue;     //     b = 255 to 1
    }
  } else {                // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 =   1 + val; // 1 to 256; allows >>8 instead of /255
  uint16_t s1 =   1 + sat; // 1 to 256; same reason
  uint8_t  s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
          (((((g * s1) >> 8) + s2) * v1) & 0xff00)       |
          (((((b * s1) >> 8) + s2) * v1)           >> 8);
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t tinyNeoPixel::getPixelColor(uint16_t n) const {
  if(n >= numLEDs) return 0; // Out of bounds, return no color.

  uint8_t *p;

  if(wOffset == rOffset) { // Is RGB-type device
    p = &pixels[n * 3];
    if(brightness) {
      /* Stored color was decimated by setBrightness().  Returned value
       * attempts to scale back to an approximation of the original 24-bit
       * value used when setting the pixel color, but there will always be
       * some error -- those bits are simply gone.  Issue is most
       * pronounced at low brightness levels.
       */
      return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  } else {                 // Is RGBW-type device
    p = &pixels[n * 4];
    if(brightness) { // Return scaled color
      return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) |
             (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else { // Return raw color
      return ((uint32_t)p[wOffset] << 24) |
             ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  }
}

/* Returns pointer to pixels[] array.  Pixel data is stored in device-
 * native format and is not translated here.  Application will need to be
 * aware of specific pixel data format and handle colors appropriately.
 */
uint8_t *tinyNeoPixel::getPixels(void) const {
  return pixels;
}

uint16_t tinyNeoPixel::numPixels(void) const {
  return numLEDs;
}

/* Adjust output brightness; 0=darkest (off), 255=brightest.  This does
 * NOT immediately affect what's currently displayed on the LEDs.  The
 * next call to show() will refresh the LEDs at this level.  However,
 * this process is potentially "lossy," especially when increasing
 * brightness.  The tight timing in the WS2811/WS2812 code means there
 * aren't enough free cycles to perform this scaling on the fly as data
 * is issued.  So we make a pass through the existing color data in RAM
 * and scale it (subsequent graphics commands also work at this
 * brightness level).  If there's a significant step up in brightness,
 * the limited number of steps (quantization) in the old data will be
 * quite visible in the re-scaled version.  For a non-destructive
 * change, you'll need to re-render the full strip data.
 */
void tinyNeoPixel::setBrightness(uint8_t b) {
  /* Stored brightness value is different than what's passed.
   * This simplifies the actual scaling math later, allowing a fast
   * 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
   * adding 1 here may (intentionally) roll over...so 0 = max brightness
   * (color values are interpreted literally; no scaling), 1 = min
   * brightness (off), 255 = just below max brightness.
   */
  uint8_t newBrightness = b + 1;
  if(newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = pixels,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for(uint16_t i=0; i<numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}

//Return the brightness value
uint8_t tinyNeoPixel::getBrightness(void) const {
  return brightness - 1;
}

void tinyNeoPixel::clear() {
  memset(pixels, 0, numBytes);
}

// A 32-bit variant of gamma8() that applies the same function
// to all components of a packed RGB or WRGB value.
uint32_t tinyNeoPixel::gamma32(uint32_t x) {
  uint8_t *y = (uint8_t *)&x;
  /* All four bytes of a 32-bit value are filtered even if RGB (not WRGB),
   * to avoid a bunch of shifting and masking that would be necessary for
   * properly handling different endianisms (and each byte is a fairly
   * trivial operation, so it might not even be wasting cycles vs a check
   * and branch for the RGB case). In theory this might cause trouble *if*
   * someone's storing information in the unused most significant byte
   * of an RGB value, but this seems exceedingly rare and if it's
   * encountered in reality they can mask values going in or coming out.
   */
  for(uint8_t i=0; i<4; i++) y[i] = gamma8(y[i]);
  return x; // Packed 32-bit return
}
