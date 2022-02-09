#ifdef ARDUINO_ARCH_AVR
  #include <Arduino.h>
  #include <tinyNeoPixel_Static.h>
  void _show_S(uint8_t *ptr, uint16_t data_length, volatile uint8_t port, uint8_t bitmask) {

    /*INDENT-OFF Astyle doesn;t like assembly*/
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

  // AVR MCUs --  Classic AVR (AVRe/AVRe+) -------------------------------

    volatile uint16_t
      i   = data_length; // Loop counter
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
     * ST instructions:              ^ ^    ^   (T = 0,2,7)
     * Diagram assumes ST takes effect at end of the 2 clocks, but it
     * doesn't matter, as long as they're all the same.
     * And as if all that wasn't good enough, we also save 8 words of flash!
     */
    volatile uint8_t n1, n2 = 0;  // First, next bits out
    hi = (*port) |  bitmask;
    lo = (*port) & ~bitmask;
    n1 = lo;
    if (b & 0x80) n1 = hi;
    // And as if all that wasn't good enough, we also save 8 words of flash!
    asm volatile(
     "headD:"                   "\n\t" // Clk  Pseudocode
      // Bit 7: 1.25 us                // on target
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 6"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "nop"                     "\n\t" // 2   T = 10   nop
      // Bit 6: 1.25 us                // on target
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n2]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 5"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "nop"                     "\n\t" // 2   T = 10   nop
      // Bit 5: 1.25 us                // on target
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 4"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "nop"                     "\n\t" // 2   T = 10   nop
      // Bit 4: 1.25 us                // on target
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n2]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 3"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "nop"                     "\n\t" // 2   T = 10   nop
      // Bit 3: 1.25 us                // on target
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 2"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "subi %A[count], 1"       "\n\t" // 1   T = 10  i-- part 1
      // Bit 2: 1.25 us                // on target
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n2]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 1"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "sbci %B[count], 0"       "\n\t" // 1   T = 10  i-- part 2 - carrying clears 0 flag unless this is also 0. (don't act on Z flag yet)
      // Bit 1: 1.375 us               // 1 clock over
      "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st  %a[port] , %[n1]"    "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 0"        "\n\t" // 1-2 T = 6   if (b & 0x40)
       "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 7   n2 = hi
      "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
      "ld   %[byte] , %a[ptr]+" "\n\t" // 2   T = 11  b = *ptr++ load next byte
      // Bit 0: 1.375 us               // 1 clock over
      "st   %a[port] , %[hi]"   "\n\t" // 2   T = 2   PORT = hi   0 and 2
      "st   %a[port] , %[n2]"   "\n\t" // 2   T = 4   PORT = n1 - 250 ns zero
      "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 5   n2   = lo -
      "sbrc %[byte] , 7"        "\n\t" // 1-2 T = 6   if (b & 0x80)
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
       * ST  instructions:              ^   ^   ^   (T = 0,3,8)
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
      if (b & 0x80) n1 = hi;

      asm volatile(
       "headD:"                   "\n\t" // Clk  Pseudocode
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 6"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
        "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
        "nop"                     "\n\t" // 2   T = 13  nop
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n2]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 5"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
        "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
        "nop"                     "\n\t" // 2   T = 13  nop
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 4"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
        "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
        "nop"                     "\n\t" // 2   T = 13  nop
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n2]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 3"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
        "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
        "nop"                     "\n\t" // 2   T = 13  nop
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 2"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10  PORT = lo
        "rjmp .+0"                "\n\t" // 2   T = 12  nop nop
        "nop"                     "\n\t" // 2   T = 13  nop
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n2]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 1"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n1]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 10   PORT = lo
        "sbiw %[count], 1"        "\n\t" // 2   T = 12  i--
        "nop"                     "\n\t" // 2   T = 13  nop
        "st  %a[port] , %[hi]"    "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st  %a[port] , %[n1]"    "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n2]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 0"        "\n\t" // 1-2 T = 7   if (b & 0x40)
         "mov %[n2]   , %[hi]"    "\n\t" // 0-1 T = 8   n2 = hi
        "st  %a[port] , %[lo]"    "\n\t" // 2   T = 9   PORT = lo
        "ld   %[byte] , %a[ptr]+" "\n\t" // 2   T = 11  b = *ptr++ load next byte
        "nop"                     "\n\t" // 2   T = 13   nop
        "st   %a[port] , %[hi]"   "\n\t" // 2   T = 2   PORT = hi   0 and 2
        "nop"                     "\n\t" // 2   T = 3   nop
        "st   %a[port] , %[n2]"   "\n\t" // 2   T = 5   PORT = n1 - 250 ns zero
        "mov  %[n1]   , %[lo]"    "\n\t" // 1   T = 6   n2   = lo -
        "sbrc %[byte] , 7"        "\n\t" // 1-2 T = 7   if (b & 0x80)
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
       * ST  instructions:              ^   ^     ^     (T = 0,4,10)
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
        if (b & 0x80) next = hi;
        bit  = 7;
        asm volatile(
          "headD:"                    "\n\t" // Clk  Pseudocode    (T = 14)
            "nop"                     "\n\t" // 1    nop,for last  (T = 15)
            "st  %a[port], %[hi]"     "\n\t" //                    (T =  2)
            "nop"                     "\n\t" // 1                  (T =  3)
            "rol  %[byte]"            "\n\t" // 1    b <<= 1       (T =  4)
            "st  %a[port], %[next]"   "\n\t" // 1    PORT = next   (T =  6)
            "mov  %[next], %[lo]"     "\n\t" // 1    next = lo     (T =  7)
            "sbrc %[byte], 7"         "\n\t" // 1-2  if (b & 0x80)  (T =  8)
             "mov %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  9)
            "dec  %[bit]"             "\n\t" // 1    bit--         (T = 10)
            "st  %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 12)
            "brne headD"              "\n\t" // 1-2  1 if false    (T = 13)
            "ld   %[byte] , %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 15)
            "st  %a[port], %[hi]"     "\n\t" //                     (T =  2)
            "sbiw %[count], 1"        "\n\t" // 2    i--            (T =  4)
            "st  %a[port] , %[next]"  "\n\t" // 2    PORT = hi      (T =  6)
            "mov  %[next] , %[lo]"    "\n\t" // 1    next = lo      (T =  7)
            "sbrc %[byte] , 7"        "\n\t" // 1-2  if (b & 0x80)   (T =  8)
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
       * ST  instructions:              ^    ^      ^     (T = 0,5,12)
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
        if (b & 0x80) next = hi;
        bit  = 7;
        asm volatile(
          "headD:"                    "\n\t" // Clk  Pseudocode    (T = 17)
            "nop"                     "\n\t" // 1    nop,for last  (T = 18)
            "st  %a[port], %[hi]"     "\n\t" //      PORT = hi     (T =  2)
            "rjmp .+0"                "\n\t" // 2    nop nop       (T =  4)
            "rol  %[byte]"            "\n\t" // 1    b <<= 1       (T =  5)
            "st  %a[port], %[next]"   "\n\t" // 1    PORT = next   (T =  7)
            "mov  %[next], %[lo]"     "\n\t" // 1    next = lo     (T =  8)
            "sbrc %[byte], 7"         "\n\t" // 1-2  if (b & 0x80)  (T =  9)
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
            "sbrc %[byte] , 7"        "\n\t" // 1-2  if (b & 0x80)   (T =  8)
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
    // ST instructions:         ^    ^       ^       (T = 0,5,13)
    volatile uint8_t next, bit;

    hi   = *port |  pinMask;
    lo   = *port & ~pinMask;
    next = lo;
    bit  = 8;

    asm volatile(
     "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
      "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
      "sbrc %[byte],  7"         "\n\t" // 1-2  if (b & 128)
       "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
      "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
      "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  7)
      "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  8)
      "breq nextbyte20"          "\n\t" // 1-2  if (bit == 0) (from dec above)
      "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 10)
      "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 12)
      "nop"                      "\n\t" // 1    nop           (T = 13)
      "st   %a[port],  %[lo]"    "\n\t" // 2    PORT = lo     (T = 15)
      "nop"                      "\n\t" // 1    nop           (T = 16)
      "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 18)
      "rjmp head20"              "\n\t" // 2    -> head20 (next bit out) (T = 20)
     "nextbyte20:"               "\n\t" //                    (T = 10)
      "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 11)
      "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 13)
      "st   %a[port], %[lo]"     "\n\t" // 2    PORT = lo     (T = 15)
      "nop"                      "\n\t" // 1    nop           (T = 16)
      "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 18)
       "brne head20"             "\n"   // 2    if (i != 0) -> (next byte) (T = 20)
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
    // ST instructions:         ^      ^       ^       (T = 0,7,15)
    volatile uint8_t next, bit;

    hi   = *port |  pinMask;
    lo   = *port & ~pinMask;
    next = lo;
    bit  = 8;

    asm volatile(
     "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
      "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
      "sbrc %[byte],  7"         "\n\t" // 1-2  if (b & 128)
       "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
      "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
      "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  7)
      "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  9)
      "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  10)
      "breq nextbyte20"          "\n\t" // 1-2  if (bit == 0) (from dec above)
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
       "brne head20"             "\n"   // 2    if (i != 0) -> (next byte)  ()
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
  /*INDENT-ON */
  }
#endif
