/* Copyright (c) 2002, 2003, 2004  Marek Michalkiewicz
   Copyright (c) 2005, 2007 Joerg Wunsch
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

//	Port to Energia / MPS430 by Yannick DEVOS XV4Y - (c) 2013
//	http://xv4y.radioclub.asia/
//	

/* $Id: crc16.h 2136 2010-06-08 12:03:38Z joerg_wunsch $ */

#ifndef _UTIL_CRC16_H_
#define _UTIL_CRC16_H_

#include <stdint.h>

#define lo8(x) ((x)&0xff) 
#define hi8(x) ((x)>>8)

    uint16_t crc16_update(uint16_t crc, uint8_t a)
    {
	int i;

	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
	    if (crc & 1)
		crc = (crc >> 1) ^ 0xA001;
	    else
		crc = (crc >> 1);
	}

	return crc;
    }

    uint16_t crc_xmodem_update (uint16_t crc, uint8_t data)
    {
        int i;

        crc = crc ^ ((uint16_t)data << 8);
        for (i=0; i<8; i++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }

        return crc;
    }
    uint16_t _crc_ccitt_update (uint16_t crc, uint8_t data)
    {
        data ^= lo8 (crc);
        data ^= data << 4;

        return ((((uint16_t)data << 8) | hi8 (crc)) ^ (uint8_t)(data >> 4) 
                ^ ((uint16_t)data << 3));
    }

    uint8_t _crc_ibutton_update(uint8_t crc, uint8_t data)
    {
	uint8_t i;

	crc = crc ^ data;
	for (i = 0; i < 8; i++)
	{
	    if (crc & 0x01)
	        crc = (crc >> 1) ^ 0x8C;
	    else
	        crc >>= 1;
	}

	return crc;
    }


#endif /* _UTIL_CRC16_H_ */
