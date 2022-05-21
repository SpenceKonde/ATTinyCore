/*
* Modifications to use constants defined by ATTinyCore by Spence Konde
* 2017-2021, and for assorted minor changes, including eliminating IAR stuff
* Code is still closely based on Atmel code as described below:
****************************************************************************
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


#include <avr/io.h>

#ifdef USIDR
#include "USI_TWI_Master.h"

unsigned char USI_TWI_Master_Transfer(unsigned char);
unsigned char USI_TWI_Master_Stop(void);
static unsigned char USI_TWI_MASTER_SPEED = 0;

union USI_TWI_state {
  unsigned char errorState; // Can reuse the TWI_state for error states due to that it will not be need if there
                            // exists an error.
  struct {
    unsigned char addressMode : 1;
    unsigned char masterWriteDataMode : 1;
    unsigned char unused : 6;
  };
} USI_TWI_state;

void USI_TWI_Master_Speed(uint8_t fm) {
  USI_TWI_MASTER_SPEED = fm ? 1 : 0;
}

/*---------------------------------------------------------------------------
 USI TWI single master initialization function
---------------------------------------------------------------------------*/
/* The pin defines are from pins_arduino.h (in abbreviated form)*
 * which is then used to generate the full suite of defines     *
 * in Arduino.h, from whence pins_arduino.h was included.       */
void USI_TWI_Master_Initialise(void) {
  #ifdef USI_PUE                                          // Turn on the pullups if it's the single example of a chip with USI and a PUE register (the 1634)
  USI_PUE         |= (1 << USI_DI_BIT);                   // Pullup on SDA
  USI_CLOCK_PUE   |= (1 << USI_CLOCK_BIT);                // Pullup on SCL - USI_CLOCK_* defines are to handle the case of SCL being on a different port than SDA, which is true on....
  #endif                                                  // *drumroll* The 1634 - and nothing else!
  USI_PORT        |= (1 << USI_DI_BIT);                   // Enable pullup on SDA, to set high as released state.
  USI_CLOCK_PORT  |= (1 << USI_CLOCK_BIT);                // Enable pullup on SCL, to set high as released state.

  USI_CLOCK_DDR   |= (1 << USI_CLOCK_BIT);                // Enable SCL as output.
  USI_DDR         |= (1 << USI_DI_BIT);                   // Enable SDA as output.

  USIDR = 0xFF;                                           // Preload dataregister with "released level" data.
  USICR = (0 << USISIE) | (0 << USIOIE) |                 // Disable Interrupts.
          (1 << USIWM1) | (0 << USIWM0) |                 // Set USI in Two-wire mode.
          (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | // Software stobe as counter clock source
          (0 << USITC);
  USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF)  | // Clear flags,
          (1 << USIDC)  | (0x0 << USICNT0);               // and reset counter.
}

