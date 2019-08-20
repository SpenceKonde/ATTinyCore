// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : USI_TWI_Slave.c
* Compiler          : IAR EWAAVR 4.11A
* Revision          : $Revision: 614 $
* Date              : $Date: 2006-07-07 16:27:05 +0800 (Fri, 07 Jul 2006) $
* Updated by        : $Author: jllassen $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All device with USI module can be used.
*                     The example is written for the ATmega169, ATtiny26, ATmega325 & ATtiny2313
*
* AppNote           : AVR312 - Using the USI module as a I2C slave
*
* Description       : Functions for USI_TWI_receiver and USI_TWI_transmitter.
*
*
****************************************************************************/


#ifdef __GNUC__
#include "avr/io.h"
#include "avr/interrupt.h"
#elif __ICCAVR__
#include "ioavr.h"
#include "inavr.h"
#endif
#ifdef USIDR
#include "USI_TWI_Slave.h"

//********** Buffer **********//
uint8_t TWI_Buffer[TWI_BUFFER_SIZE];

//********** Static Variables **********//

static unsigned char          TWI_slaveAddress;
static volatile unsigned char USI_TWI_Overflow_State;

/*=========================> Locals <=======================================*/
static uint8_t *        TWI_RxBuf = TWI_Buffer;
static volatile uint8_t TWI_RxHead;
static volatile uint8_t TWI_RxTail;

static uint8_t *        TWI_TxBuf = TWI_Buffer + TWI_RX_BUFFER_SIZE;
static volatile uint8_t TWI_TxHead;
static volatile uint8_t TWI_TxTail;

void Flush_TWI_Buffers(void)
{
  TWI_RxTail = 0;
  TWI_RxHead = 0;
  TWI_TxTail = 0;
  TWI_TxHead = 0;
}

//********** USI_TWI functions **********//

