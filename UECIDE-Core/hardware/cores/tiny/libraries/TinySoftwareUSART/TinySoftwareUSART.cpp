
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <TinyUSARTConfig.h>

#include "wiring_private.h"

#include "TinySoftwareUSART.h"


#define USART_PORT_CONCAT(a, b)            a ## b

#define USART_OUTPORT(name)           USART_PORT_CONCAT(PORT, name)
#define USART_INPORT(name)            USART_PORT_CONCAT(PIN, name)
#define USART_DDRPORT(name)           USART_PORT_CONCAT(DDR, name)

//USART Data/Control Registers
#define USART_RXData  GPIOR2
#define USART_TXData  GPIOR1
#define USART_Control GPIOR0
//USART Control Register Bit Masks
#define USART_Control_RX_Busy        (7)
#define USART_Control_RX_Stop        (6)
#define USART_Control_RX_Error       (5) //Stop bit expected, but not recieved, data was dropped. Or buffer overflow.
#define USART_Control_TX_Busy        (4)
#define USART_Control_TX_Start_Stop  (3)
#define USART_Control_TX_Count_Mask (_BV(2) | _BV(1) | _BV(0))

#if !defined(TinySoftwareUSART_Rx_Buffer)
  #error RX Buffer not found. Please #define TinySoftwareUSART_Rx_Buffer as the name of your extern USARTRingBuffer rx_buffer
#elif !defined(TinySoftwareUSART_Tx_Buffer)
  #error TX Buffer not found. Please #define TinySoftwareUSART_Tx_Buffer as the name of your extern USARTRingBuffer tx_buffer
#elif !defined(TinySoftwareUSART_Port_Name)
  #error Cannot find the interrupt enable register. Please define TinySoftwareUSART_Port_Name as e.g. B
#elif !defined(TinySoftwareUSART_TX_Bit)
  #error Cannot find the tx pin bit. Please define TinySoftwareUSART_TX_Bit as e.g. PORTB0
#elif !defined(TinySoftwareUSART_RX_Bit)
  #error Cannot find the rx pin bit. Please define TinySoftwareUSART_RX_Bit as e.g. PORTB1
#elif !defined(TinySoftwareUSART_CK_Bit)
  #error Cannot find the clock pin bit. Please define TinySoftwareUSART_CK_Bit as e.g. PORTB2
#elif !defined(TinySoftwareUSART_Config_Reg)
  #error Cannot find the interrupt configuration register. Please define TinySoftwareUSART_Config_Reg as e.g. MCUCR
#elif !defined(TinySoftwareUSART_Config_Set_Bits)
  #error Cannot find the bits to set in the interrupt configuration register. Please define TinySoftwareUSART_Config_Set_Bits as e.g. (_BV(ISC00) | _BV(ISC01))
#elif !defined(TinySoftwareUSART_Config_Clear_Bits)
  #error Cannot find the bits to clear in the interrupt configuration register. Please define TinySoftwareUSART_Config_Clear_Bits as e.g. (_BV(ISC01) | _BV(ISC00))
#elif !defined(TinySoftwareUSART_Interrupt_Enable_Reg)
  #error Cannot find the interrupt enable register. Please define TinySoftwareUSART_Interrupt_Enable_Reg as e.g. GIMSK
#elif !defined(TinySoftwareUSART_Interrupt_Flag_Reg)
  #error Cannot find the interrupt flag register. Please define TinySoftwareUSART_Interrupt_Flag_Reg as e.g. GIFR
#elif !defined(TinySoftwareUSART_Interrupt_Enable_Bit)
  #error Cannot find the interrupt enable bit. Please define TinySoftwareUSART_Interrupt_Enable_Bit as e.g. INT0
#elif !defined(TinySoftwareUSART_Interrupt_Flag_Bit)
  #error Cannot find the interrupt flag bit. Please define TinySoftwareUSART_Interrupt_Flag_Bit as e.g. INTF0
