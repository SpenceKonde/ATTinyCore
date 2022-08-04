/*
* Modifications to use constants defined by ATTinyCore by Spence Konde
* 2017-2021, and for assorted minor changes, including eliminating IAR stuff
* Code is still closely based on Atmel code as described below:
****************************************************************************
*
* Atmel Corporation
*
* File              : USI_TWI_Master.h
* Date              : $Date: 2016-7-15 $
* Updated by        : $Author: Atmel $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All device with USI module can be used.
*                     The example is written for the ATmega169, ATtiny26 and ATtiny2313
*
* AppNote           : AVR310 - Using the USI module as a TWI Master
*
* Description       : This is an implementation of an TWI master using
*                     the USI module as basis. The implementation assumes the AVR to
*                     be the only TWI master in the system and can therefore not be
*                     used in a multi-master system.
* Usage             : Initialize the USI module by calling the USI_TWI_Master_Initialise()
*                     function. Hence messages/data are transceived on the bus using
*                     the USI_TWI_Start_Transceiver_With_Data() function. If the transceiver
*                     returns with a fail, then use USI_TWI_Get_Status_Info to evaluate the
*                     cause of the failure.
*
****************************************************************************/

#include <avr/io.h>
#include <Arduino.h>
#ifndef TWDR
#include <util/delay_basic.h>

//********** Defines **********//
// Generalized way of getting a correct delay that is acceptable by standard or fast mode I2C


#define usToDelayLoopIter(us,usdivisor,roundfactor) ((((((us) * F_CPU)/(usdivisor))/3)+(roundfactor))/1000000)
// this defines the number of passes through 3-clock simple loop that will give the requested runtime, in us.
// The factor of 1 million is left in there until the very end to prevent integer math effects from building on each other.
// Rounding factor is the amounnt added after the rest of calculationr to the intermediate value before dividing by 1 million

#define CLKBASE ((F_CPU)
#define T2_TWI (usToDelayLoopIter(5,1,500000)) //5us
#define T4_TWI (usToDelayLoopIter(4,1,500000)) //4us
// these all err in the direction of being too slow, which is fine
#define T2_TWI_FM (usToDelayLoopIter(3,2,500000)) //1.5us = 3/2 -> 3 is us passed, 2 is divisor
#define T4_TWI_FM (usToDelayLoopIter(3,4,750000)) //0.75us = 3/4th -> 3 is the us passed, 4 us divisor.


// Defines controlling code generating
//#define PARAM_VERIFICATION
//#define NOISE_TESTING
#define SIGNAL_VERIFY

// USI_TWI messages and flags and bit masks
//#define SUCCESS   7
//#define MSG       0
/****************************************************************************
  Bit and byte definitions
****************************************************************************/
#define TWI_READ_BIT 0 // Bit position for R/W bit in "address byte".
#define TWI_ADR_BITS 1 // Bit position for LSB of the slave address bits in the init byte.
#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

#define USI_TWI_NO_DATA 0x00           // Transmission buffer is empty
#define USI_TWI_DATA_OUT_OF_BOUND 0x01 // Transmission buffer is outside SRAM space
#define USI_TWI_UE_START_CON 0x02      // Unexpected Start Condition
#define USI_TWI_UE_STOP_CON 0x03       // Unexpected Stop Condition
#define USI_TWI_UE_DATA_COL 0x04       // Unexpected Data Collision (arbitration)
#define USI_TWI_NO_ACK_ON_DATA 0x05    // The slave did not acknowledge all data
#define USI_TWI_NO_ACK_ON_ADDRESS 0x06 // The slave did not acknowledge the address
#define USI_TWI_MISSING_START_CON 0x07 // Generated Start Condition not detected on bus
#define USI_TWI_MISSING_STOP_CON 0x08  // Generated Stop Condition not detected on bus


// #include "pins_arduino.h"
// This is already pulled in by Arduino.h on ATTinyCore. If it's not coming through correctly via Arduino.h, we're fucked anyway.
// because this library **requires** the defines derived from it.

// General defines
#define TRUE 1
#define FALSE 0

#define DELAY_T2TWI_FM (_delay_loop_1(T2_TWI_FM))
#define DELAY_T4TWI_FM (_delay_loop_1(T4_TWI_FM))
#define DELAY_T2TWI (_delay_loop_1(T2_TWI))
#define DELAY_T4TWI (_delay_loop_1(T4_TWI))
//********** Prototypes **********//

void USI_TWI_Master_Initialise(void);
void USI_TWI_Master_Speed(uint8_t);
unsigned char USI_TWI_Start_Transceiver_With_Data_Stop(unsigned char *, unsigned char, unsigned char);
unsigned char USI_TWI_Start_Transceiver_With_Data(unsigned char *, unsigned char);
unsigned char USI_TWI_Get_State_Info(void);
#endif
