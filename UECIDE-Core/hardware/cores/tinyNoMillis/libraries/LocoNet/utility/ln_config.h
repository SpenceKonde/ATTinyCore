#ifndef _LN_CONFIG_H_INCLUDED
#define _LN_CONFIG_H_INCLUDED

/****************************************************************************
 * Copyright (C) 2004 Alex Shepherd
 * 
 * Portions Copyright (C) Digitrax Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 ****************************************************************************/

// The RX port *MUST BE* the ICP pin (port PINB bit PB0, Arduino pin 8 on a '168) 

#define LN_RX_PORT  PINB

#ifdef PB0
#define LN_RX_BIT   PB0
#else
#define LN_RX_BIT   PORTB0
#endif

#define LN_RX_DDR   DDRB

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#undef  LN_SW_UART_TX_NON_INVERTED  // Normal is to be inverted...
#define LN_BIT_PERIOD  (F_CPU / 16666)

// From sysdef.h:
#define LN_SB_SIGNAL          TIMER1_CAPT_vect
#if defined(TIMSK)
#define LN_SB_INT_ENABLE_REG  TIMSK
#define LN_SB_INT_ENABLE_BIT  TICIE1
#define LN_SB_INT_STATUS_REG  TIFR
#define LN_SB_INT_STATUS_BIT  ICF1
#else
#define LN_SB_INT_ENABLE_REG  TIMSK1
#define LN_SB_INT_ENABLE_BIT  ICIE1
#define LN_SB_INT_STATUS_REG  TIFR1
#define LN_SB_INT_STATUS_BIT  ICF1
#endif
    
#define LN_TMR_SIGNAL         TIMER1_COMPA_vect
#if defined(TIMSK)
#define LN_TMR_INT_ENABLE_REG TIMSK
#define LN_TMR_INT_STATUS_REG TIFR
#else
#define LN_TMR_INT_ENABLE_REG TIMSK1
#define LN_TMR_INT_STATUS_REG TIFR1
#endif
#define LN_TMR_INT_ENABLE_BIT OCIE1A
#define LN_TMR_INT_STATUS_BIT OCF1A
#define LN_TMR_INP_CAPT_REG   ICR1      // [BA040319] added defines for:
#define LN_TMR_OUTP_CAPT_REG  OCR1A     // ICR1, OCR1A, TCNT1, TCCR1B
#define LN_TMR_COUNT_REG      TCNT1     // and replaced their occurence in
#define LN_TMR_CONTROL_REG    TCCR1B    // the code.
    
#define LN_TMR_PRESCALER      1
  
#define LN_TIMER_TX_RELOAD_ADJUST   106 //  14,4 us delay borrowed from FREDI sysdef.h

#define LN_INIT_COMPARATOR() { TCCR1A = 0; TCCR1B = 0x01; }    // no prescaler, normal mode

#define LN_TX_RETRIES_MAX  25

#endif
