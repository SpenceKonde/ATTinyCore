/*****************************************************************************
*
* Atmel Corporation
*
* File              : USI_TWI_Master.c
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
*                     the USI_TWI_Transceive() function. The transceive function
*                     returns a status byte, which can be used to evaluate the
*                     success of the transmission.
*
****************************************************************************/


#if __GNUC__
#include <avr/io.h>
#else
#include <inavr.h>
#include <ioavr.h>
#endif
#ifdef USIDR
#include "USI_TWI_Master.h"

unsigned char USI_TWI_Master_Transfer(unsigned char);
unsigned char USI_TWI_Master_Stop(void);

union USI_TWI_state {
  unsigned char errorState; // Can reuse the TWI_state for error states due to that it will not be need if there
                            // exists an error.
  struct {
    unsigned char addressMode : 1;
    unsigned char masterWriteDataMode : 1;
    unsigned char unused : 6;
  };
} USI_TWI_state;

/*---------------------------------------------------------------
 USI TWI single master initialization function
---------------------------------------------------------------*/
void USI_TWI_Master_Initialise(void)
{
  PORT_USI |= (1 << PIN_USI_SDA); // Enable pullup on SDA, to set high as released state.
  PORT_USI_CL |= (1 << PIN_USI_SCL); // Enable pullup on SCL, to set high as released state.

  DDR_USI_CL |= (1 << PIN_USI_SCL); // Enable SCL as output.
  DDR_USI |= (1 << PIN_USI_SDA); // Enable SDA as output.

  USIDR = 0xFF;                                           // Preload dataregister with "released level" data.
  USICR = (0 << USISIE) | (0 << USIOIE) |                 // Disable Interrupts.
          (1 << USIWM1) | (0 << USIWM0) |                 // Set USI in Two-wire mode.
          (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | // Software stobe as counter clock source
          (0 << USITC);
  USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | // Clear flags,
          (0x0 << USICNT0);                                             // and reset counter.
}

/*---------------------------------------------------------------
Use this function to get hold of the error message from the last transmission
---------------------------------------------------------------*/
unsigned char USI_TWI_Get_State_Info(void)
{
  return (USI_TWI_state.errorState); // Return error state.
}

/*---------------------------------------------------------------
 USI Transmit and receive function. LSB of first byte in data
 indicates if a read or write cycles is performed. If set a read
 operation is performed.

 Function generates (Repeated) Start Condition, sends address and
 R/W, Reads/Writes Data, and verifies/sends ACK.

 Success or error code is returned. Error codes are defined in
 USI_TWI_Master.h
---------------------------------------------------------------*/
unsigned char USI_TWI_Start_Transceiver_With_Data(unsigned char *msg, unsigned char msgSize) {
  return USI_TWI_Start_Transceiver_With_Data_Stop(msg, msgSize, TRUE);
}

/*---------------------------------------------------------------
 USI Transmit and receive function.

 Same as USI_TWI_Start_Transceiver_With_Data() but with an additional
 parameter that defines if a Stop Condition should be send at the end
 of the transmission.
---------------------------------------------------------------*/
#ifndef __GNUC__
__x // AVR compiler
#endif
    unsigned char
    USI_TWI_Start_Transceiver_With_Data_Stop(unsigned char *msg, unsigned char msgSize, unsigned char stop)
{
  unsigned char tempUSISR_8bit = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)
                                 |                 // Prepare register value to: Clear flags, and
                                 (0x0 << USICNT0); // set USI to shift 8 bits i.e. count 16 clock edges.
  unsigned char tempUSISR_1bit = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)
                                 |                 // Prepare register value to: Clear flags, and
                                 (0xE << USICNT0); // set USI to shift 1 bit i.e. count 2 clock edges.

  USI_TWI_state.errorState  = 0;
  USI_TWI_state.addressMode = TRUE;

#ifdef PARAM_VERIFICATION
  if (msg > (unsigned char *)RAMEND) // Test if address is outside SRAM space
  {
    USI_TWI_state.errorState = USI_TWI_DATA_OUT_OF_BOUND;
    return (FALSE);
  }
  if (msgSize <= 1) // Test if the transmission buffer is empty
  {
    USI_TWI_state.errorState = USI_TWI_NO_DATA;
    return (FALSE);
  }
#endif

#ifdef NOISE_TESTING // Test if any unexpected conditions have arrived prior to this execution.
  if (USISR & (1 << USISIF)) {
    USI_TWI_state.errorState = USI_TWI_UE_START_CON;
    return (FALSE);
  }
  if (USISR & (1 << USIPF)) {
    USI_TWI_state.errorState = USI_TWI_UE_STOP_CON;
    return (FALSE);
  }
  if (USISR & (1 << USIDC)) {
    USI_TWI_state.errorState = USI_TWI_UE_DATA_COL;
    return (FALSE);
  }
#endif

  if (!(*msg
        & (1 << TWI_READ_BIT))) // The LSB in the address byte determines if is a masterRead or masterWrite operation.
  {
    USI_TWI_state.masterWriteDataMode = TRUE;
  }

  /* Release SCL to ensure that (repeated) Start can be performed */
  PORT_USI_CL |= (1 << PIN_USI_SCL); // Release SCL.
  while (!(PIN_USI_CL & (1 << PIN_USI_SCL)))
    ; // Verify that SCL becomes high.