#elif !defined(TinySoftwareUSART_vect)
  #error Tiny Software USART cannot find the its interrupt vector! Please check TinySoftwareUSART_vect is defined
#else

ISR(TinySoftwareUSART_vect, ISR_NAKED){
  //Interrupt Entry
  asm volatile (
      "push r30           \n\t"
      "in   r30, __SREG__ \n\t" //back up the SREG
      "push r30           \n\t"
      "push r31           \n\t"
      :::
  );
  {
    //Transmit
    if (USART_Control & _BV(USART_Control_TX_Busy)){
      //Busy sending packet
      if (USART_Control & _BV(USART_Control_TX_Start_Stop)){
        //Stop Bit
        asm volatile (
          "sbi  %2 , %3     \n\t" //output stop bit
          "lds  r30, %0     \n\t"
          "lds  r31, %1     \n\t"
          "cp   r31, r30    \n\t" //check if head == tail for TX buffer
          "brne 2f          \n\t"
          "cbi  %4 , %5     \n\t" //buffer empty, so finished sending
          "2:               \n\t"
          "cbi  %4 , %6     \n\t" //no longer busy. If start_stop is set, then next cycle will set busy flag again.
          :
          : "i" (&TinySoftwareUSART_Tx_Buffer.head), 
            "i" (&TinySoftwareUSART_Tx_Buffer.tail), 
            "I" (_SFR_IO_ADDR(USART_OUTPORT(TinySoftwareUSART_Port_Name))), 
            "I" (TinySoftwareUSART_TX_Bit), 
            "I" (_SFR_IO_ADDR(USART_Control)), 
            "I" (USART_Control_TX_Start_Stop), 
            "I" (USART_Control_TX_Busy) 
          : "r30", "r31"
        );
      } else {
        //Data bit
        asm volatile (
          "in   r30, %0   \n\t"
          "ror  r30       \n\t" //rotate into carry (don't care what gets shifted in!)
          "out  %0 , r30  \n\t" //store data back to tx reg for next cycle.
          "cbi  %1 , %2   \n\t" //assume a zero to begin with
          "brcc 1f        \n\t" //if the carry is clear, then assumption was correct.
          "sbi  %1 , %2   \n\t" //else we know to output a 1
          "1:             \n\t"
          "in   r30, %3   \n\t" //read in the control register
          "mov  r31, r30  \n\t" //make a copy for later.
          "inc  r31       \n\t" //one more bit sent
          "andi r31, %4   \n\t" //mask off the counter
          "brne 1f        \n\t" //if not zero, still more to send, so skip the next bit.
          "ori  r30, %5   \n\t" //Set the start_stop flag as the next bit to come is the stop bit.
          "1:             \n\t"
          "andi r30, ~%4  \n\t" //mask out the counter
          "or   r30, r31  \n\t" //or in the new value
          "out  %3 , r30  \n\t" //output back to the control register
          : 
          : "I" (_SFR_IO_ADDR(USART_TXData)), 
            "I" (_SFR_IO_ADDR(USART_OUTPORT(TinySoftwareUSART_Port_Name))), 
            "I" (TinySoftwareUSART_TX_Bit), 
            "I" (_SFR_IO_ADDR(USART_Control)), 
            "M" (USART_Control_TX_Count_Mask), 
            "M" (_BV(USART_Control_TX_Start_Stop))
          : "r30", "r31"
        );
      }
    } else if (USART_Control & _BV(USART_Control_TX_Start_Stop)) {
      //Start bit
      register unsigned char * ptr asm("r30");
      asm volatile (
        "lds  %A0, %1   \n\t" //Load in TX Buffer Tail
        "ldi  %B0, 0x1  \n\t" //
        "add  %B0, %A0  \n\t" //New Tail is old tail + 1
        "andi %B0, %2   \n\t" //Mask off unused bits to match ring buffer size
        "sts  %1 , %B0  \n\t" //Store new tail back into TX Buffer.
        "ldi  %B0, 0x0  \n\t"
        : "=z" (ptr)
        : "i"  (&TinySoftwareUSART_Tx_Buffer.tail), 
          "M"  (SERIAL_BUFFER_SIZE-1)
        : 
      );
      ptr = (unsigned char*)((unsigned int)ptr + (unsigned int)TinySoftwareUSART_Tx_Buffer.buffer); //offset pointer by address of buffer (already contains the index in the buffer, and a[b] == b + a)
      asm volatile (
        "ld   %A0, %a0  \n\t" //Load in the data from the buffer
        "out  %1 , %A0  \n\t" //Output to TX Data buffer
        "cbi  %2 , %3   \n\t" //Output the start bit
        "in   %A0, %4   \n\t" //Load in the control register
        "andi %A0, ~%5  \n\t" //next cycle is data 0, so clear counter and clear start/stop marker
        "ori  %A0, %6   \n\t" //set the busy marker as transferring data has begun
        "out  %4 , %A0  \n\t" //Output back to control register.
        : 
        : "z" (ptr),
          "I" (_SFR_IO_ADDR(USART_TXData)),
          "I" (_SFR_IO_ADDR(USART_OUTPORT(TinySoftwareUSART_Port_Name))), 
          "I" (TinySoftwareUSART_TX_Bit), 
          "I" (_SFR_IO_ADDR(USART_Control)), 
          "M" (_BV(USART_Control_TX_Start_Stop) | USART_Control_TX_Count_Mask), 
          "M" (_BV(USART_Control_TX_Busy))
        : 
      );
    } //else no data to send.
  }
  
  {
    //Recieve
    if (USART_Control & _BV(USART_Control_RX_Stop)) { //Stop Bit
      if (USART_INPORT(TinySoftwareUSART_Port_Name) & _BV(TinySoftwareUSART_RX_Bit)) { //Check if there is in fact a stop bit
        register unsigned char * ptr asm("r30");
        asm volatile (
          "push r0          \n\t" //need an extra register unfortunately!
          "lds  %B0, %1     \n\t" //Read in current head.
          "mov  r0 , %B0    \n\t" //backup head value as about to mess with it.
          "inc  %B0         \n\t" 
          "andi %B0, %2     \n\t" //Increment head and mask off as a ring buffer.
          "lds  %A0, %3     \n\t" //Read in current tail.
          "cp   %A0, %B0    \n\t" //check if head != tail for TX buffer
          "breq 1f          \n\t" //Check if head == tail, and if so jump to setting the error bit. Otherwise there is space in the buffer.
          "sts  %1 , %B0    \n\t" //Store the new head back into RX Buffer.
          "mov  %A0, r0     \n\t" //Restore head value from backup as using it to calculate address.
          "ldi  %B0, 0x0    \n\t"
          : "=z" (ptr)
          : "i" (&TinySoftwareUSART_Rx_Buffer.head),
            "M" (SERIAL_BUFFER_SIZE-1),
            "i" (&TinySoftwareUSART_Rx_Buffer.tail)
          :
        );
        ptr = (unsigned char*)((unsigned int)ptr + (unsigned int)TinySoftwareUSART_Rx_Buffer.buffer); //offset pointer by address of buffer (already contains the index in the buffer, and a[b] == b + a)
        asm volatile (
          "in   r0 , %1     \n\t" //Read in the new data
          "st   %a0, r0     \n\t" //Store the data into the buffer.
          "pop  r0          \n\t" //finished with it!
          : 
          : "z" (ptr),
            "I" (_SFR_IO_ADDR(USART_RXData))
          : 
        );
      } else { //Otherwise a framing error occurred.
        asm volatile (
          "1:             \n\t"
          "sbi  %0 , %1   \n\t" //Set the error bit.
          :
          : "I" (_SFR_IO_ADDR(USART_Control)),
            "I" (USART_Control_RX_Error)
          : 
        );
      }
      asm volatile (
        "in   r30, %0   \n\t" //Load in the control register
        "andi r30, ~%1  \n\t" //no longer busy or a start bit.
        "out  %0 , r30  \n\t" //Output back to control register.
        : 
        : "I" (_SFR_IO_ADDR(USART_Control)), 
          "M" (_BV(USART_Control_RX_Busy) | _BV(USART_Control_RX_Stop))
        : "r30"
      );
      //USART_Control &= ~(_BV(USART_Control_RX_Busy) | _BV(USART_Control_RX_Stop));
    } else if (USART_Control & _BV(USART_Control_RX_Busy)) { //Data Bit
      asm volatile (
        "in   r30, %0   \n\t"
        "clc            \n\t" //assume data in is a 0
        "sbic %1 , %2   \n\t" //check if assumption is correct
        "sec            \n\t" //set carry if data in was a 1
        "ror  r30       \n\t" //shift carry into data register (next bit in)
        "brcc 1f        \n\t" //if the carry is clear, then there is still data to come.
        "sbi  %3 , %4   \n\t" //We know now that all 8 data bits have been clocked in, so inform that we are now awaiting a stop bit
        "1:             \n\t"
        "out  %0 , r30  \n\t" //store back to rx data register
        : 
        : "I" (_SFR_IO_ADDR(USART_RXData)), 
          "I" (_SFR_IO_ADDR(USART_INPORT(TinySoftwareUSART_Port_Name))), 
          "I" (TinySoftwareUSART_RX_Bit), 
          "I" (_SFR_IO_ADDR(USART_Control)), 
          "I" (USART_Control_RX_Stop)
        : "r30"
      );
    } else if (!(USART_INPORT(TinySoftwareUSART_Port_Name) & _BV(TinySoftwareUSART_RX_Bit))) { //Start Bit 
      asm volatile (
        "ldi  r30, 0x80 \n\t" //initialise data. Carry flag will be zero until 8 bits have been shited in at which point this 1 will drop off the end.
        "out  %0 , r30  \n\t" //Output to data register
        "sbi  %1 , %2   \n\t" //Now busy processing the recieve packet.
        : 
        : "I" (_SFR_IO_ADDR(USART_RXData)), 
          "I" (_SFR_IO_ADDR(USART_Control)),
          "I" (USART_Control_RX_Busy)
        : "r30"
      );
    } //else idle.
  }
  
  //Interrupt Exit 
  asm volatile (
      "pop  r31           \n\t"
      "pop  r30           \n\t"
      "out  __SREG__, r30 \n\t" //restore the SREG
      "pop  r30           \n\t"
      "reti               \n\t"
      :::
  );
  //sbi(TinySoftwareUSART_Interrupt_Flag_Reg,TinySoftwareUSART_Interrupt_Flag_Bit); //clear the flag - may not be needed.
}


