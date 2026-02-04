/*****************************************************************************
*
*
* File              USI_TWI_Master.c compiled with gcc
* Date              Friday, 10/31/08		Boo!
* Updated by        jkl
*

* AppNote           : AVR310 - Using the USI module as a TWI Master
*
*		Extensively modified to provide complete I2C driver.
*	
*Notes: 
*		- T4_TWI and T2_TWI delays are modified to work with 1MHz default clock
*			and now use hard code values. They would need to change
*			for other clock rates. Refer to the Apps Note.
*
*	12/17/08	Added USI_TWI_Start_Memory_Read Routine		-jkl
*		Note msg buffer will have slave adrs ( with write bit set) and memory adrs;
*			length should be these two bytes plus the number of bytes to read.
****************************************************************************/
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include "USI_TWI_Master.h"

unsigned char USI_TWI_Start_Transceiver_With_Data( unsigned char * , unsigned char );
unsigned char USI_TWI_Master_Transfer( unsigned char );
unsigned char USI_TWI_Master_Start( void );

union  USI_TWI_state
{
  unsigned char errorState;         // Can reuse the TWI_state for error states since it will not be needed if there is an error.
  struct
  {
    unsigned char addressMode         : 1;
    unsigned char masterWriteDataMode : 1;
	unsigned char memReadMode		  : 1;
    unsigned char unused              : 5;
  }; 
}   USI_TWI_state;

