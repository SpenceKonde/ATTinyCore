/*==============================================================================

  ToneTimer.h - Veneer for the Tone Timer.

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

#ifndef ToneTimer_h
#define ToneTimer_h

#include "core_build_options.h"
#include "core_timers.h"

#define tonetimer_(t)                             TIMER_PASTE_A( timer, TIMER_TO_USE_FOR_TONE, t )
#define ToneTimer_(f)                             TIMER_PASTE_A( Timer, TIMER_TO_USE_FOR_TONE, f )
#define TONETIMER_(c)                             TIMER_PASTE_A( TIMER, TIMER_TO_USE_FOR_TONE, c )

#define ToneTimer_SetToPowerup                    ToneTimer_(SetToPowerup)
#define ToneTimer_SetWaveformGenerationMode       ToneTimer_(SetWaveformGenerationMode)
#define ToneTimer_OutputComparePinA               ToneTimer_(OutputComparePinA)
#define ToneTimer_OutputComparePinB               ToneTimer_(OutputComparePinB)
#define ToneTimer_SetCompareOutputModeA           ToneTimer_(SetCompareOutputModeA)
#define ToneTimer_SetCompareOutputModeB           ToneTimer_(SetCompareOutputModeB)
#define ToneTimer_DisconnectOutputs               ToneTimer_(DisconnectOutputs)
#define ToneTimer_ClockSelect                     ToneTimer_(ClockSelect)
#define ToneTimer_SetCount                        ToneTimer_(SetCount)
#define TONETIMER_NUMBER_PRESCALERS               TONETIMER_(NUMBER_PRESCALERS)
#define ToneTimer_SetOutputCompareMatchAndClear   ToneTimer_(SetOutputCompareMatchAndClear)
#define ToneTimer_InterruptsOff                   ToneTimer_(InterruptsOff)
#define ToneTimer_EnableOutputCompareInterruptA   ToneTimer_(EnableOutputCompareInterruptA)
#define TONETIMER_COMPA_vect                      TONETIMER_(COMPA_vect)
#define TONETIMER_SUPPORTS_PHASE_CORRECT_PWM      TONETIMER_(SUPPORTS_PHASE_CORRECT_PWM)

#endif
