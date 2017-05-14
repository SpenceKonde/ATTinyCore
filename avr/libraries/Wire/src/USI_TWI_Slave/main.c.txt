/**
 * \file
 *
 * \brief Using USI module as I2C slave example
 *
 * Copyright (C) 2016 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifdef __GNUC__
#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/cpufunc.h"
#elif __ICCAVR__
#include "ioavr.h"
#include "inavr.h"
#endif

#include "atmel_start.h"
#include "USI_TWI_Slave.h"

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ 0x20
#define TWI_CMD_MASTER_WRITE_ALOT 0x30

int main(void)
{
	unsigned char TWI_slaveAddress, temp;

	atmel_start_init();
	// LED feedback port - connect port B to the STK500 LEDS
	// Below equals to PORTB = 0x55, low level is valid
	PORTB_set_port_level(0xff, true);
	PORTB_set_port_level((0xff - 0x55), false);

	PORTA_set_port_dir(0xFF, PORT_DIR_OUT);
	// Own TWI slave address
	TWI_slaveAddress = 0x10;
	USI_TWI_Slave_Initialise(TWI_slaveAddress);

#ifdef __GNUC__
	sei();
#elif __ICCAVR__
	__enable_interrupt();
#endif
	// This example is made to work together with the AVR310 USI TWI Master application note. In adition to connecting
	// the TWI
	// pins, also connect PORTB to the LEDS. The code reads a message as a TWI slave and acts according to if it is a
	// general call, or an address call. If it is an address call, then the first byte is considered a command byte and
	// it then responds differently according to the commands.

	// This loop runs forever. If the TWI Transceiver is busy the execution will just continue doing other operations.
	for (;;) {

		if (USI_TWI_Data_In_Receive_Buffer()) {
			temp = USI_TWI_Receive_Byte();
			// Below equals to PORTB = temp, low level is valid
			PORTB_set_port_level(0xff, true);
			PORTB_set_port_level((0xff - temp), false);

			USI_TWI_Transmit_Byte(temp);
		}

// Do something else while waiting for the TWI transceiver to complete.
#ifdef __GNUC__
		_NOP();
#elif __ICCAVR__
		__no_operation(); // Put own code here.
#endif
	}
	return 0;
}