// Constructor ////////////////////////////////////////////////////////////////

TinySoftwareUSART::TinySoftwareUSART()
{
  _rx_buffer = &TinySoftwareUSART_Rx_Buffer;
  _tx_buffer = &TinySoftwareUSART_Tx_Buffer;
}

// Public Methods //////////////////////////////////////////////////////////////

void TinySoftwareUSART::begin()
{
  USART_DDRPORT(TinySoftwareUSART_Port_Name) &= ~(_BV(TinySoftwareUSART_RX_Bit) | _BV(TinySoftwareUSART_CK_Bit)); //inputs
  USART_OUTPORT(TinySoftwareUSART_Port_Name) |= (_BV(TinySoftwareUSART_TX_Bit) | _BV(TinySoftwareUSART_RX_Bit) | _BV(TinySoftwareUSART_CK_Bit)); //pull-up resistors and stop bit.
  USART_DDRPORT(TinySoftwareUSART_Port_Name) |= _BV(TinySoftwareUSART_TX_Bit); //output
  
  USART_Control = 0; //reset the control register.
  
#ifdef TinySoftwareUSART_Using_Analog_Comp
  //The analog comparator requires extra setup.
  cbi(ACSR,ACIE);  //turn off the comparator interrupt to allow change of ACD
#ifdef ACBG
  sbi(ACSR,ACBG); //enable the internal bandgap reference - used instead of AIN0 to allow it to be used for TX.
#endif
  cbi(ACSR,ACD);  //turn on the comparator for RX
#ifdef ACIC
  cbi(ACSR,ACIC);  //prevent the comparator from affecting timer1 - just to be safe.
#endif
#ifdef ACSRB
  ACSRB = 0; //Use AIN0 as +, AIN1 as -, no hysteresis - just like ones without this register.
#endif

#endif

  TinySoftwareUSART_Config_Reg |= TinySoftwareUSART_Config_Set_Bits;  //interrupt on rising edge
  TinySoftwareUSART_Config_Reg &= ~TinySoftwareUSART_Config_Clear_Bits;
  
  TinySoftwareUSART_Interrupt_Flag_Reg |= _BV(TinySoftwareUSART_Interrupt_Flag_Bit); //clear the flag before enabling.
  TinySoftwareUSART_Interrupt_Enable_Reg |= _BV(TinySoftwareUSART_Interrupt_Enable_Bit); //enable the interrupt
}

