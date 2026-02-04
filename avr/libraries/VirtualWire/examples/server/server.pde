// server.pde
//
// Simple example of how to use VirtualWire to send and receive messages
// with a DR3100 module.
// Wait for a message from another arduino running the 'client' example,
// and send a reply.
// You can use this as the basis of a remote control/remote sensing system
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: server.pde,v 1.1 2008/04/20 09:24:17 mikem Exp $

#include <VirtualWire.h>

void setup()
{
    Serial.begin(9600);	// Debugging only
    Serial.println("setup");

    // Initialise the IO and ISR
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec
    vw_rx_start();       // Start the receiver PLL running
}

void loop()
{
    const char *msg = "hello";
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    // Wait for a message
    vw_wait_rx();
    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
	int i;
	const char *msg = "goodbye";

        digitalWrite(13, true); // Flash a light to show received good message
	// Message with a good checksum received, dump it.
	Serial.print("Got: ");
	
	for (i = 0; i < buflen; i++)
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(" ");
	}
	Serial.println("");

	// Send a reply
	vw_send((uint8_t *)msg, strlen(msg));
        digitalWrite(13, false);
    }
}
