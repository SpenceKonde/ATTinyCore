/****************************************************************************
 * 	Copyright (C) 2011 Jeffrey Marten Gillmor
 * 
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 * 
 * 	This library is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * 	Lesser General Public License for more details.
 * 
 * 	You should have received a copy of the GNU Lesser General Public
 * 	License along with this library; if not, write to the Free Software
 * 	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *****************************************************************************
 * 
 * 	IMPORTANT:
 * 
 * 	Some of the message formats used in this code are Copyright Digitrax, Inc.
 * 	and are used with permission as part of the EmbeddedLocoNet project. That
 * 	permission does not extend to uses in other software products. If you wish
 * 	to use this code, algorithm or these message formats outside of
 * 	EmbeddedLocoNet, please contact Digitrax Inc, for specific permission.
 * 
 * 	Note: The sale any LocoNet device hardware (including bare PCB's) that
 * 	uses this or any other LocoNet software, requires testing and certification
 * 	by Digitrax Inc. and will be subject to a licensing agreement.
 * 
 * 	Please contact Digitrax Inc. for details.
 * /
/*****************************************************************************
* 
* Title :   LocoNetOverTCP Ethernet Buffer arduino sketch
* Author:   Jeffrey Marten Gillmor jeffrey.martengillmor@gmail.com
* Date:     15-Nov-2011
* Software: AVR-GCC
* Target:    Arduino
* 
* DESCRIPTION
* This is an arduino sketch that acts as a buffer between loconet and ethernet.
* It uses the LoconetOverTCP protocol that was designed by Stefan Bormann, a
* description of which is available here:
*        http://loconetovertcp.sourceforge.net/Protocol/index.html
* 
* 	As TCP packets are received, they are scanned for loconet messages, if a 
*   loconet message is successfuly decoded, it is then transmitted out over 
*   loconet. Likewise, if a loconet mesage is received via loconet, this is
*   packaged up and sent out over TCP.
* 
*   The Loconet RX is on arudino pin 8, the tx on arduino pin 6.
*   The Ethernet connection is via an arduino ethernet shield connected to 
*   arduino pins 13,12,11 & 10.
*****************************************************************************/

#include <LocoNet.h>
#include <SPI.h>
#include <Ethernet.h>


#define DEBUG /* enable this to print debug information via the serial port */

/* This is the TCP port used to communicate over ethernet 
   LoconetOcerTCP defaults to PORT 1024 */
#define TCP_PORT 1234

/* Modify these variables to match your network */
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 11, 177 };
byte gateway[] = {  192, 168, 11, 254 };
byte subnet[] = {  255, 255, 255, 0 };

const char digitMap[] = "0123456789abcdef";
const char receiveString[] = "RECEIVE ";
static   LnBuf        LnTxBuffer ;
static   lnMsg        *LnPacket;

/* LoconetOcerTCP defaults to PORT 1024 */
Server server(TCP_PORT);
boolean gotATCPMessage = false; // whether or not you got a message from the client yet


static void SendLoconetMessageViaTCP(lnMsg *LoconetPacket, uint8_t PacketSize);
static boolean ProcessTCPRxByte(uint8_t offset, uint8_t rxByte, LnBuf* LnTxBuffer);

#ifdef DEBUG
static void dumpPacket(char* dPacket);
#endif

void setup()
{
	/* First initialize the LocoNet interface */
	LocoNet.init();

	/* Configure the serial port for 57600 baud */
	Serial.begin(57600);

	/*Initialize a LocoNet packet buffer to buffer bytes from the PC */
	initLnBuf(&LnTxBuffer) ;

	/* initialise the ethernet device */
	Ethernet.begin(mac, ip, gateway, subnet);

	/*start listening for clients */
	server.begin();

}

