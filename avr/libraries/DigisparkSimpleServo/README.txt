==================
SimpleServo - v0.8
==================

"Shh, ya hear that?  Me neither… It's simple… Maybe *too* simple…"

This is a simplistic library to bundle up and generalize bit-banging servo PWM.  It has some good points and trade-offs and is primarily being developed for the Digispark.

First the good: 
  - Can control any number of servos
  - All software, no fancy hardware required
  - Produces a relatively clean signal
  - Allows (or will) tuning pulse parameters to suit the widely varied tastes of different servos
  - A single instance can be used with multiple similar servos

Now some limitations:
  - It can only control one servo at a time
  - The program can't do anything else while the servo is being signaled 
  - Documentation is incomplete (at the moment)

When typical servos stop receiving a control signal, they stay where they are; while they won't actively hold their position, most require some force to backdrive.  So if that's enough for your project, you can move each servo in turn, and sample inputs, set LEDs and such in between.  

=======
Methods
=======

See comments in SimpleServo.h (and SimpleServo.m) for now.

*(( TODO: Document the additional methods in detail, add an example ))*

=======
License
=======

The MIT License (MIT)
Copyright (c) 2013 Benjamin Holt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
