// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * File              : USI_TWI_Slave.h
 * Compiler          : IAR EWAAVR 4.11A
 * Revision          : $Revision: 6351 $
 * Date              : $Date: 2010-01-29 20:15:43 +0800 (Fri, 29 Jan 2010) $
 * Updated by        : $Author: hskinnemoen $
 *
 * Support mail      : avr@atmel.com
 *
 * Supported devices : All device with USI module can be used.
 *                     The example is written for the ATmega169, ATtiny26 & ATtiny2313
 *
 * AppNote           : AVR312 - Using the USI module as a TWI slave
 *
 * Description       : Header file for USI_TWI driver
 *
 *
 *
 ****************************************************************************/
//********** Prototypes **********//
#include <avr/io.h>
#ifndef TWDR
void          USI_TWI_Slave_Initialise(unsigned char);
void          USI_TWI_Slave_Disable();
void          USI_TWI_Transmit_Byte(unsigned char);
unsigned char USI_TWI_Receive_Byte(void);
unsigned char USI_TWI_Peek_Receive_Byte(void);
unsigned char USI_TWI_Data_In_Receive_Buffer(void);
unsigned char USI_TWI_Space_In_Transmission_Buffer(void);
unsigned char USI_TWI_Slave_Is_Active();

void (*USI_TWI_On_Slave_Transmit)(void);
void (*USI_TWI_On_Slave_Receive)(int);

void Timer_Init(void);

#define TRUE 1
#define FALSE 0

typedef unsigned char uint8_t;

//////////////////////////////////////////////////////////////////
///////////////// Driver Buffer Definitions //////////////////////
//////////////////////////////////////////////////////////////////
// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes
extern uint8_t TWI_Buffer[];

#define TWI_RX_BUFFER_SIZE (16)
#define TWI_RX_BUFFER_MASK (TWI_RX_BUFFER_SIZE - 1)

#if (TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK)
#error TWI RX buffer size is not a power of 2
#endif

// 1,2,4,8,16,32,64,128 or 256 bytes are allowed buffer sizes

#define TWI_TX_BUFFER_SIZE (16)
#define TWI_TX_BUFFER_MASK (TWI_TX_BUFFER_SIZE - 1)

#if (TWI_TX_BUFFER_SIZE & TWI_TX_BUFFER_MASK)
#error TWI TX buffer size is not a power of 2
#endif

#define TWI_BUFFER_SIZE (TWI_RX_BUFFER_SIZE + TWI_TX_BUFFER_SIZE)
//////////////////////////////////////////////////////////////////

#define USI_SLAVE_CHECK_ADDRESS (0x00)
#define USI_SLAVE_SEND_DATA (0x01)
#define USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA (0x02)
#define USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA (0x03)
#define USI_SLAVE_REQUEST_DATA (0x04)
#define USI_SLAVE_GET_DATA_AND_SEND_ACK (0x05)

// Device dependent defines
#include "pins_arduino.h"

#define SET_USI_TO_SEND_ACK()                                                                                          \
  {                                                                                                                  \
    USIDR = 0;                      /* Prepare ACK                         */                                      \
    DDR_USI |= (1 << PORT_USI_SDA); /* Set SDA as output                   */                                      \
    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)                                \
            |                  /* Clear all flags, except Start Cond  */                                           \
            (0x0E << USICNT0); /* set USI counter to shift 1 bit. */                                               \
  }

#define SET_USI_TO_SEND_NACK()                                                                                          \
  {                                                                                                                  \
    DDR_USI &= ~(1 << PORT_USI_SDA); /* Set SDA as intput, NACK is SDA high */                                     \
    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)                                \
            |                  /* Clear all flags, except Start Cond  */                                           \
            (0x0E << USICNT0); /* set USI counter to shift 1 bit. */                                               \
  }

#define SET_USI_TO_READ_ACK()                                                                                          \
  {                                                                                                                  \
    DDR_USI &= ~(1 << PORT_USI_SDA); /* Set SDA as intput */                                                       \
    USIDR = 0;                       /* Prepare ACK        */                                                      \
    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)                                \
            |                  /* Clear all flags, except Start Cond  */                                           \
            (0x0E << USICNT0); /* set USI counter to shift 1 bit. */                                               \
  }

#define SET_USI_TO_TWI_START_CONDITION_MODE()                                                                          \
  {                                                                                                                  \
    DDR_USI &= ~(1 << PORT_USI_SDA); /* Set SDA as intput */                                                       \
    USICR = (1 << USISIE) | (0 << USIOIE) | /* Enable Start Condition Interrupt. Disable Overflow Interrupt.*/     \
            (1 << USIWM1) | (0 << USIWM0) | /* Set USI in Two-wire mode. No USI Counter overflow hold.      */     \
            (1 << USICS1) | (0 << USICS0) | (0 << USICLK)                                                          \
            | /* Shift Register Clock Source = External, positive edge        */                                   \
            (0 << USITC);                                                                                          \
    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)                                \
            | /* Clear all flags, except Start Cond                            */                                  \
            (0x0 << USICNT0);                                                                                      \
  }

#define SET_USI_TO_SEND_DATA()                                                                                         \
  {                                                                                                                  \
    DDR_USI |= (1 << PORT_USI_SDA); /* Set SDA as output                  */                                       \
    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)                                \
            |                 /* Clear all flags, except Start Cond */                                             \
            (0x0 << USICNT0); /* set USI to shift out 8 bits        */                                             \
  }

#define SET_USI_TO_READ_DATA()                                                                                         \
  {                                                                                                                  \
    DDR_USI &= ~(1 << PORT_USI_SDA); /* Set SDA as input                   */                                      \
    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC)                                \
            |                 /* Clear all flags, except Start Cond */                                             \
            (0x0 << USICNT0); /* set USI to shift out 8 bits        */                                             \
  }
#endif
