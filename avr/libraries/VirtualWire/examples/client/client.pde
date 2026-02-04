// client.pde
//
// Simple example of how to use VirtualWire to send and receive messages
// with a DR3100 module.
// Send a message to another arduino running the 'server' example, which
// should send a reply, which we will check
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: client.pde,v 1.1 2008/04/20 09:24:17 mikem Exp $

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

    digitalWrite(13, true); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    Serial.println("Sent");
    digitalWrite(13, false);

    // Wait at most 200ms for a reply
    if (vw_wait_rx_max(200))
    {
	if (vw_get_message(buf, &buflen)) // Non-blocking
	{
	    int i;
	    
	    // Message with a good checksum received, dump it.
	    Serial.print("Got: ");
	    
	    for (i = 0; i < buflen; i++)
	    {
		Serial.print(buf[i], HEX);
		Serial.print(" ");
	    }
	    Serial.println("");
	}
    }
    else
	Serial.println("Timout");
	
}