#ifdef TWI_FAST_MODE
  DELAY_T4TWI; // Delay for T4TWI if TWI_FAST_MODE
#else
  DELAY_T2TWI; // Delay for T2TWI if TWI_STANDARD_MODE
#endif

  /* Generate Start Condition */
  PORT_USI &= ~(1 << PIN_USI_SDA); // Force SDA LOW.
  DELAY_T4TWI;
  PORT_USI_CL &= ~(1 << PIN_USI_SCL); // Pull SCL LOW.
  PORT_USI |= (1 << PIN_USI_SDA);  // Release SDA.

#ifdef SIGNAL_VERIFY
  if (!(USISR & (1 << USISIF))) {
    USI_TWI_state.errorState = USI_TWI_MISSING_START_CON;
    return (FALSE);
  }
#endif

  /*Write address and Read/Write data */
  do {
    /* If masterWrite cycle (or initial address transmission)*/
    if (USI_TWI_state.addressMode || USI_TWI_state.masterWriteDataMode) {
      /* Write a byte */
      PORT_USI_CL &= ~(1 << PIN_USI_SCL);         // Pull SCL LOW.
      USIDR = *(msg++);                        // Setup data.
      USI_TWI_Master_Transfer(tempUSISR_8bit); // Send 8 bits on bus.

      /* Clock and verify (N)ACK from slave */
      DDR_USI &= ~(1 << PIN_USI_SDA); // Enable SDA as input.
      if (USI_TWI_Master_Transfer(tempUSISR_1bit) & (1 << TWI_NACK_BIT)) {
        if (USI_TWI_state.addressMode)
          USI_TWI_state.errorState = USI_TWI_NO_ACK_ON_ADDRESS;
        else
          USI_TWI_state.errorState = USI_TWI_NO_ACK_ON_DATA;
        return (FALSE);
      }
      USI_TWI_state.addressMode = FALSE; // Only perform address transmission once.
    }
    /* Else masterRead cycle*/
    else {
      /* Read a data byte */
      DDR_USI &= ~(1 << PIN_USI_SDA); // Enable SDA as input.
      *(msg++) = USI_TWI_Master_Transfer(tempUSISR_8bit);

      /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
      if (msgSize == 1) // If transmission of last byte was performed.
      {
        USIDR = 0xFF; // Load NACK to confirm End Of Transmission.
      } else {
        USIDR = 0x00; // Load ACK. Set data register bit 7 (output for SDA) low.
      }
      USI_TWI_Master_Transfer(tempUSISR_1bit); // Generate ACK/NACK.
    }
  } while (--msgSize); // Until all data sent/received.

  if (stop) {
    USI_TWI_Master_Stop(); // Send a STOP condition on the TWI bus.
  }

  /* Transmission successfully completed*/
  return (TRUE);
}

/*---------------------------------------------------------------
 Core function for shifting data in and out from the USI.
 Data to be sent has to be placed into the USIDR prior to calling
 this function. Data read, will be return'ed from the function.
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Transfer(unsigned char temp)
{
  USISR = temp;                                          // Set USISR according to temp.
                                                         // Prepare clocking.
  temp = (0 << USISIE) | (0 << USIOIE) |                 // Interrupts disabled
         (1 << USIWM1) | (0 << USIWM0) |                 // Set USI in Two-wire mode.
         (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | // Software clock strobe as source.
         (1 << USITC);                                   // Toggle Clock Port.
  do {
    DELAY_T2TWI;
    USICR = temp; // Generate positive SCL edge.
    while (!(PIN_USI_CL & (1 << PIN_USI_SCL)))
      ; // Wait for SCL to go high.
    DELAY_T4TWI;
    USICR = temp;                   // Generate negative SCL edge.
  } while (!(USISR & (1 << USIOIF))); // Check for transfer complete.

  DELAY_T2TWI;
  temp  = USIDR;                 // Read out data.
  USIDR = 0xFF;                  // Release SDA.
  DDR_USI |= (1 << PIN_USI_SDA); // Enable SDA as output.

  return temp; // Return the data from the USIDR
}

/*---------------------------------------------------------------
 Function for generating a TWI Stop Condition. Used to release
 the TWI bus.
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Stop(void)
{
  PORT_USI &= ~(1 << PIN_USI_SDA); // Pull SDA low.
  PORT_USI_CL |= (1 << PIN_USI_SCL);  // Release SCL.
  while (!(PIN_USI_CL & (1 << PIN_USI_SCL)))
    ; // Wait for SCL to go high.
  DELAY_T4TWI;
  PORT_USI |= (1 << PIN_USI_SDA); // Release SDA.
  DELAY_T2TWI;

#ifdef SIGNAL_VERIFY
  if (!(USISR & (1 << USIPF))) {
    USI_TWI_state.errorState = USI_TWI_MISSING_STOP_CON;
    return (FALSE);
  }
#endif

  return (TRUE);
}
#endif
