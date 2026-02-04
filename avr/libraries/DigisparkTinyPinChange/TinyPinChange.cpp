/********************************************************************************/
/* PROJECT:   All based on ATtinyX5, ATtinyX4, Atiny167, ATmega328P, ATmega2560 */
/* MODULE:    TinyPinChange                                                     */
/* VERSION:   1.2 (20/12/2014)                                                  */
/* DATE:      30/01/2011                                                        */
/* TARGET:    ATtinyX5, ATtinyX4, ATtiny167, ATmega328P, ATmega2560             */
/* COMPILER:  WinAvr, avr-gcc, avr-g++                                          */
/* IDE:       ARDUINO, AVR Studio 4                                             */
/* PROGRAMER: AVR-JTAG-ICE MKII, ARDUINO IDE                                    */
/* AUTHOR:    P.LOUSSOUARN     (P.Loussouarn: http://p.loussouarn.free.fr)      */
/********************************************************************************/
#include <TinyPinChange.h>
#include <avr/interrupt.h>

/*************************************************************************
								MACROS
*************************************************************************/

#define PIN_CHANGE_HANDLER_MAX_NB	3	/* ISR max number Pin Change ISR can handle per port */


/*************************************************************************
							GLOBAL VARIABLES
*************************************************************************/
struct PinChangeStruct
{
	void			(*Isr[PIN_CHANGE_HANDLER_MAX_NB])(void);
	uint8_t			LoadedIsrNb;
	uint8_t			Event;
	uint8_t			PinPrev;
	uint8_t			PinCur;
};

struct PinChangePortStruct
{
	PinChangeStruct Port[PIN_CHG_PORT_NB];
};

static volatile struct PinChangePortStruct	PinChange;

/*************************************************************************
							INTERRUPT SUB-ROUTINE
*************************************************************************/
#define DECLARE_PIN_CHANGE_ISR(VirtualPortIdx)																			\
ISR(PCINT##VirtualPortIdx##_vect)																						\
{																														\
uint8_t Idx;																											\
	PinChange.Port[VirtualPortIdx].PinCur = (PC_PIN##VirtualPortIdx) & (PC_PCMSK##VirtualPortIdx);							\
	PinChange.Port[VirtualPortIdx].Event = PinChange.Port[VirtualPortIdx].PinPrev ^ PinChange.Port[VirtualPortIdx].PinCur;	\
	PinChange.Port[VirtualPortIdx].PinPrev = PinChange.Port[VirtualPortIdx].PinCur;										\
	for(Idx = 0; Idx < PinChange.Port[VirtualPortIdx].LoadedIsrNb; Idx++)														\
	{																													\
		PinChange.Port[VirtualPortIdx].Isr[Idx]();																		\
	}																													\
}

DECLARE_PIN_CHANGE_ISR(0)

#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
DECLARE_PIN_CHANGE_ISR(1)
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
DECLARE_PIN_CHANGE_ISR(2)
#endif

/*************************************************************************
							PUBLIC FUNCTIONS
*************************************************************************/

/*********************************************************************
	PinChange Initialization Function
Input:
	Void
Output:
	Void
*********************************************************************/
void TinyPinChange_Init(void)
{
/* ATtinyX5, ATtiny167, ATtinyX4, UNO, MEGA */
	PinChange.Port[0].PinCur = PC_PIN0 & PC_PCMSK0;//PINB for ATtinyX5, UNO or MEGA, PINA for ATtinyX4 or ATtiny167
	PinChange.Port[0].PinPrev = PinChange.Port[0].PinCur;
#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
/* ATtinyX4, ATtiny167, UNO or MEGA */
	PinChange.Port[1].PinCur = PC_PIN1 & PC_PCMSK1;//PINB for for ATtinyX4 or ATtiny167, PINC for UNO, PINJ for MEGA
	PinChange.Port[1].PinPrev = PinChange.Port[1].PinCur;
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
/* UNO or MEGA */
	PinChange.Port[2].PinCur = PC_PIN2 & PC_PCMSK2;//PIND for UNO, PINK for MEGA
	PinChange.Port[2].PinPrev = PinChange.Port[2].PinCur;
#endif
}

/*********************************************************************
	PinChange RegisterIsr Function
Input:
	Pointer on a PinChange Function
Output:
	The associated VirtualPortIdx (0 to 2)
	< 0 in case of failure
*********************************************************************/
int8_t TinyPinChange_RegisterIsr(uint8_t Pin, void(*Isr)(void))
{
int8_t IsrIdx, PortIdx, AlreadyLoaded = 0;

	PortIdx = DigitalPinToPortIdx(Pin);

	for(IsrIdx = 0; IsrIdx < PIN_CHANGE_HANDLER_MAX_NB; IsrIdx++)
	{
		if(PinChange.Port[PortIdx].Isr[IsrIdx] == Isr)
		{
			AlreadyLoaded = 1;
			break; /* Already loaded */
		}
	}
	
	if(!AlreadyLoaded)
	{
		if(PinChange.Port[PortIdx].LoadedIsrNb < PIN_CHANGE_HANDLER_MAX_NB)
		{
			/* Not aready loaded: load it */
			PinChange.Port[PortIdx].Isr[PinChange.Port[PortIdx].LoadedIsrNb] = Isr;
			PinChange.Port[PortIdx].LoadedIsrNb++;
		}
		else PortIdx = -1; /* Failure */
	}
	return(PortIdx);
}

/*********************************************************************
	PinChange Enable Pin Function
Input:
	Pin: the Pin
Output:
	Void
*********************************************************************/
void TinyPinChange_EnablePin(uint8_t Pin)
{
    if(digitalPinToPCICR(Pin))
    {
      *digitalPinToPCICR(Pin) |= _BV(digitalPinToPCICRbit(Pin));
      *digitalPinToPCMSK(Pin) |= _BV(digitalPinToPCMSKbit(Pin));
    }
}

/*********************************************************************
	PinChange Disable Pin Function
Input:
	Pin: the Pin
Output:
	Void
*********************************************************************/
void TinyPinChange_DisablePin(uint8_t Pin)
{
    if(digitalPinToPCICR(Pin))
    {
	  *digitalPinToPCMSK(Pin) &= (_BV(digitalPinToPCMSKbit(Pin)) ^ 0xFF);
    }
}

/*********************************************************************
	PinChange GetPortEvent Function
Input:
	VirtualPortIdx: Index of the Port
Output:
	The bits which have changed in the port
*********************************************************************/
uint8_t TinyPinChange_GetPortEvent(uint8_t VirtualPortIdx)
{
	return(PinChange.Port[VirtualPortIdx].Event);
}

/*********************************************************************
	PinChange GetCurPortSt Function
Input:
	VirtualPortIdx: Index of the Port
Output:
	Current Status of the port
*********************************************************************/
uint8_t TinyPinChange_GetCurPortSt(uint8_t VirtualPortIdx)
{
	return(PinChange.Port[VirtualPortIdx].PinCur);
}
