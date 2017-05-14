/*****************************************************************************
*
* Atmel Corporation
*
* File              : main.c
* Date              : $Date: 2016-7-15 $
* Updated by        : $Author: Atmel $
*
* Support mail      : avr@atmel.com
*
* Supported devices : All device with USI module can be used.
*                     The example is written for the ATmega169, ATtiny26 & ATtiny2313
*
* AppNote           : AVR310 - Using the USI module as a TWI Master
*
* Description       : Example of how to use the driver for TWI master
*                     communication with the USI module.
*                     This code reads PORTD and sends the status to the TWI slave.
*                     Then it reads data from the slave and puts the data on PORTB.
*                     To run the example use STK500 and connect PORTB to the LEDS,
*                     and PORTD to the switches.
*
****************************************************************************/
#include "atmel_start.h"
#include "atmel_start_pins.h"
#if __GNUC__
#include <avr/io.h>
#include <avr/interrupt.h>
#else
#include <inavr.h>
#include <ioavr.h>
#endif
#include "USI_TWI_Master.h"

#define SLAVE_ADDR 0x10

#define MESSAGEBUF_SIZE 4

#define TWI_GEN_CALL 0x00 // The General Call address is 0

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ 0x20

// Sample TWI transmission states, used in the main application.
#define SEND_DATA 0x01
#define REQUEST_DATA 0x02
#define READ_DATA_FROM_BUFFER 0x03

unsigned char TWI_Act_On_Failure_In_Last_Transmission(unsigned char TWIerrorMsg)
{
	// A failure has occurred, use TWIerrorMsg to determine the nature of the failure
	// and take appropriate actions.
	// Se header file for a list of possible failures messages.
	asm("nop");
	return TWIerrorMsg;
}

int main(void)
{
	unsigned char messageBuf[MESSAGEBUF_SIZE];
	unsigned char TWI_targetSlaveAddress, temp, pressedButton, myCounter = 0;

	system_init();
	// LED feedback port - connect port B to the STK500 LEDS
	DDRB  = 0xFF;
	PORTB = myCounter;

	// Switch port - connect portD to the STK500 switches
	DDRD = 0x00;

	USI_TWI_Master_Initialise();

	asm("sei");

	TWI_targetSlaveAddress = 0x10;

	// This example is made to work together with the AVR311 TWI Slave application note and stk500.
	// In adition to connecting the TWI pins, also connect PORTB to the LEDS and PORTD to the switches.
	// The code reads the pins to trigger the action you request. There is an example sending a general call,
	// address call with Master Read and Master Write. The first byte in the transmission is used to send
	// commands to the TWI slave.

	// This is a stk500 demo example. The buttons on PORTD are used to control different TWI operations.
	for (;;) {
		pressedButton = ~PIND;
		if (pressedButton) // Check if any button is pressed
		{
			do {
				temp = ~PIND;
			} // Wait until key released
			while (temp);

			temp = TRUE; // I.e. Success. Used to trap failed operations.

			switch (pressedButton) {
			// Send a Generall Call
			case (1 << PD0):
				messageBuf[0]
				    = TWI_GEN_CALL; // The first byte must always consit of General Call code or the TWI slave address.
				messageBuf[1] = 0xAA; // The command or data to be included in the general call.
				temp          = USI_TWI_Start_Transceiver_With_Data(messageBuf, 2);
				break;

			// Send a Address Call, sending a command and data to the Slave
			case (1 << PD1):
				messageBuf[0] = (TWI_targetSlaveAddress << TWI_ADR_BITS)
				                | (FALSE << TWI_READ_BIT); // The first byte must always consit of General Call code or
				                                           // the TWI slave address.
				messageBuf[1] = TWI_CMD_MASTER_WRITE;      // The first byte is used for commands.
				messageBuf[2] = myCounter;                 // The second byte is used for the data.
				temp          = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);
				break;

			// Send a Address Call, sending a request, followed by a resceive
			case (1 << PD2):
				// Send the request-for-data command to the Slave
				messageBuf[0] = (TWI_targetSlaveAddress << TWI_ADR_BITS)
				                | (FALSE << TWI_READ_BIT); // The first byte must always consit of General Call code or
				                                           // the TWI slave address.
				messageBuf[1] = TWI_CMD_MASTER_READ;       // The first byte is used for commands.
				temp          = USI_TWI_Start_Transceiver_With_Data(messageBuf, 2);

				// One might need to delay before proceding to enable the slave to interpret the last message,
				// and prepare the next request. Or as in this simple sample rerun the request until he responds.

				// Transmit request and get the received data from the transceiver buffer
				messageBuf[0] = (TWI_targetSlaveAddress << TWI_ADR_BITS)
				                | (TRUE << TWI_READ_BIT); // The first byte must always consit of General Call code or
				                                          // the TWI slave address.
				do
					temp = USI_TWI_Start_Transceiver_With_Data(messageBuf, 2);
				while (USI_TWI_Get_State_Info() == USI_TWI_NO_ACK_ON_ADDRESS);

				PORTB = messageBuf[1]; // Store data on PORTB.
				break;

			case (1 << PD3):
				// Send the request-for-data command to the Slave
				if (TWI_targetSlaveAddress == 0x10)
					TWI_targetSlaveAddress = 0x15;
				else
					TWI_targetSlaveAddress = 0x10;
				break;

			// Get status from Transceiver and put it on PORTB
			case (1 << PD5):
				PORTB = USI_TWI_Get_State_Info();
				break;

			// Increment myCounter and put it on PORTB
			case (1 << PD6):
				PORTB = ++myCounter;
				break;

			// Reset myCounter and put it on PORTB
			case (1 << PD7):
				PORTB = myCounter = 0;
				break;
			}

			if (!temp) // One of the operations failed.
			{          // Use TWI status information to detemine cause of failure and take appropriate actions.
				TWI_Act_On_Failure_In_Last_Transmission(USI_TWI_Get_State_Info());
				asm("nop"); // Put own code here.
			}
		}
	}
}
