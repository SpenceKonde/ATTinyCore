/*==============================================================================

  core_adc.h - Veneer for the analog-to-digital converter.

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

#ifndef core_adc_h
#define core_adc_h

#include <avr/io.h>
#include <binary.h>

#include "core_build_options.h"
#include "core_macros.h"


/*=============================================================================
  Some common things
=============================================================================*/

#if  defined(__AVR_ATtiny1634__) || defined( __AVR_ATtinyX41__  ) || defined( __AVR_ATtiny828__ )

/*
  From the '841, '84, and '85 datasheets... By default, the successive approximation
  circuitry requires an input clock frequency between 50 kHz and 200 kHz to
  get maximum resolution.
*/

#if F_CPU > 12000000L
  // above 12mhz, prescale by 128, the highest prescaler available
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_128
#elif F_CPU >= 6000000L
  // 12 MHz / 64 ~= 188 KHz
  // 8 MHz / 64 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_64
#elif F_CPU >= 3000000L
  // 4 MHz / 32 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_32
#elif F_CPU >= 1500000L
  // 2 MHz / 16 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_16
#elif F_CPU >= 750000L
  // 1 MHz / 8 = 125 KHz
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_8
#elif F_CPU < 400000L
  // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_2
#else //speed between 400khz and 750khz
  #define ADC_ARDUINO_PRESCALER   ADC_Prescaler_Value_4 //prescaler of 4
#endif
typedef enum
{
  ADC_Prescaler_Value_2      = B001,
  ADC_Prescaler_Value_4      = B010,
  ADC_Prescaler_Value_8      = B011,
  ADC_Prescaler_Value_16     = B100,
  ADC_Prescaler_Value_32     = B101,
  ADC_Prescaler_Value_64     = B110,
  ADC_Prescaler_Value_128    = B111,
  ADC_Prescaler_Index_1      = B001,
  ADC_Prescaler_Index_2      = B010,
  ADC_Prescaler_Index_3      = B011,
  ADC_Prescaler_Index_4      = B100,
  ADC_Prescaler_Index_5      = B101,
  ADC_Prescaler_Index_6      = B110,
  ADC_Prescaler_Index_7      = B111
}
adc_ps_t;

__attribute__((always_inline)) static inline void ADC_PrescalerSelect( adc_ps_t ps )
{
  ADCSRA = (ADCSRA & ~MASK3(ADPS2,ADPS1,ADPS0)) | (ps << ADPS0);
}

__attribute__((always_inline)) static inline void ADC_Enable( void )
{
  ADCSRA |= MASK1( ADEN );
}

#endif


/*=============================================================================
  Veneer for the ATtiny841 ADC
=============================================================================*/

#if defined( __AVR_ATtinyX41__ )

typedef enum
{
  ADC_Reference_VCC                             = B000,
  ADC_Reference_Internal_1p1                    = B001,
  ADC_Reference_Internal_2p2                    = B010,
  ADC_Reference_Internal_4p096                  = B011,
  ADC_Reference_External                        = B100,
  ADC_Reference_Internal_1p1_aref               = B101,
  ADC_Reference_Internal_2p2_aref               = B110,
  ADC_Reference_Internal_4p096_aref             = B111
}
adc_vr_t;

__attribute__((always_inline)) static inline void ADC_SetVoltageReference( adc_vr_t vr )
{
  ADMUXB = (ADMUXB & ~MASK3(REFS2,REFS1,REFS0)) | (((vr & B111) >> 0) << REFS0);
}

typedef enum
{
  ADC_Input_ADC0            = B000000,
  ADC_Input_ADC1            = B000001,
  ADC_Input_ADC2            = B000010,
  ADC_Input_ADC3            = B000011,
  ADC_Input_ADC4            = B000100,
  ADC_Input_ADC5            = B000101,
  ADC_Input_ADC6            = B000110,
  ADC_Input_ADC7            = B000111,
  ADC_Input_ADC8            = B001000,
  ADC_Input_ADC9            = B001001,
  ADC_Input_ADC10           = B001010,
  ADC_Input_ADC11           = B001011,
  ADC_Input_temp            = B001100,  // For temperature sensor.
  ADC_Input_1p1             = B001101,  // 1.1V (I Ref)
  ADC_Input_GND             = B001110,  // 0V (AGND)
  ADC_Input_supply          = B001111,  // Supply voltage
  ADC_Diff_0_1              = B010000,
  ADC_Diff_0_3              = B010001,
  ADC_Diff_1_2              = B010010,
  ADC_Diff_1_3              = B010011,
  ADC_Diff_2_3              = B010100,
  ADC_Diff_3_4              = B010101,
  ADC_Diff_3_5              = B010110,
  ADC_Diff_3_6              = B010111,
  ADC_Diff_3_7              = B011000,
  ADC_Diff_4_5              = B011001,
  ADC_Diff_4_6              = B011010,
  ADC_Diff_4_7              = B011011,
  ADC_Diff_5_6              = B011100,
  ADC_Diff_5_7              = B011101,
  ADC_Diff_6_7              = B011110,
  ADC_Diff_8_9              = B011111,
  ADC_Diff_0_0              = B100000,
  ADC_Diff_1_1              = B100001,
  ADC_Diff_2_2              = B100010,
  ADC_Diff_3_3              = B100011,
  ADC_Diff_4_4              = B100100,
  ADC_Diff_5_5              = B100101,
  ADC_Diff_6_6              = B100110,
  ADC_Diff_7_7              = B100111,
  ADC_Diff_8_8              = B101000,
  ADC_Diff_9_9              = B101001,
  ADC_Diff_10_8             = B101010,
  ADC_Diff_10_9             = B101011,
  ADC_Diff_11_8             = B101100,
  ADC_Diff_11_9             = B101101,
  ADC_Diff_1_0              = B110000,
  ADC_Diff_3_0              = B110001,
  ADC_Diff_2_1              = B110010,
  ADC_Diff_3_1              = B110011,
  ADC_Diff_3_2              = B110100,
  ADC_Diff_4_3              = B110101,
  ADC_Diff_5_3              = B110110,
  ADC_Diff_6_3              = B110111,
  ADC_Diff_7_3              = B111000,
  ADC_Diff_5_4              = B111001,
  ADC_Diff_6_4              = B111010,
  ADC_Diff_7_4              = B111011,
  ADC_Diff_6_5              = B111100,
  ADC_Diff_7_5              = B111101,
  ADC_Diff_7_6              = B111110,
  ADC_Diff_9_8              = B111111,
}
adc_ic_t;

