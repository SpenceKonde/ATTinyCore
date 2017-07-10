/*==============================================================================

  core_atomic.h - Veneer for atomicity.

  Copyright 2015 Nick Iaconis.

  This file is part of Arduino-Tiny-841.

  Arduino-Tiny-841 is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  Arduino-Tiny-841 is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Arduino-Tiny-841.  If not, see <http://www.gnu.org/licenses/>.

==============================================================================*/
#include <avr/interrupt.h>

#ifndef CoreAtomic_h
#define CoreAtomic_h

#define MAKE_ATOMIC(OP)   MAKE_ATOMIC2(OP)
#define MAKE_ATOMIC2(OP)  { /* save interrupt flag */ \
                            uint8_t SaveSREG = SREG; \
                            /* disable interrupts */ \
                            cli(); \
                            /* access the shared data */ \
                            OP ; \
                            /* restore the interrupt flag */ \
                            SREG = SaveSREG; \
                          }

#endif