/*----------------------------------------------------------
  Initialise USI for TWI Slave mode.
----------------------------------------------------------*/
void USI_TWI_Slave_Initialise(unsigned char TWI_ownAddress)
{
  Flush_TWI_Buffers();

  TWI_slaveAddress = TWI_ownAddress;

  USI_TWI_On_Slave_Transmit = 0;
  USI_TWI_On_Slave_Receive = 0;

  PORT_USI_CL |= (1 << PORT_USI_SCL);     // Set SCL high
  PORT_USI |= (1 << PORT_USI_SDA);        // Set SDA high
  DDR_USI_CL |= (1 << PORT_USI_SCL);      // Set SCL as output
  DDR_USI &= ~(1 << PORT_USI_SDA);        // Set SDA as input
  USICR = (1 << USISIE) | (0 << USIOIE) | // Enable Start Condition Interrupt. Disable Overflow Interrupt.
          (1 << USIWM1) | (0 << USIWM0) | // Set USI in Two-wire mode. No USI Counter overflow prior
                                          // to first Start Condition (potential failure)
          (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | // Shift Register Clock Source = External, positive edge
          (0 << USITC);
  USISR = 0xF0; // Clear all flags and reset overflow counter
}

// Disable USI for TWI Slave mode.
void USI_TWI_Slave_Disable()
{
  DDR_USI_CL &= ~(1 << PORT_USI_SCL);  // Set SCL as input
  DDR_USI &= ~(1 << PORT_USI_SDA);  // Set SDA as input
  USICR = 0x00; // Disable USI
  USISR = 0xF0; // Clear all flags and reset overflow counter
}

// Puts data in the transmission buffer, Waits if buffer is full.
void USI_TWI_Transmit_Byte(unsigned char data)
{
  unsigned char tmphead;

  tmphead = (TWI_TxHead + 1) & TWI_TX_BUFFER_MASK; // Calculate buffer index.
  while (tmphead == TWI_TxTail)
    ;                         // Wait for free space in buffer.
  TWI_TxBuf[tmphead] = data;    // Store data in buffer.
  TWI_TxHead         = tmphead; // Store new index.
}

// Returns a byte from the receive buffer. Waits if buffer is empty.
unsigned char USI_TWI_Receive_Byte(void)
{
  unsigned char tmptail;
  unsigned char tmpRxTail; // Temporary variable to store volatile
  tmpRxTail = TWI_RxTail;  // Not necessary, but prevents warnings
  while (TWI_RxHead == tmpRxTail)
    ;
  tmptail    = (TWI_RxTail + 1) & TWI_RX_BUFFER_MASK; // Calculate buffer index
  TWI_RxTail = tmptail;                               // Store new index
  return TWI_RxBuf[tmptail];                          // Return data from the buffer.
}

// Returns a byte from the receive buffer without incrementing TWI_RxTail. Waits if buffer is empty.
unsigned char USI_TWI_Peek_Receive_Byte(void)
{
  unsigned char tmptail;
  unsigned char tmpRxTail; // Temporary variable to store volatile
  tmpRxTail = TWI_RxTail;  // Not necessary, but prevents warnings
  while (TWI_RxHead == tmpRxTail)
    ;
  tmptail    = (TWI_RxTail + 1) & TWI_RX_BUFFER_MASK; // Calculate buffer index
  return TWI_RxBuf[tmptail];                          // Return data from the buffer.
}

// Check if there is data in the receive buffer.
unsigned char USI_TWI_Data_In_Receive_Buffer(void)
{
  unsigned char tmpRxTail;          // Temporary variable to store volatile
  tmpRxTail = TWI_RxTail;           // Not necessary, but prevents warnings
  return (TWI_RxHead - tmpRxTail) & TWI_RX_BUFFER_MASK; // Return 0 (FALSE) if the receive buffer is empty.
}

// Check if there is space in the transmission buffer.
unsigned char USI_TWI_Space_In_Transmission_Buffer(void)
{
  unsigned char tmpTxHead;
  tmpTxHead = (TWI_TxHead + 1) & TWI_TX_BUFFER_MASK; // Calculate next buffer index.
  return (TWI_TxTail - tmpTxHead) & TWI_TX_BUFFER_MASK; // Return 0 (FALSE) if the transmission buffer is full.
}

// Check if there is an active data session.
unsigned char USI_TWI_Slave_Is_Active()
{
  // Active if Overflow Interrupt is enabled and no Stop Condition occurred
  return (USICR & (1 << USIOIE)) && ((USISR & (1 << USIPF)) == 0);
}

/*----------------------------------------------------------
 Detects the USI_TWI Start Condition and initializes the USI
 for reception of the "TWI Address" packet.
----------------------------------------------------------*/
#ifdef __GNUC__
ISR(USI_START_VECTOR)
#elif __ICCAVR__
#pragma vector = USI_START_VECTOR
__interrupt void USI_Start_Condition_ISR(void)
#endif
{
  unsigned char tmpPin; // Temporary variable for pin state
  unsigned char tmpRxHead; // Temporary variable to store volatile
  // call slave receive callback on repeated start
  if (USI_TWI_On_Slave_Receive) {
    tmpRxHead = TWI_RxHead;
    if (TWI_RxTail != tmpRxHead) { // data in receive buffer
      USI_TWI_On_Slave_Receive(USI_TWI_Data_In_Receive_Buffer());
      // reset rx buffer
      TWI_RxTail = tmpRxHead;
    }
  }

  USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
  DDR_USI &= ~(1 << PORT_USI_SDA); // Set SDA as input
  while ((tmpPin = (PIN_USI_CL & (1 << PORT_USI_SCL))) && ((PIN_USI & (1 << PIN_USI_SDA)) == 0))
    ; // Wait for SCL to go low to ensure the "Start Condition" has completed.
        // If a Stop condition arises then leave the interrupt to prevent waiting forever.
  if (tmpPin) {
    // Stop Condition (waiting for next Start Condition)
    USICR = (1 << USISIE) | (0 << USIOIE) | // Enable Start Condition Interrupt. Disable Overflow Interrupt.
            (1 << USIWM1) | (0 << USIWM0) | // Set USI in Two-wire mode. No USI Counter overflow prior
                                            // to first Start Condition (potential failure)
            (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | // Shift Register Clock Source = External, positive edge
            (0 << USITC);
  } else {
    // really Start Condition (Enable Overflow Interrupt)
    USICR = (1 << USISIE) | (1 << USIOIE)
            | // Enable Overflow and Start Condition Interrupt. (Keep StartCondInt to detect RESTART)
            (1 << USIWM1) | (1 << USIWM0) |                 // Set USI in Two-wire mode.
            (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | // Shift Register Clock Source = External, positive edge
            (0 << USITC);
  }
  USISR = (1 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) | // Clear flags
          (0x0 << USICNT0); // Set USI to sample 8 bits i.e. count 16 external pin toggles.
}

/*----------------------------------------------------------
 Handles all the communication. Is disabled only when waiting
 for new Start Condition.
----------------------------------------------------------*/
#ifdef __GNUC__
ISR(USI_OVERFLOW_VECTOR)
#elif __ICCAVR__
#pragma vector = USI_OVERFLOW_VECTOR
__interrupt void USI_Counter_Overflow_ISR(void)
#endif
{
  unsigned char tmpRxHead;
  unsigned char tmpTxTail; // Temporary variables to store volatiles
  unsigned char tmpUSIDR;

  switch (USI_TWI_Overflow_State) {
  // ---------- Address mode ----------
  // Check address and send ACK (and next USI_SLAVE_SEND_DATA) if OK, else reset USI.
  case USI_SLAVE_CHECK_ADDRESS:
    if ((USIDR == 0) || ((USIDR >> 1) == TWI_slaveAddress)) {
      if (USIDR & 0x01) {
        if (USI_TWI_On_Slave_Transmit) {
          // reset tx buffer and call callback
          tmpTxTail = TWI_TxHead;
          TWI_TxTail = tmpTxTail;
          USI_TWI_On_Slave_Transmit();
        }
        USI_TWI_Overflow_State = USI_SLAVE_SEND_DATA;
      } else {
        USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
      }
      SET_USI_TO_SEND_ACK();
    } else {
      SET_USI_TO_TWI_START_CONDITION_MODE();
    }
    break;

  // ----- Master write data mode ------
  // Check reply and goto USI_SLAVE_SEND_DATA if OK, else reset USI.
  case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
    if (USIDR) // If NACK, the master does not want more data.
    {
      SET_USI_TO_TWI_START_CONDITION_MODE();
      return;
    }
  // From here we just drop straight into USI_SLAVE_SEND_DATA if the master sent an ACK

  // Copy data from buffer to USIDR and set USI to shift byte. Next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
  case USI_SLAVE_SEND_DATA:

    // Get data from Buffer
    tmpTxTail = TWI_TxTail; // Not necessary, but prevents warnings
    if (TWI_TxHead != tmpTxTail) {
      TWI_TxTail = (TWI_TxTail + 1) & TWI_TX_BUFFER_MASK;
      USIDR      = TWI_TxBuf[TWI_TxTail];
    } else // If the buffer is empty then:
    {
      SET_USI_TO_TWI_START_CONDITION_MODE();
      return;
    }
    USI_TWI_Overflow_State = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
    SET_USI_TO_SEND_DATA();
    break;

  // Set USI to sample reply from master. Next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
  case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
    USI_TWI_Overflow_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
    SET_USI_TO_READ_ACK();
    break;

  // ----- Master read data mode ------
  // Set USI to sample data from master. Next USI_SLAVE_GET_DATA_AND_SEND_ACK.
  case USI_SLAVE_REQUEST_DATA:
    USI_TWI_Overflow_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;
    SET_USI_TO_READ_DATA();
    // call slave receive callback on stop condition
    if (USI_TWI_On_Slave_Receive) {
      tmpRxHead = TWI_RxHead;
      if (TWI_RxTail != tmpRxHead) { // data in receive buffer
        // check for stop Condition
        while ((USISR & ((1 << USI_START_COND_INT) | (1 << USIPF) | (0xE << USICNT0))) == 0)
          ;// wait for either Start or Stop Condition
          // cancel after one SCL cycle
        if (USISR & (1 << USIPF)) { // Stop Condition
          USI_TWI_On_Slave_Receive(USI_TWI_Data_In_Receive_Buffer());
          // reset rx buffer
          TWI_RxTail = tmpRxHead;
        }
      }
    }
    break;

  // Copy data from USIDR and send ACK. Next USI_SLAVE_REQUEST_DATA
  case USI_SLAVE_GET_DATA_AND_SEND_ACK:
    // Put data into Buffer
    USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
    tmpUSIDR              = USIDR; // Not necessary, but prevents warnings
    tmpRxHead = (TWI_RxHead + 1) & TWI_RX_BUFFER_MASK;
    if (TWI_RxTail != tmpRxHead) {
      TWI_RxHead            = tmpRxHead;
      TWI_RxBuf[TWI_RxHead] = tmpUSIDR;
      SET_USI_TO_SEND_ACK();
    } else // If the buffer is full then:
    {
      SET_USI_TO_SEND_NACK();
    }
    break;
  }
}
#endif