__attribute__((always_inline)) static inline void ADC_SetInputChannel( adc_ic_t ic )
{

  ADMUXA = (ADMUXA & ~MASK6(MUX5,MUX4,MUX3,MUX2,MUX1,MUX0)) | (ic << MUX0);

}

__attribute__((always_inline)) static inline void ADC_StartConversion( void )
{
  ADCSRA |= MASK1( ADSC );
}

__attribute__((always_inline)) static inline uint8_t ADC_ConversionInProgress( void )
{
  return( (ADCSRA & (1<<ADSC)) != 0 );
}

__attribute__((always_inline)) static inline uint16_t ADC_GetDataRegister( void )
{
  return( ADC );
}

#endif




/*=============================================================================
  Veneer for the ATtiny1634 ADC
=============================================================================*/
#if defined( __AVR_ATtiny1634__ )

typedef enum
{
  ADC_Reference_VCC                             = B00,
  ADC_Reference_External                        = B01,
  ADC_Reference_Internal_1p1                    = B10,
  ADC_Reference_Reserved_1                      = B11
}
adc_vr_t;

typedef enum
{
  ADC_Input_ADC0            = B0000,
  ADC_Input_ADC1            = B0001,
  ADC_Input_ADC2            = B0010,
  ADC_Input_ADC3            = B0011,
  ADC_Input_ADC4            = B0100,
  ADC_Input_ADC5            = B0101,
  ADC_Input_ADC6            = B0110,
  ADC_Input_ADC7            = B0111,
  ADC_Input_ADC8            = B1000,
  ADC_Input_ADC9            = B1001,
  ADC_Input_ADC10           = B1010,
  ADC_Input_ADC11           = B1011,

  ADC_Input_GND             = B1100,  // 0V (AGND)
  ADC_Input_1p1             = B1101,  // 1.1V (I Ref)
  ADC_Input_ADC12           = B1110,  // For temperature sensor.

}
adc_ic_t;

__attribute__((always_inline)) static inline void ADC_SetVoltageReference( adc_vr_t vr )
{
  ADMUX = (ADMUX & ~MASK2(REFS1,REFS0)) | (((vr & B11) >> 0) << REFS0);
}

__attribute__((always_inline)) static inline void ADC_SetInputChannel( adc_ic_t ic )
{
  ADMUX = (ADMUX & ~MASK4(MUX3,MUX2,MUX1,MUX0)) | (ic << MUX0);
}

__attribute__((always_inline)) static inline void ADC_StartConversion( void )
{
  ADCSRA |= MASK1( ADSC );
}

__attribute__((always_inline)) static inline uint8_t ADC_ConversionInProgress( void )
{
  return( (ADCSRA & (1<<ADSC)) != 0 );
}

__attribute__((always_inline)) static inline uint16_t ADC_GetDataRegister( void )
{
  return( ADC );
}

#endif

/*=============================================================================
  Veneer for the ATtiny828 ADC 
=============================================================================*/
#if defined( __AVR_ATtiny828__ )

typedef enum
{
  ADC_Reference_VCC                             = 0,
  ADC_Reference_Internal_1p1                    = 1,
}
adc_vr_t;

//Save resources - no need for typedef enum to 

__attribute__((always_inline)) static inline void ADC_SetVoltageReference( adc_vr_t vr )
{
  ADMUXB = (ADMUXB & ~MASK1(REFS)) | ((vr & 1) << REFS);
}

__attribute__((always_inline)) static inline void ADC_SetInputChannel( uint8_t ic )
{
  ADMUXA = (ADMUXA & ~MASK5(MUX4,MUX3,MUX2,MUX1,MUX0)) | (ic);
}

__attribute__((always_inline)) static inline void ADC_StartConversion( void )
{
  ADCSRA |= MASK1( ADSC );
}

__attribute__((always_inline)) static inline uint8_t ADC_ConversionInProgress( void )
{
  return( (ADCSRA & (1<<ADSC)) != 0 );
}

__attribute__((always_inline)) static inline uint16_t ADC_GetDataRegister( void )
{
  return( ADC );
}

#endif


#endif
