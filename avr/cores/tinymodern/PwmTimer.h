/*==============================================================================

  PwmTimer.h - Veneer for the PWM timers.

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

#ifndef PwmTimer_h
#define PwmTimer_h

#include "core_pins.h"
#include "core_timers.h"

#define PwmTimer3_(t,f,c)                         TIMER_PASTE_CHANNEL_A( Timer, t, f, c )
#define PwmTimer2_(t,f)                           TIMER_PASTE_A( Timer, t, f )

#if CORE_PWM_COUNT >= 1
#define Pwm0_SetCompareOutputMode                 PwmTimer3_( CORE_PWM0_TIMER, SetCompareOutputMode, CORE_PWM0_CHANNEL )
#define Pwm0_Disconnected                         PwmTimer2_( CORE_PWM0_TIMER, Disconnected )
#define Pwm0_Clear                                PwmTimer2_( CORE_PWM0_TIMER, Clear )
#define Pwm0_SetOutputCompareMatch                PwmTimer3_( CORE_PWM0_TIMER, SetOutputCompareMatch, CORE_PWM0_CHANNEL )
#endif

#if CORE_PWM_COUNT >= 2
#define Pwm1_SetCompareOutputMode                 PwmTimer3_( CORE_PWM1_TIMER, SetCompareOutputMode, CORE_PWM1_CHANNEL )
#define Pwm1_Disconnected                         PwmTimer2_( CORE_PWM1_TIMER, Disconnected )
#define Pwm1_Clear                                PwmTimer2_( CORE_PWM1_TIMER, Clear )
#define Pwm1_SetOutputCompareMatch                PwmTimer3_( CORE_PWM1_TIMER, SetOutputCompareMatch, CORE_PWM1_CHANNEL )
#endif

#if CORE_PWM_COUNT >= 3
#define Pwm2_SetCompareOutputMode                 PwmTimer3_( CORE_PWM2_TIMER, SetCompareOutputMode, CORE_PWM2_CHANNEL )
#define Pwm2_Disconnected                         PwmTimer2_( CORE_PWM2_TIMER, Disconnected )
#define Pwm2_Clear                                PwmTimer2_( CORE_PWM2_TIMER, Clear )
#define Pwm2_SetOutputCompareMatch                PwmTimer3_( CORE_PWM2_TIMER, SetOutputCompareMatch, CORE_PWM2_CHANNEL )
#endif

#if CORE_PWM_COUNT >= 4
#define Pwm3_SetCompareOutputMode                 PwmTimer3_( CORE_PWM3_TIMER, SetCompareOutputMode, CORE_PWM3_CHANNEL )
#define Pwm3_Disconnected                         PwmTimer2_( CORE_PWM3_TIMER, Disconnected )
#define Pwm3_Clear                                PwmTimer2_( CORE_PWM3_TIMER, Clear )
#define Pwm3_SetOutputCompareMatch                PwmTimer3_( CORE_PWM3_TIMER, SetOutputCompareMatch, CORE_PWM3_CHANNEL )
#endif

#if CORE_PWM_COUNT >= 5
#define Pwm4_SetCompareOutputMode                 PwmTimer3_( CORE_PWM4_TIMER, SetCompareOutputMode, CORE_PWM4_CHANNEL )
#define Pwm4_Disconnected                         PwmTimer2_( CORE_PWM4_TIMER, Disconnected )
#define Pwm4_Clear                                PwmTimer2_( CORE_PWM4_TIMER, Clear )
#define Pwm4_SetOutputCompareMatch                PwmTimer3_( CORE_PWM4_TIMER, SetOutputCompareMatch, CORE_PWM4_CHANNEL )
#endif

#if CORE_PWM_COUNT >= 6
#define Pwm5_SetCompareOutputMode                 PwmTimer3_( CORE_PWM5_TIMER, SetCompareOutputMode, CORE_PWM5_CHANNEL )
#define Pwm5_Disconnected                         PwmTimer2_( CORE_PWM5_TIMER, Disconnected )
#define Pwm5_Clear                                PwmTimer2_( CORE_PWM5_TIMER, Clear )
#define Pwm5_SetOutputCompareMatch                PwmTimer3_( CORE_PWM5_TIMER, SetOutputCompareMatch, CORE_PWM5_CHANNEL )
#endif

#if CORE_PWM_COUNT >= 7
#error Only 6 pins PWM are supported.  Add more macro defintions.
#endif

#endif
