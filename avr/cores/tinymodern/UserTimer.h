/*==============================================================================

  UserTimer.h - Veneer for the User Timer (same timer as the one used for 
      Tone)

  Copyright 2010 Rowdy Dog Software.

  This file is part of Arduino-Tiny.

  Arduino-Tiny is free software: you can redistribute it and/or modify it 
  under the terms of the GNU Lesser General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.

  Arduino-Tiny is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with Arduino-Tiny.  If not, see <http://www.gnu.org/licenses/>.

==============================================================================*/

#ifndef UserTimer_h
#define UserTimer_h

#include "core_build_options.h"
#include "core_timers.h"


/*=============================================================================
  Assume there are only two timers.  One for millis and one for everything 
  else.
=============================================================================*/

#if TIMER_TO_USE_FOR_MILLIS == 0
#define TIMER_TO_USE_FOR_USER                     1
#elif TIMER_TO_USE_FOR_MILLIS == 1
#define TIMER_TO_USE_FOR_USER                     0
#else
#error Unexpected condition in UserTimer.h.
#endif


/*=============================================================================
  Macros to help generate the macros below
=============================================================================*/

#define usertimer_(t)                             TIMER_PASTE_A( timer, TIMER_TO_USE_FOR_USER, t )
#define UserTimer_(f)                             TIMER_PASTE_A( Timer, TIMER_TO_USE_FOR_USER, f )
#define USERTIMER_(c)                             TIMER_PASTE_A( TIMER, TIMER_TO_USE_FOR_USER, c )


/*=============================================================================
  Macros to provide a veneer over the data-types, functions, and constants in 
  core_timers.h
=============================================================================*/

#define UserTimer_SetToPowerup                    UserTimer_(SetToPowerup)
#define UserTimer_InterruptsOff                   UserTimer_(InterruptsOff)
#define UserTimer_ClockSelect                     UserTimer_(ClockSelect)
#define UserTimer_SetWaveformGenerationMode       UserTimer_(SetWaveformGenerationMode)
#define UserTimer_SetCompareOutputModeA           UserTimer_(SetCompareOutputModeA)
#define UserTimer_SetCompareOutputModeB           UserTimer_(SetCompareOutputModeB)
#define UserTimer_SetOutputCompareMatchAndClear   UserTimer_(SetOutputCompareMatchAndClear)
#define UserTimer_EnableOutputCompareInterruptA   UserTimer_(EnableOutputCompareInterruptA)
#define UserTimer_EnableOverflowInterrupt         UserTimer_(EnableOverflowInterrupt)
#define UserTimer_GetCount                        UserTimer_(GetCount)
#define UserTimer_SetCount                        UserTimer_(SetCount)
#define UserTimer_IsOverflowSet                   UserTimer_(IsOverflowSet)

#define USERTIMER_OVF_vect                        USERTIMER_(OVF_vect)
#define USERTIMER_COMPA_vect                      USERTIMER_(COMPA_vect)
#define USERTIMER_COMPB_vect                      USERTIMER_(COMPB_vect)


#endif
