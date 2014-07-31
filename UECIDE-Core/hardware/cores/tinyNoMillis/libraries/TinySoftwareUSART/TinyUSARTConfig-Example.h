
#ifndef TinyUSARTConfig_h
#define TinyUSARTConfig_h
#include "TinySoftwareUSART.h"

extern USARTRingBuffer rx_buffer; //You can call these whatever you want, just make sure the #defines below match
extern USARTRingBuffer tx_buffer; //These must be initialised in your sketch as: USARTRingBuffer tx_buffer={0};

//Define the TX and RX USARTRingBuffer variable names. These will be used for data in/out privately
#define TinySoftwareUSART_Rx_Buffer             rx_buffer
#define TinySoftwareUSART_Tx_Buffer             tx_buffer

//Define the Port Name for the pins used for USART, e.g. Port B would be 'B'
#define TinySoftwareUSART_Port_Name             B

//Define the bit numbers of each of the three communication pins
#define TinySoftwareUSART_TX_Bit                PORTB0
#define TinySoftwareUSART_RX_Bit                PORTB1
#define TinySoftwareUSART_CK_Bit                PORTB2

//Define the register used to configure the interrupt
#define TinySoftwareUSART_Config_Reg            MCUCR
//Define the bit mask which contains all bits to be set such that a rising edge interrupt can be selected
#define TinySoftwareUSART_Config_Set_Bits       (_BV(ISC00) | _BV(ISC01))
//Also define a bit mask which contains all bits to be cleared from the config register
#define TinySoftwareUSART_Config_Clear_Bits     0

//Define the register names for enabling the interrupt and reading its flag
#define TinySoftwareUSART_Interrupt_Enable_Reg  GIMSK
#define TinySoftwareUSART_Interrupt_Flag_Reg    GIFR

//Define the bits for the interrupt enable and flag
#define TinySoftwareUSART_Interrupt_Enable_Bit  INT0
#define TinySoftwareUSART_Interrupt_Flag_Bit    INTF0

//Define the interrupt vector name
#define TinySoftwareUSART_vect                  INT0_vect


/* Notes:
  A file containing the above #defines should be located in your sketch folder in which any configuration of pins/etc. should be made.
  The file should be called "TinyUSARTConfig.h".
  
  You cannot have more than one instance of TinySoftwareUSART.
  
  It is possible to use the external interrupt pins, INT0 or INT1 (or others if there are more) for the clock pin. For these,
  the interrupt needs configuring for falling edge using the Config_Set_Bits and Config_Clear_Bits.
  
  Alternatively, you can connect the clock to the Analog Comparator (AIN1) in which case the internal bandgap is used as a reference
  and you need to make sure Config_Set_Bits and Config_Clear_Bits select a rising edge of ACO as the interrupt source.
  
  To use Analog Comparator, additional setup is reguired, so #define the following to have it inserted:
  #define TinySoftwareUSART_Using_Analog_Comp
*/

#endif