/*---------------------------------------------------------------
 USI TWI single master initialization function
---------------------------------------------------------------*/
void USI_TWI_Master_Initialise( void )
{
  PORT_USI |= (1<<PIN_USI_SDA);           // Enable pullup on SDA, to set high as released state.
  PORT_USI |= (1<<PIN_USI_SCL);           // Enable pullup on SCL, to set high as released state.
  
  DDR_USI  |= (1<<PIN_USI_SCL);           // Enable SCL as output.
  DDR_USI  |= (1<<PIN_USI_SDA);           // Enable SDA as output.
  
  USIDR    =  0xFF;                       // Preload dataregister with "released level" data.
  USICR    =  (0<<USISIE)|(0<<USIOIE)|                            // Disable Interrupts.
              (1<<USIWM1)|(0<<USIWM0)|                            // Set USI in Two-wire mode.
              (1<<USICS1)|(0<<USICS0)|(1<<USICLK)|                // Software stobe as counter clock source
              (0<<USITC);
  USISR   =   (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      // Clear flags,
              (0x0<<USICNT0);                                     // and reset counter.
}

/*---------------------------------------------------------------
Use this function to get hold of the error message from the last transmission
---------------------------------------------------------------*/
unsigned char USI_TWI_Get_State_Info( void )
{
  return ( USI_TWI_state.errorState );                            // Return error state.
}
/*---------------------------------------------------------------
 USI Random (memory) Read function. This function sets up for call
 to USI_TWI_Start_Transceiver_With_Data which does the work.
 Doesn't matter if read/write bit is set or cleared, it'll be set
 correctly in this function.
 
 The msgSize is passed to USI_TWI_Start_Transceiver_With_Data.
 
 Success or error code is returned. Error codes are defined in 
 USI_TWI_Master.h
---------------------------------------------------------------*/
unsigned char USI_TWI_Start_Random_Read( unsigned char *msg, unsigned char msgSize)
{
  *(msg) &= ~(TRUE<<TWI_READ_BIT);		// clear the read bit if it's set
  USI_TWI_state.errorState = 0;
  USI_TWI_state.memReadMode = TRUE;
  
  return (USI_TWI_Start_Transceiver_With_Data( msg, msgSize));
}
/*---------------------------------------------------------------
 USI Normal Read / Write Function
 Transmit and receive function. LSB of first byte in buffer 
 indicates if a read or write cycles is performed. If set a read
 operation is performed.

 Function generates (Repeated) Start Condition, sends address and
 R/W, Reads/Writes Data, and verifies/sends ACK.
 
 Success or error code is returned. Error codes are defined in 
 USI_TWI_Master.h
---------------------------------------------------------------*/
unsigned char USI_TWI_Start_Read_Write( unsigned char *msg, unsigned char msgSize)
{
    
	USI_TWI_state.errorState = 0;				// Clears all mode bits also
  
	return (USI_TWI_Start_Transceiver_With_Data( msg, msgSize));
	
}
/*---------------------------------------------------------------
 USI Transmit and receive function. LSB of first byte in buffer 
 indicates if a read or write cycles is performed. If set a read
 operation is performed.

 Function generates (Repeated) Start Condition, sends address and
 R/W, Reads/Writes Data, and verifies/sends ACK.
 
 This function also handles Random Read function if the memReadMode
 bit is set. In that case, the function will:
 The address in memory will be the second
 byte and is written *without* sending a STOP. 
 Then the Read bit is set (lsb of first byte), the byte count is 
 adjusted (if needed), and the function function starts over by sending
 the slave address again and reading the data.
 
 Success or error code is returned. Error codes are defined in 
 USI_TWI_Master.h
---------------------------------------------------------------*/
unsigned char USI_TWI_Start_Transceiver_With_Data( unsigned char *msg, unsigned char msgSize)
{
  unsigned char const tempUSISR_8bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      // Prepare register value to: Clear flags, and
                                 (0x0<<USICNT0);                                     // set USI to shift 8 bits i.e. count 16 clock edges.
  unsigned char const tempUSISR_1bit = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|      // Prepare register value to: Clear flags, and
                                 (0xE<<USICNT0); 									// set USI to shift 1 bit i.e. count 2 clock edges.
	unsigned char *savedMsg;
	unsigned char savedMsgSize; 

//This clear must be done before calling this function so that memReadMode can be specified.
//  USI_TWI_state.errorState = 0;				// Clears all mode bits also

  USI_TWI_state.addressMode = TRUE;			// Always true for first byte

#ifdef PARAM_VERIFICATION
  if(msg > (unsigned char*)RAMEND)                 // Test if address is outside SRAM space
  {
    USI_TWI_state.errorState = USI_TWI_DATA_OUT_OF_BOUND;
    return (FALSE);
  }
  if(msgSize <= 1)                                 // Test if the transmission buffer is empty
  {
    USI_TWI_state.errorState = USI_TWI_NO_DATA;
    return (FALSE);
  }
#endif

#ifdef NOISE_TESTING                                // Test if any unexpected conditions have arrived prior to this execution.
  if( USISR & (1<<USISIF) )
  {
    USI_TWI_state.errorState = USI_TWI_UE_START_CON;
    return (FALSE);
  }
  if( USISR & (1<<USIPF) )
  {
    USI_TWI_state.errorState = USI_TWI_UE_STOP_CON;
    return (FALSE);
  }
  if( USISR & (1<<USIDC) )
  {
    USI_TWI_state.errorState = USI_TWI_UE_DATA_COL;
    return (FALSE);
  }
#endif

  if ( !(*msg & (1<<TWI_READ_BIT)) )                // The LSB in the address byte determines if is a masterRead or masterWrite operation.
  {
    USI_TWI_state.masterWriteDataMode = TRUE;
  }

//	if (USI_TWI_state.memReadMode)
//	{
		savedMsg = msg;
		savedMsgSize = msgSize;
//	}

	if ( !USI_TWI_Master_Start( ))
  {
	return (FALSE);                           // Send a START condition on the TWI bus.
  }

/*Write address and Read/Write data */
  do
  {
    /* If masterWrite cycle (or inital address tranmission)*/
    if (USI_TWI_state.addressMode || USI_TWI_state.masterWriteDataMode)
    {
      /* Write a byte */
      PORT_USI &= ~(1<<PIN_USI_SCL);                // Pull SCL LOW.
      USIDR     = *(msg++);                        // Setup data.
      USI_TWI_Master_Transfer( tempUSISR_8bit );    // Send 8 bits on bus.
      
      /* Clock and verify (N)ACK from slave */
      DDR_USI  &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
      if( USI_TWI_Master_Transfer( tempUSISR_1bit ) & (1<<TWI_NACK_BIT) ) 
      {
        if ( USI_TWI_state.addressMode )
          USI_TWI_state.errorState = USI_TWI_NO_ACK_ON_ADDRESS;
        else
          USI_TWI_state.errorState = USI_TWI_NO_ACK_ON_DATA;
        return (FALSE);
      }
	  
	  if ((!USI_TWI_state.addressMode) && USI_TWI_state.memReadMode)// means memory start address has been written
	  {
		msg = savedMsg;					// start at slave address again
		*(msg) |= (TRUE<<TWI_READ_BIT);  // set the Read Bit on Slave address
		USI_TWI_state.errorState = 0;
		USI_TWI_state.addressMode = TRUE;	// Now set up for the Read cycle
		msgSize = savedMsgSize;				// Set byte count correctly
		// NOte that the length should be Slave adrs byte + # bytes to read + 1 (gets decremented below)
		if ( !USI_TWI_Master_Start( ))
		{
			USI_TWI_state.errorState = USI_TWI_BAD_MEM_READ;
			return (FALSE);                           // Send a START condition on the TWI bus.
		}
	  }
	  else
	  {
		USI_TWI_state.addressMode = FALSE;            // Only perform address transmission once.
	  }
    }
    /* Else masterRead cycle*/
    else
    {
      /* Read a data byte */
      DDR_USI   &= ~(1<<PIN_USI_SDA);               // Enable SDA as input.
      *(msg++)  = USI_TWI_Master_Transfer( tempUSISR_8bit );

      /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
      if( msgSize == 1)                            // If transmission of last byte was performed.
      {
        USIDR = 0xFF;                              // Load NACK to confirm End Of Transmission.
      }
      else
      {
        USIDR = 0x00;                              // Load ACK. Set data register bit 7 (output for SDA) low.
      }
      USI_TWI_Master_Transfer( tempUSISR_1bit );   // Generate ACK/NACK.
    }
  }while( --msgSize) ;                             // Until all data sent/received.

  // usually a stop condition is sent here, but TinyWireM needs to choose whether or not to send it

/* Transmission successfully completed*/
  return (TRUE);
}

/*---------------------------------------------------------------
 Core function for shifting data in and out from the USI.
 Data to be sent has to be placed into the USIDR prior to calling
 this function. Data read, will be return'ed from the function.
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Transfer( unsigned char temp )
{
  USISR = temp;                                     // Set USISR according to temp.
                                                    // Prepare clocking.
  temp  =  (0<<USISIE)|(0<<USIOIE)|                 // Interrupts disabled
           (1<<USIWM1)|(0<<USIWM0)|                 // Set USI in Two-wire mode.
           (1<<USICS1)|(0<<USICS0)|(1<<USICLK)|     // Software clock strobe as source.
           (1<<USITC);                              // Toggle Clock Port.
  do
  { 
	_delay_us(T2_TWI);
    USICR = temp;                          // Generate positve SCL edge.
    while( !(PIN_USI & (1<<PIN_USI_SCL)) );// Wait for SCL to go high.
	_delay_us(T4_TWI);
    USICR = temp;                          // Generate negative SCL edge.
  }while( !(USISR & (1<<USIOIF)) );        // Check for transfer complete.
  
	_delay_us(T2_TWI);
  temp  = USIDR;                           // Read out data.
  USIDR = 0xFF;                            // Release SDA.
  DDR_USI |= (1<<PIN_USI_SDA);             // Enable SDA as output.

  return temp;                             // Return the data from the USIDR
}
/*---------------------------------------------------------------
 Function for generating a TWI Start Condition. 
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Start( void )
{
/* Release SCL to ensure that (repeated) Start can be performed */
  PORT_USI |= (1<<PIN_USI_SCL);                     // Release SCL.
  while( !(PORT_USI & (1<<PIN_USI_SCL)) );          // Verify that SCL becomes high.
  _delay_us(T2_TWI);

/* Generate Start Condition */
  PORT_USI &= ~(1<<PIN_USI_SDA);                    // Force SDA LOW.
	_delay_us(T4_TWI);                         
  PORT_USI &= ~(1<<PIN_USI_SCL);                    // Pull SCL LOW.
  PORT_USI |= (1<<PIN_USI_SDA);                     // Release SDA.

#ifdef SIGNAL_VERIFY
  if( !(USISR & (1<<USISIF)) )
  {
    USI_TWI_state.errorState = USI_TWI_MISSING_START_CON;  
    return (FALSE);
  }
#endif
  return (TRUE);
}
/*---------------------------------------------------------------
 Function for generating a TWI Stop Condition. Used to release 
 the TWI bus.
---------------------------------------------------------------*/
unsigned char USI_TWI_Master_Stop( void )
{
  PORT_USI &= ~(1<<PIN_USI_SDA);           // Pull SDA low.
  PORT_USI |= (1<<PIN_USI_SCL);            // Release SCL.
  while( !(PIN_USI & (1<<PIN_USI_SCL)) );  // Wait for SCL to go high.  
	_delay_us(T4_TWI);
  PORT_USI |= (1<<PIN_USI_SDA);            // Release SDA.
	_delay_us(T2_TWI);
  
#ifdef SIGNAL_VERIFY
  if( !(USISR & (1<<USIPF)) )
  {
    USI_TWI_state.errorState = USI_TWI_MISSING_STOP_CON;    
    return (FALSE);
  }
#endif

  return (TRUE);
}