void TinySoftwareUSART::end()
{
  TinySoftwareUSART_Interrupt_Flag_Reg |= _BV(TinySoftwareUSART_Interrupt_Flag_Bit); //clear the flag
  TinySoftwareUSART_Interrupt_Enable_Reg &= ~_BV(TinySoftwareUSART_Interrupt_Enable_Bit); //disable the interrupt
  
  _rx_buffer->head = _rx_buffer->tail;
  
#ifdef TinySoftwareUSART_Using_Analog_Comp
#ifdef ACBG
  cbi(ACSR,ACBG); //disable the bandgap reference
#endif
  sbi(ACSR,ACD);  //turn off the comparator to save power
#endif

  USART_DDRPORT(TinySoftwareUSART_Port_Name) &= ~_BV(TinySoftwareUSART_TX_Bit); //release the TX pin.
}

int TinySoftwareUSART::available(void)
{
  unsigned char avail = (_rx_buffer->head - _rx_buffer->tail) & (SERIAL_BUFFER_SIZE - 1);
  return (int)avail;
}

int TinySoftwareUSART::peek(void)
{
  unsigned char tail = _rx_buffer->tail;
  if (_rx_buffer->head == tail) {
    return -1;
  } else {
    return _rx_buffer->buffer[tail];
  }
}

int TinySoftwareUSART::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  unsigned char tail = _rx_buffer->tail;
  if (_rx_buffer->head == tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer->buffer[tail];
    _rx_buffer->tail = (tail + 1) & (SERIAL_BUFFER_SIZE - 1);
    return c;
  }
}

size_t TinySoftwareUSART::write(uint8_t ch)
{
  unsigned char head = _tx_buffer->head;
  unsigned char newHead = (_tx_buffer->head + 1) & (SERIAL_BUFFER_SIZE - 1);
	
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  // ???: return 0 here instead?
  while (newHead == _tx_buffer->tail)
    ;
	
  _tx_buffer->buffer[head] = ch;
  _tx_buffer->head = newHead;
  
  unsigned char oldSREG = SREG;
  cli();
  if (!(USART_Control & _BV(USART_Control_TX_Busy))){
    //If not busy, then set the start flag to begin transfer.
    USART_Control |= _BV(USART_Control_TX_Start_Stop);
  } //Otherwise it will start automatically when the current one is complete.
  SREG = oldSREG;
  return 1;
}

void TinySoftwareUSART::flush()
{
  while (_tx_buffer->head != _tx_buffer->tail);
}

TinySoftwareUSART::operator bool() {
  return true;
}
#endif
