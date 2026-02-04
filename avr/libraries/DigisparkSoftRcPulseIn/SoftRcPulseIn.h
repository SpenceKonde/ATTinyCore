/*
 English: by RC Navy (2012-2015)
 =======
 <SoftRcPulseIn>: an asynchronous library to read Input Pulse Width from standard Hobby Radio-Control. This library is a non-blocking version of pulseIn().
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012-2015)
 ========
 <SoftRcPulseIn>: une librairie asynchrone pour lire les largeur d'impulsions des Radio-Commandes standards. Cette librairie est une version non bloquante de pulsIn().
 http://p.loussouarn.free.fr
*/

#ifndef SOFT_RC_PULSE_IN_H
#define SOFT_RC_PULSE_IN_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <TinyPinChange.h>

#include <inttypes.h>

#define SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT

class SoftRcPulseIn
{
  public:
    SoftRcPulseIn();
    static int   LibVersion(void);
    static int   LibRevision(void);
    static char *LibTextVersionRevision(void);
    static void  SoftRcPulseInInterrupt(void);
    uint8_t      attach(uint8_t Pin, uint16_t PulseMin_us = 600, uint16_t PulseMax_us = 2400);
    boolean      available();
    boolean      timeout(uint8_t TimeoutMs, uint8_t *State);
    uint16_t     width_us();
    private:
    class SoftRcPulseIn  *next;
    static SoftRcPulseIn *first;
	uint8_t _Pin;
	uint8_t _PinMask;
	uint8_t _VirtualPortIdx;
	uint16_t _Min_us;
	uint16_t _Max_us;
	uint32_t _Start_us;
	uint32_t _Width_us;
	boolean  _Available;
#ifdef SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT
	uint8_t _LastTimeStampMs;
#endif
};
/*******************************************************/
/* Application Programming Interface (API) en Francais */
/*******************************************************/

/*      Methodes en Francais                            English native methods */
#define attache											attach
#define disponible										available
#define largeur_us										width_us

#endif