/*---------------------------------------------------------------
Use this function to get hold of the error message from the last transmission
---------------------------------------------------------------*/
unsigned char USI_TWI_Get_State_Info(void) {
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

unsigned char USI_TWI_Start_Transceiver_With_Data_Stop(unsigned char *msg, unsigned char msgSize, unsigned char stop) {
  unsigned char tempUSISR_8bit = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | (0x0 << USICNT0);
  // Prepare register value to: Clear flags set USI to shift 8 bits i.e. count 16 clock edges.

  unsigned char tempUSISR_1bit = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | (0xE << USICNT0);
  // Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.

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

  if (!(*msg & (1 << TWI_READ_BIT))) // The LSB in the address byte determines if is a masterRead or masterWrite operation.
  {
    USI_TWI_state.masterWriteDataMode = TRUE;
  }
  /* Release SCL to ensure that (repeated) Start can be performed */
  USI_CLOCK_PORT |= (1 << USI_CLOCK_BIT);           // Release SCL.
  while (!(USI_CLOCK_PIN & (1 << USI_CLOCK_BIT)));  // Verify that SCL becomes high.
  if (USI_TWI_MASTER_SPEED) DELAY_T4TWI_FM; else DELAY_T2TWI;          // Delay for T4TWI if TWI_FAST_MODE,  T2TWI if TWI_STANDARD_MODE
                                                    /* Generate Start Condition */
  USI_PORT &= ~(1 << USI_DI_BIT);                   // Force SDA LOW.
  if (USI_TWI_MASTER_SPEED) DELAY_T4TWI_FM; else DELAY_T4TWI;
  USI_CLOCK_PORT &= ~(1 << USI_CLOCK_BIT);          // Pull SCL LOW.
  USI_PORT |= (1 << USI_DI_BIT);                    // Release SDA.
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
      USI_CLOCK_PORT &= ~(1 << USI_CLOCK_BIT);  // Pull SCL LOW.
      USIDR = *(msg++);                         // Setup data.
      USI_TWI_Master_Transfer(tempUSISR_8bit);  // Send 8 bits on bus.
                                                /* Clock and verify (N)ACK from slave */
      USI_DDR &= ~(1 << USI_DI_BIT);            // Enable SDA as input.
      if (USI_TWI_Master_Transfer(tempUSISR_1bit) & (1 << TWI_NACK_BIT)) {
        if (USI_TWI_state.addressMode) {
          USI_TWI_state.errorState = USI_TWI_NO_ACK_ON_ADDRESS;
        } else {
          USI_TWI_state.errorState = USI_TWI_NO_ACK_ON_DATA;
        }
        return (FALSE);
      }
      USI_TWI_state.addressMode = FALSE; // Only perform address transmission once.
    } else {                /* Else masterRead cycle - Read a data byte */
      USI_DDR &= ~(1 << USI_DI_BIT);           // Enable SDA as input.
      *(msg++) = USI_TWI_Master_Transfer(tempUSISR_8bit);
                            /* Prepare to generate ACK (or NACK) */
      if (msgSize == 1) {   // If transmission of last byte was performed.
        USIDR = 0xFF;       // Load NACK to confirm End Of Transmission.
      } else {
        USIDR = 0x00;       // Load ACK. Set data register bit 7 (output for SDA) low.
      }
      USI_TWI_Master_Transfer(tempUSISR_1bit);      // Generate ACK/NACK.
    }
  } while (--msgSize);      // repeat until all data sent/received.

  if (stop) {
    USI_TWI_Master_Stop();  // Send a STOP condition on the TWI bus.
  }
  return (TRUE);            /* Transmission successfully completed */
}

/*---------------------------------------------------------------
 Core function for shifting data in and out from the USI.
 Data to be sent has to be placed into the USIDR prior to calling
 this function. Data read, will be return'ed from the function.
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Transfer(unsigned char temp) {
  USISR = temp;                                          // Set USISR according to temp.
                                                         // Prepare clocking.
  temp = (0 << USISIE) | (0 << USIOIE) |                 // Interrupts disabled
         (1 << USIWM1) | (0 << USIWM0) |                 // Set USI in Two-wire mode.
         (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | // Software clock strobe as source.
         (1 << USITC);                                   // Toggle Clock Port.
  do {
    if (USI_TWI_MASTER_SPEED) DELAY_T2TWI_FM; else DELAY_T2TWI;
    USICR = temp;                                        // Generate positive SCL edge.
    while (!(USI_CLOCK_PIN & (1 << USI_CLOCK_BIT)));        // Wait for SCL to go high.
    if (USI_TWI_MASTER_SPEED) DELAY_T4TWI_FM; else DELAY_T4TWI;
    USICR = temp;                                        // Generate negative SCL edge.
  } while (!(USISR & (1 << USIOIF)));                    // Check for transfer complete.

  if (USI_TWI_MASTER_SPEED) DELAY_T2TWI_FM; else DELAY_T2TWI;
  temp  = USIDR;                                         // Read out data.
  USIDR = 0xFF;                                          // Release SDA.
  USI_DDR |= (1 << USI_DI_BIT);                          // Enable SDA as output.

  return temp; // Return the data from the USIDR
}

/*---------------------------------------------------------------
 Function for generating a TWI Stop Condition. Used to release
 the TWI bus.
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Stop(void) {
  USI_PORT &= ~(1 << USI_DI_BIT);            // Pull SDA low.
  USI_CLOCK_PORT |= (1 << USI_CLOCK_BIT);         // Release SCL.
  while (!(USI_CLOCK_PIN & (1 << USI_CLOCK_BIT)));   // Wait for SCL to go high.
  if (USI_TWI_MASTER_SPEED) DELAY_T4TWI_FM; else DELAY_T4TWI;
  USI_PORT |= (1 << USI_DI_BIT);             // Release SDA.
  if (USI_TWI_MASTER_SPEED) DELAY_T2TWI_FM; else DELAY_T2TWI;

#ifdef SIGNAL_VERIFY
  if (!(USISR & (1 << USIPF))) {
    USI_TWI_state.errorState = USI_TWI_MISSING_STOP_CON;
    return (FALSE);
  }
#endif

  return (TRUE);
}
#endif