void loop()
{  
	Client TcpClient = server.available();


	char RxByte = 0;
	static boolean IdenFound = false;
	static int IdenBytes = 0;
	int MsgIdx = 8;


	/* Check for any received loconet packets */
	LnPacket = LocoNet.receive() ;
	if( LnPacket )
	{
		/* If a packet has been received, get the length of the 
       loconet packet */
		uint8_t LnPacketSize = getLnMsgSize( LnPacket ) ;


#ifdef DEBUG
		Serial.print("Received a message of ");
		Serial.print(LnPacketSize, DEC);
		Serial.println(" Bytes from loconet,");
#endif

		/* Send the loconet packet out over TCP */
		SendLoconetMessageViaTCP(LnPacket, LnPacketSize);
	}


	/* Check for any received TCP packets */
	if (TcpClient)
	{
		if (!gotATCPMessage)
		{
			Serial.println("We have a new client");
			server.write("VERSION ArduinoLoconetEtherBuffer V0.2 built 17 November 2011\r\n");
			gotATCPMessage = true;
		}

		RxByte = TcpClient.read();

		switch (RxByte)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if(true == ProcessTCPRxByte(0x30, (uint8_t)RxByte, &LnTxBuffer))
			{
				IdenFound = false;
			}
			break;

		case 'A':
		case 'B':
		case 'C':
		case 'F':
			if(true == ProcessTCPRxByte(0x37, (uint8_t)RxByte, &LnTxBuffer))
			{
				IdenFound = false;
			}
			break;

		case ' ':
			/* Do nothing */
			break;

		case 'S':
			IdenBytes = 1;
			if (IdenFound == true)
			{
				IdenFound = false;
			}
			break;

		case 'E':
			if(IdenFound == true)
			{
				if(true == ProcessTCPRxByte(0x37, (uint8_t)RxByte, &LnTxBuffer))
				{
					IdenFound = false;
				}
			}
			else
			{
				IdenBytes++;
			}
			break;

		case 'N':
			IdenBytes++;
			break;

		case 'D':
			if(IdenFound == true)
			{
				if(true == ProcessTCPRxByte(0x37, (uint8_t)RxByte, &LnTxBuffer))
				{
					IdenFound = false;
				}
			}
			else
			{
				if(IdenBytes == 3)
				{
#ifdef DEBUG
					Serial.print("Received a TCP message: ");
#endif
					IdenFound = true;
				}
				IdenBytes = 0;
			}
			break;

		case -1:
		default:
			if (IdenFound == true)
			{
				server.write("SENT OK \r\n");
				IdenFound = false;
				IdenBytes = 0;
			}
			break;
		}
	}
	else if (IdenFound == true)
	{
		server.write("SENT OK \r\n");
#ifdef DEBUG
		Serial.println(" ");
		Serial.println("SENT OK \r\n");
#endif
		IdenFound = false;
		IdenBytes = 0;
	}
	else
	{
		IdenFound = false;
		IdenBytes = 0;
	}
}



static boolean ProcessTCPRxByte(uint8_t offset, uint8_t rxByte, LnBuf* LnTxBuffer)
{
	static boolean highNibble = true;
	static uint8_t loconetByte = 0; 
	static lnMsg   *LoconetPacket;

	if (highNibble)
	{
		loconetByte = (rxByte - offset) << 4;
		highNibble = false;
		return false;
	}
	else
	{
		loconetByte |= (rxByte - offset);
		highNibble = true;

#ifdef DEBUG
		Serial.print(" 0x");
		if(loconetByte < 16)
		{
			Serial.print("0");
		}
		Serial.print(loconetByte, HEX);
#endif
		/* Add it to the buffer */

		addByteLnBuf( LnTxBuffer, loconetByte ) ;

		/* Check to see if we have received a complete packet yet */
		LoconetPacket = recvLnMsg( LnTxBuffer ) ;
		if(LoconetPacket )
		{
			/* Send the packet received via TCP to the LocoNet*/
			LocoNet.send( LoconetPacket ) ;
#ifdef DEBUG
				Serial.println(" ");
#endif	
			return true;
		}
		return false;    
	}
}


static void SendLoconetMessageViaTCP(lnMsg *LoconetPacket, uint8_t PacketSize)
{
	char TcpTxMsg[8+(PacketSize*3)+2] ;
	uint8_t MsgIdx = 8;

	/* Initialise the the buffer that will be use to construct the message that is to 
           be transmitted over LoconetOverTCP*/
	memset(TcpTxMsg,0,sizeof(TcpTxMsg));
	memcpy(TcpTxMsg,receiveString, sizeof(receiveString));


	// Send the received packet out byte by byte to the PC
	for( uint8_t Index = 0; Index < PacketSize; Index++ )
	{
		/* Convert the high nibble of the current Loconet Packet byte to ASCII */
		TcpTxMsg[MsgIdx++] = (digitMap[(LoconetPacket->data[Index] >> 4) & 0xF]);

		/* Convert the low nibble of the current Loconet Packet byte to ASCII */
		TcpTxMsg[MsgIdx++] =(digitMap[(LoconetPacket->data[ Index ] & 0xF)]);

		/* Add a "space" between the current and next "loconet byte" */
		TcpTxMsg[MsgIdx++] = ' ';
	}
	/* Decrement the last MsgIdx, as there is no need for a space after the last byte */
	MsgIdx--;

	/* For some reason JMRI likes to have a "\r\n" at the end of the TCP message, though the
       loconetOverTCP standard states that an \r or an \n is accepetable */
	TcpTxMsg[MsgIdx++] = 0xD;
	TcpTxMsg[MsgIdx++] = 0xA;
	TcpTxMsg[MsgIdx] = 0;

	/* Send the message out over TCP */
	server.write(TcpTxMsg);

#ifdef DEBUG
	dumpPacket(TcpTxMsg);
#endif
}



#ifdef DEBUG
static void dumpPacket(char* dPacket)
{
	int i = 0;
	Serial.println("Dumping TX TCP Packet");
	Serial.print(" 0: ");
	while(*dPacket != 0)
	{
		Serial.print("0x:");
		if(*dPacket < 16)
		{
			Serial.print("0");
		}
		Serial.print(*dPacket,HEX);
		i++;
		*dPacket++;
		if((i%10) == 0)
		{
                        Serial.println("  ");
			Serial.print(i, DEC);
			Serial.print("  ");
		}
		else
		{
			Serial.print(" ");
		}
	} 
	Serial.println("\r\n");
}

#endif

