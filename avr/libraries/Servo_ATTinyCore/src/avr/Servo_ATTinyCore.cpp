/*
 Servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 Copyright (c) 2009 Michael Margolis.  All right reserved.
 Copyright (c) 2011 Ilya Brutman.  All right reserved.
 Copyright (c) 2019 Spence Konde.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if defined(ARDUINO_ARCH_AVR)

#include <avr/interrupt.h>
#include <Arduino.h>

#include "Servo.h"

#if (defined(__AVR_ATtinyX5__) || defined (__AVR_ATtinyX61__))


    #define TCNTn   TCNT1
    #define OCRnx   OCR1A
    #define OCFnx   OCF1A
    #define OCIEnx  OCIE1A
// Trim Duration is about the total combined time spent inside the Compare Match ISR
// This time is in timer ticks, where each tick is always 8 microseconds.
#define TRIM_DURATION 4


//This is the driver class responsible for generating the servo control pulses
// This class is purely static.
class ServoSequencer
{
public:
    //=============================================================================
    // Servo Sequencer public functions
    //=============================================================================
    static uint8_t  registerServo();
    //              Reserves a slot for a servo in the pulse sequence

    static void     deregisterServo(uint8_t servoSlotNumber);
    //              Frees an occupied slot

    static void     setServoPulseLength(uint8_t servoNumber, uint16_t newLengthInMicroseconds);
    //              Updates the pulse length of a servo to a new value

    static uint16_t getServoPulseLength(uint8_t servoNumber);
    //              returns the pulse length of a servo in microseconds

    static void     setServoPin(uint8_t servoNumber, uint8_t newPin);
    //              Updates the pin that will be pulsed

    static void     enableDisableServo(uint8_t servoNumber, bool servoShouldBeEnabled);
    //              Enable or disable a servo in a slot. A pulse is not generated for a servo slot that is disabled.

    static bool     isEnabled(uint8_t servoNumber);
    //              returns true is the servo is enabled

    inline static void timerCompareMatchISR();
    //              Handles a timer compare match.
    //              Should only be called when the timer compare match interrupt fires.

    //=============================================================================
    // Servo Sequencer public constants
    //=============================================================================
    //static const uint8_t kInvalidServoIndex = 0xFF;



private:
    //=============================================================================
    // Servo Sequencer types
    //=============================================================================
    //This enum defines the states of the Servo Sequencer
    enum SequencerState_t
    {
        WAITING_FOR_512_MARK,
        WAITING_TO_SET_PIN_LOW,
        WAITING_FOR_2048_MARK,
        WAITING_TO_SET_PIN_HIGH
    };

    //This struct defines a single servo slot
    struct ServoEntry
    {
        uint8_t  pulseLengthInTicks;    //length of pulse in ticks after offset is applied
        uint8_t  pin;                   //which pin to pulse on portB
        bool     enabled;               //True when this servo should be pulsed
        bool     slotOccupied;          //True when this servo entry is allocated to a servo
    };

    //=============================================================================
    // Servo Sequencer private variables
    //=============================================================================
      //The number of servos to support. See NOTE1 below.
    static volatile SequencerState_t  state;                            //The current state of the driver
    static          bool              timerIsSetup;                     //True if the timer used by this driver was configured
    static          bool              servoArrayIsInited;               //True if the servo Registry array was initialized with default values.
    static          ServoEntry        servoRegistry[MAX_SERVOS]; //The array of servo slots
    static volatile uint8_t           servoIndex;                       //The index of the current servo slot we are working with.
                                                                        //With 5 servos, we go through the whole servoRegistry every 20 milliseconds exactly.

    //=============================================================================
    // Servo Sequencer private functions
    //=============================================================================
    //This is a purely static class. Disallow making instances of this class.
    ServoSequencer();              //private constructor
    static void servoTimerSetup(); //Configures the timer used by this driver
    static void setupTimerPrescaler(); //helper function to setup the prescaler
    static void initServoArray();  //sets default values to each element of the servoRegistry array

};//end ServoSequencer



//=============================================================================
// Servo Sequencer static variables initialization
//=============================================================================
volatile ServoSequencer::SequencerState_t ServoSequencer::state         = ServoSequencer::WAITING_TO_SET_PIN_HIGH;
         bool                             ServoSequencer::timerIsSetup  = false;
         bool                             ServoSequencer::servoArrayIsInited = false;
volatile uint8_t                          ServoSequencer::servoIndex    = 0;
         ServoSequencer::ServoEntry       ServoSequencer::servoRegistry[MAX_SERVOS];
         //TODO: Add the rest of the class variables here for better organization?


//NOTE1:
// With 5 servos, each servo is pulsed exactly every 20 milliseconds. You can increase this number to have
// the driver support more servos. The servos are pulsed in sequence and each servo takes up 4 ms of time.
// So if you change this number to 6 then each servo will be pulsed every 24 milliseconds.



//=============================================================================
// FUNCTION:    bool registerServo()
//
// DESCRIPTION: Reserves a slot for a servo in the pulse sequence.
//              There is a limited number of slots.
//
// INPUT:       Nothing
//
// RETURNS:     On success returns the slot number.
//              If no free slot is found returns INVALID_SERVO.
//=============================================================================
uint8_t ServoSequencer::registerServo()
{

    if(servoArrayIsInited == false)
    {
        initServoArray();
    }
    else
    {
        //the servo array is already inited. Do nothing.
        //It needs to be setup only once. We do it when the first servo is registered.
    }


    //find a free slot in the servo registry
    for(uint8_t i = 0; i < MAX_SERVOS; i++)
    {
        if(servoRegistry[i].slotOccupied == false)
        {
            //found a free slot.
            servoRegistry[i].slotOccupied = true;
            //return the slot number
            return i;
        }
        else
        {
            //this slot is not free, check the next one.
        }
    }
    //no free slots were found.
    return INVALID_SERVO;
}//end registerServo


//=============================================================================
// FUNCTION:    bool deregisterServo(uint8_t servoSlotNumber)
//
// DESCRIPTION: Frees up a slot in the pulse sequence.
//
// INPUT:       servoSlotNumber - The slot number to deallocate.
//
// RETURNS:     Nothing
//=============================================================================
void ServoSequencer::deregisterServo(uint8_t servoSlotNumber)
{
    //make sure we got a valid slot number
    if(servoSlotNumber < MAX_SERVOS)
    {
        servoRegistry[servoSlotNumber].enabled      = false;
        servoRegistry[servoSlotNumber].slotOccupied = false;
    servoRegistry[servoSlotNumber].pulseLengthInTicks = 128; //restore the pulse length to the default setting.
    }
    else
    {
        //we got a slot number that is out of range. Do nothing.
    }
}//end deregisterServo





//=============================================================================
// FUNCTION:    void setServoPulseLength(uint8_t servoNumber, uint16_t newLengthInMicroseconds)
//
// DESCRIPTION: Updates a servo slots with a new pulse length.
//              Only works on allocated slots.
//
// INPUT:       servoNumber - the slot number to update
//              newLengthInMicroseconds - the new pulse length to set
//
// RETURNS:     Nothing
//=============================================================================
void ServoSequencer::setServoPulseLength(uint8_t servoNumber, uint16_t newLengthInMicroseconds)
{
    //make sure we got a valid slot number and the slot is registered to a servo
    if( (servoNumber < MAX_SERVOS      ) &&
        (servoRegistry[servoNumber].slotOccupied == true)   )
    {
        //Convert the servo pulse length into timer ticks.
        //Each timer tick is 8 microseconds.
        int16_t newLengthInClockTicks = newLengthInMicroseconds / 8;
        //subtract the pulse offset
        newLengthInClockTicks -= 64;

        //make sure the length of this pulse is within the acceptable range
        if( (newLengthInClockTicks > -1) && (newLengthInClockTicks < 256) )
        {
            servoRegistry[servoNumber].pulseLengthInTicks = static_cast<uint8_t>(newLengthInClockTicks);
            //Programming note: If pulseLengthInTicks is ever changed to be larger than 1 byte in size then
            //                 interrupts would need to be disabled when updating it to a new value.
        }
        else
        {
            //The new pulse length is too short or long than what we can generate
        }
    }
    else
    {
        //Servo number is out of range or is not allocate to a servo. Do nothing.
    }
}//end setServoPulseLength



//=============================================================================
// FUNCTION:    void getServoPulseLength(uint8_t servoNumber)
//
// DESCRIPTION: Gets the pulse length of a servo slot.
//              Only works on allocated slots.
//
// INPUT:       servoNumber - which slot to get the pulse length from
//
// RETURNS:     The pulse length in microseconds
//=============================================================================
uint16_t ServoSequencer::getServoPulseLength(uint8_t servoNumber)
{
    uint16_t pulseLength = 0;

    //make sure we got a valid slot number and the slot is registered to a servo
    if( (servoNumber < MAX_SERVOS      ) &&
        (servoRegistry[servoNumber].slotOccupied == true)   )
    {
        pulseLength = (servoRegistry[servoNumber].pulseLengthInTicks * 8) + 64;
    }
    else
    {
        //Servo number is out of range or is not allocate to a servo. Do nothing.
    }

    return pulseLength;
}//end getServoPulseLength


//=============================================================================
// FUNCTION:    void setServoPin(uint8_t servoNumber, uint8_t newPin)
//
// DESCRIPTION: Sets which pin on PortB should be pulsed for a servo slot.
//
// INPUT:       servoNumber - which servo slot to update
//              newPin - which pin on portB should be pulsed
//
// RETURNS:     Nothing
//
//=============================================================================
void ServoSequencer::setServoPin(uint8_t servoNumber, uint8_t newPin)
{
    //make sure we got a valid slot number and the slot is registered to a servo
    if( (servoNumber < MAX_SERVOS      ) &&
        (servoRegistry[servoNumber].slotOccupied == true)   )
    {

        servoRegistry[servoNumber].pin = newPin;

    }
    else
    {
        //Servo number is out of range or is not allocate to a servo. Do nothing.
    }
}//end setServoPin


//=============================================================================
// FUNCTION:    void enableDisableServo(uint8_t servoNumber, bool servoShouldBeEnabled)
//
// DESCRIPTION: Enabled or disables an allocated servo slot.
//              An enabled slot generates a pwm wave on its pin.
//              A disabled slot does not.
//
// INPUT:       servoNumber - which servo slot to update
//              servoShouldBeEnabled - true to enable the servo slot
//                                     false to disable the servo slot
//
// RETURNS:     Nothing
//
//=============================================================================
void ServoSequencer::enableDisableServo(uint8_t servoNumber, bool servoShouldBeEnabled)
{

    //make sure we got a valid slot number and the slot is registered to a servo
    if( (servoNumber < MAX_SERVOS      ) &&
        (servoRegistry[servoNumber].slotOccupied == true)   )
    {
        if(servoShouldBeEnabled == true)
        {

            //if this is the very first servo we are enabling then configure the servo timer
            if( timerIsSetup == false)
            {
                servoTimerSetup();
                timerIsSetup = true;
            }

            else
            {
                //The timer is already setup. Do nothing.
                //It needs to be setup only once. We do it when the first servo is enabled.
                //We setup the timer as late as possible. This allows this servo library
                //to be more compatible with various frameworks written for the attiny45/85,
                //which typically configure all the timers to their liking on start up.
                //Configuring our timer late allows us to overwrite these settings.
            }
            //enable the servo. Its pulse will now be output on its pin.
            servoRegistry[servoNumber].enabled = true;
        }
        else
        {
            //disable the servo. Its pulse will cease to be generated.
            servoRegistry[servoNumber].enabled = false;
            //TODO: set this servo pin low, if it is high.
            //      Actually, ideally the pulse should finish by itself
            //      forcing the pin low will generate a weird length pulse for the servo
            //      Need to add some sort of "disable pending" status
        }
    }
    else
    {
        //Servo number is out of range or is not allocate to a servo. Do nothing.
    }
}//end enableDisableServo



//=============================================================================
// FUNCTION:    bool isEnabled(uint8_t servoNumber)
//
// DESCRIPTION: Determines if a servo slot is enabled.
//
// INPUT:       servoNumber - which servo slot to check
//
// RETURNS:     true if the slot is enabled
//              false if it is disabled or not allocated to a servo.
//
//=============================================================================
bool ServoSequencer::isEnabled(uint8_t servoNumber)
{
    //make sure we got a valid slot number and the slot is registered to a servo
    if( (servoNumber < MAX_SERVOS      ) &&
        (servoRegistry[servoNumber].slotOccupied == true)   )
    {
        return servoRegistry[servoNumber].enabled;
    }
    else
    {
        //Servo number is out of range or is not allocate to a servo.
        //So therefore it is not enabled.
        return false;
    }
}//end isEnabled


//=============================================================================
// FUNCTION:    void servoTimerSetup()
//
// DESCRIPTION: Sets up the timer used by this servo driver
//              No servo pulses can be generated until this function is called.
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//
//=============================================================================
void ServoSequencer::servoTimerSetup()
{
    //set up the timer prescaler based on which timer was selected and our F_CPU clock
    setupTimerPrescaler();
    #ifdef __AVR_ATtinyX61__
    TCCR1A=0;
    #endif
    // Enable Output Compare Match Interrupt
    TIMSK |= (1 << OCIEnx);

    //reset the counter to 0
    TCNTn  = 0;
    //set the compare value to any number larger than 0
    OCRnx = 255;
    // Enable global interrupts
    sei();

    /*
    TCNT0 - The Timer/Counter
    OCR0A and OCR0B - Output Compare Registers
    TIFR0 - Timer Interrupt Flag Register
    TIMSK - Timer Interrupt Mask Register
    TCCR0B Timer/Counter Control Register B
    */

}//end servoTimerSetup


//=============================================================================
// FUNCTION:    void setupTimerPrescaler()
//
// DESCRIPTION: Helper function that sets up the timer prescaller based on what
//              timer is selected and the F_CPU frequence.
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//
//=============================================================================
void ServoSequencer::setupTimerPrescaler()
{
    #ifdef PLLTIMER1
        #error "PLL Timer source not compatible with Servo, turn it off."
    #endif
    #ifdef LOWPLLTIMER1
        #error "PLL Timer source not compatible with Servo, turn it off."
    #endif
    //reset the Timer Counter Control Register to its reset value
  #if defined(__AVR_ATtinyX5__)
    TCCR1 = 0;

    #if F_CPU == 8000000L
        //set counter1 prescaler to 64
        //our F_CPU is 8mhz so this makes each timer tick be 8 microseconds long
        //TCCR1 &= ~(1<< CS13); //clear
        //TCCR1 |=  (1<< CS12); //set
        //TCCR1 |=  (1<< CS11); //set
        //TCCR1 |=  (1<< CS10); //set
        TCCR1=7;
    #elif F_CPU == 16000000L //16MHz
        //TCCR1|=(1<<CS13)
        TCCR1=8;

    #elif F_CPU == 1000000L
        //set counter1 prescaler to 8
        //our F_CPU is 1mhz so this makes each timer tick be 8 microseconds long
        //TCCR1 &= ~(1<< CS13); //clear
        //TCCR1 |=  (1<< CS12); //set
        //TCCR1 &= ~(1<< CS11); //clear
        //TCCR1 &= ~(1<< CS10); //clear
        TCCR1=4;
    #else
        #error "Servo only supported at 1MHz, 8MHz and 16MHz on tiny25/45/85. "
    #endif
  #elif defined(__AVR_ATtinyX61__)
    TCCR1B = 0;

    #if F_CPU == 8000000L
        //set counter1 prescaler to 64
        //our F_CPU is 8mhz so this makes each timer tick be 8 microseconds long
        //TCCR1 &= ~(1<< CS13); //clear
        //TCCR1 |=  (1<< CS12); //set
        //TCCR1 |=  (1<< CS11); //set
        //TCCR1 |=  (1<< CS10); //set
        TCCR1B=7;
    #elif F_CPU == 16000000L //16MHz
        //TCCR1|=(1<<CS13)
        TCCR1B=8;

    #elif F_CPU == 1000000L
        //set counter1 prescaler to 8
        //our F_CPU is 1mhz so this makes each timer tick be 8 microseconds long
        //TCCR1 &= ~(1<< CS13); //clear
        //TCCR1 |=  (1<< CS12); //set
        //TCCR1 &= ~(1<< CS11); //clear
        //TCCR1 &= ~(1<< CS10); //clear
        TCCR1B=4;
    #else
        #error "Servo only supported at 1MHz, 8MHz and 16MHz on tiny261/461/861. "
    #endif

  #endif
}//end setupTimerPrescaler


//=============================================================================
// FUNCTION:    void initServoArray()
//
// DESCRIPTION: Sets default values to each element of the servoRegistry array
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//
//=============================================================================
void ServoSequencer::initServoArray()
{
    //init the Servo Registry array
    for(uint8_t i = 0; i < MAX_SERVOS; ++i)
    {
        servoRegistry[i].pulseLengthInTicks = 128;
        servoRegistry[i].pin = 0;
        servoRegistry[i].enabled = false;
        servoRegistry[i].slotOccupied = false;
    }

    servoArrayIsInited = true;
}//end initServoArray


//=============================================================================
// FUNCTION:    void timerCompareMatchISR()
//
// DESCRIPTION: Interrupt service routine for timer0 compare A match.
//              This is where the magic happens.
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//=============================================================================
void ServoSequencer::timerCompareMatchISR()
{
    switch (state)
    {
    case WAITING_TO_SET_PIN_HIGH:
        //go to the next servo in the registry
        ++servoIndex;
        //if we are the end of the registry, go to the beginning of it
        if(servoIndex == MAX_SERVOS)
        {
            servoIndex = 0;
        }
        else
        {
            //we are not at the end, leave the servo index as is
        }

        //if this servo is enabled set the pin high
        if( servoRegistry[servoIndex].enabled == true )
        {
          #if defined(__AVR_ATtinyX5__)
            PORTB |= (1 << servoRegistry[servoIndex].pin);
          #elif defined(__AVR_ATtinyX61__)
            uint8_t bit = digitalPinToBitMask(servoRegistry[servoIndex].pin);
            uint8_t port = digitalPinToPort(servoRegistry[servoIndex].pin);
            volatile uint8_t *out;
            out = portOutputRegister(port);
            *out|=bit;
          #else
            #error "Unsupported part - how did execution get here?"
          #endif
        }
        else
        {
            //This servo position is not enabled, don't manipulate the pin
        }

        //reset the counter to 0
        TCNTn  = 0;
        //set the compare value to 64 (512 us). This is the constant pulse offset.
        OCRnx = 64 - TRIM_DURATION; //trim off 4 ticks (32us), this is about the total combined time we spent inside this ISR;
        //update our state
        state = WAITING_FOR_512_MARK;
        break;


    case WAITING_FOR_512_MARK:
        //set the compare value to the additional amount of timer ticks the pulse should last
        OCRnx = servoRegistry[servoIndex].pulseLengthInTicks;
        //update our state
        state = WAITING_TO_SET_PIN_LOW;

        //reset the counter to 0
        TCNTn  = 0;

        //Did we just set OCRnx to zero?
        if(OCRnx == 0)
        {
           //Since we are setting OCRnx and TCNTn to 0 we are not going to get an interrupt
           //until the counter overflows and goes back to 0.
           //set the counter its highest value, to have it overflow right away.
           TCNTn = 0xFF;
           //This will cause this interrupt to fire again almost immediately (at the next timer tick)
        }
        else
        {
            //otherwise we need to clear the OCF0A flag because it is possible that the
            //counter value incremented and matched the output compare value while this
            //function was being executed
            TIFR = (1 << OCF0A);  // write logical 1 to the OCF0A flag to clear it
                                  // also have to write 0 to all other bits for this to work.
        }
        break;


    case WAITING_TO_SET_PIN_LOW:
        //if this servo is enabled set the pin low
        if( servoRegistry[servoIndex].enabled == true )
        {
          #if defined(__AVR_ATtinyX5__)
            PORTB &= ~(1 << servoRegistry[servoIndex].pin);
          #elif defined(__AVR_ATtinyX61__)
            uint8_t bit = digitalPinToBitMask(servoRegistry[servoIndex].pin);
            uint8_t port = digitalPinToPort(servoRegistry[servoIndex].pin);
            volatile uint8_t *out;
            out = portOutputRegister(port);
            *out&=~bit;
          #else
            #error "Unsupported part - how did execution get here?"
          #endif
        }
        else
        {
            //This servo position is not enabled, don't manipulate the pin
        }

        //check if the length of this pulse is 2048 microseconds or longer
        if( (64 + servoRegistry[servoIndex].pulseLengthInTicks) > 255 )
        {
            //This pulse length has passed the 2048 us mark, so we skip state WAITING_FOR_2048_MARK
            //update state
            state = WAITING_TO_SET_PIN_HIGH;
            //set the compare value to the amount of time (in timer ticks) we need to wait to reach
            //4096 microseconds mark
            //which is 512 minus the total pulse length. (resulting number will be between 0 and 255 inclusive)
            OCRnx = 512 - (64 + servoRegistry[servoIndex].pulseLengthInTicks);
        }
        else
        {
            //This pulse length has not reached the 2048 us mark, therefore we have to get to that mark first
            //update state
            state = WAITING_FOR_2048_MARK;
            //set OCRnx to the amount of time (in timer ticks) we have to wait to reach this mark
            //which is 255 minus the total pulse length
            OCRnx = 255 - (64 + servoRegistry[servoIndex].pulseLengthInTicks);
        }

        //reset the counter to 0
        TCNTn  = 0;

        break;

    case WAITING_FOR_2048_MARK:
        //update state
        state = WAITING_TO_SET_PIN_HIGH;
        //reset the counter to 0
        TCNTn  = 0;
        //set the compare value to the longest length of time, 255 ticks, or 2040 microseconds
        //This will take us to the ~4096 microsecond mark,
        //at which point the cycle starts again with the next servo slot.
        OCRnx = 255;
        break;
    }//end switch
}//end timerCompareMatchISR




//=============================================================================
// Non Member Functions
//=============================================================================


//=============================================================================
// FUNCTION:    Interrupt service routine for timer1 compare A match
//
// DESCRIPTION: AVR Libc provided function that is vectored into when the
//              timer0 compare A match interrupt fires.
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//=============================================================================
ISR(TIMER1_COMPA_vect)
{
    ServoSequencer::timerCompareMatchISR();
}//end ISR TIM0_COMPA_vect


































//=============================================================================
// Servo Class Functions
//=============================================================================

//=============================================================================
// FUNCTION:    constructor
//
// DESCRIPTION: Constructor
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//
//=============================================================================
Servo::Servo()
{
    servoIndex=INVALID_SERVO;
    min=MIN_PULSE_WIDTH;
    max=MAX_PULSE_WIDTH;
}//end constructor

//=============================================================================
// FUNCTION:    destructor
//
// DESCRIPTION: destructor
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//
//=============================================================================
Servo::~Servo()
{
  detach();
}//end destructor

//=============================================================================
// FUNCTION:    void attach(uint8_t pin)
//
// DESCRIPTION: Attaches a servo motor to an i/o pin on Port B.
//
// INPUT:       pin - which pin on portB to attach to
//
// RETURNS:     The servo number of this servo.
//
//=============================================================================
uint8_t Servo::attach(uint8_t pin)
{

  //Do we need to register with the servo sequencer?
  delay(1000);
    if(servoIndex == INVALID_SERVO)
  {
    //Yep, we do, so register and save our servo number.
    servoIndex = ServoSequencer::registerServo();
    if(servoIndex == INVALID_SERVO)
    {
      //We got an invalid servo number. That means the servo sequencer is full and can't handle any more servos.
      return INVALID_SERVO;
    }
  }

    //valid pin values are between 0 and 5, inclusive.
  #if defined(__AVR_ATtinyX5__)
    if( pin <= 5 )
    {
        DDRB |= (1<<pin); //set pin as output
        //set the servo pin
        ServoSequencer::setServoPin(servoIndex, pin);
        //enable the servo to start outputting the PWM wave
        ServoSequencer::enableDisableServo(servoIndex, true);
    }

  #elif defined(__AVR_ATtinyX61__)
    if (pin <= 16) {
      uint8_t bit = digitalPinToBitMask(pin);
      uint8_t port = digitalPinToPort(pin);
      volatile uint8_t *reg;
      if (port == NOT_A_PIN) return INVALID_SERVO;
      reg = portModeRegister(port);
      *reg |= bit;
      ServoSequencer::setServoPin(servoIndex, pin);
      //enable the servo to start outputting the PWM wave
      ServoSequencer::enableDisableServo(servoIndex, true);
    }
  #else
    #error "This part isn't supported - how did execution get here?"
  #endif
    else
    {
        //bad pin value. do nothing.
    }

    return servoIndex;

}//end attach





//=============================================================================
// FUNCTION:    uint8_t attach(uint8_t pin, uint16_t min, uint16_t max)
//
// DESCRIPTION: Attaches a servo motor to an i/o pin on Port B and also sets
//              the minimum and maximum pulse length values for this servo.
//
// INPUT:       pin - which pin on portB to attach to
//              min - minimum pulse length to use
//              max - maximum pulse length to use
//
// RETURNS:
//
//=============================================================================
uint8_t Servo::attach(uint8_t pin, uint16_t newMin, uint16_t newMax)
{
    min = newMin;
    max = newMax;
    return attach(pin);
}//end attach with min/max


//=============================================================================
// FUNCTION:    void detach()
//
// DESCRIPTION: Stops an attached servos from pulsing its i/o pin.
//
// INPUT:       Nothing
//
// RETURNS:     Nothing
//
//=============================================================================
void Servo::detach()
{
    ServoSequencer::deregisterServo(servoIndex);
    servoIndex = INVALID_SERVO;
}


//=============================================================================
// FUNCTION:    void write(uint16_t value)
//
// DESCRIPTION: Sets the servo angle in degrees.
//              invalid angle that is valid as pulse in microseconds is
//              treated as microseconds.
//
// INPUT:       value - position for the servo to move to
//
// RETURNS:     Nothing
//
//=============================================================================
void Servo::write(uint16_t value)
{
    //make sure we have a valid servo number. If it's invalid then exit doing nothing.
    if(servoIndex == INVALID_SERVO) return;

    //for now, only accept angles, and angles that are between 0 and 200 degrees
    if( value > 180 )
    {
        //treat this number as microseconds
        writeMicroseconds( value );
    }
    else
    {
        //treat this number as degrees
        uint16_t servoPulseLengthInUs = map(value, 0, 180, min, max);
        writeMicroseconds( servoPulseLengthInUs );
    }
}//end write


//=============================================================================
// FUNCTION:    void writeMicroseconds(uint16_t value)
//
// DESCRIPTION: Sets the servo pulse width in microseconds
//
// INPUT:       value - the pulse width of the servo pulse in microseconds
//
// RETURNS:     Nothing
//
//=============================================================================
void Servo::writeMicroseconds(uint16_t value)
{
    //make sure we have a valid servo number. If it's invalid then exit doing nothing.
    if(servoIndex == INVALID_SERVO) return;

    ServoSequencer::setServoPulseLength(servoIndex, value );
}//end writeMicroseconds





//=============================================================================
// FUNCTION:    uint16_t readMicroseconds()
//
// DESCRIPTION: Gets the last written servo pulse width in microseconds.
//
// INPUT:       Nothing
//
// RETURNS:     The pulse width in microseconds
//
//=============================================================================
uint16_t Servo::readMicroseconds()
{
    //make sure we have a valid servo number. If it's invalid then exit doing nothing.
    if(servoIndex == INVALID_SERVO) return 0;

    return ServoSequencer::getServoPulseLength(servoIndex);
}//end readMicroseconds


//=============================================================================
// FUNCTION:    uint16_t read()
//
// DESCRIPTION: Gets the last written servo pulse width as an angle between 0 and 180.
//
// INPUT:       Nothing
//
// RETURNS:     Angle between 0 and 180
//
//=============================================================================
uint16_t Servo::read()
{
    //make sure we have a valid servo number. If it's invalid then exit doing nothing.
    if(servoIndex == INVALID_SERVO) return 0;

    uint16_t servoPulseLengthInUs = readMicroseconds();
    uint16_t servoPositionInDegrees = map(servoPulseLengthInUs, min, max, 0, 180);
    return servoPositionInDegrees;
}//end read


//=============================================================================
// FUNCTION:    bool attached()
//
// DESCRIPTION: Returns true if there is a servo attached.
//
// INPUT:       Nothing
//
// RETURNS:     true, if attached
//              false, otherwise
//
//=============================================================================
bool Servo::attached()
{
    //make sure we have a valid servo number. If it's invalid then exit doing nothing.
    if(servoIndex == INVALID_SERVO) return false;

    return ServoSequencer::isEnabled(servoIndex);
}//end attached



#else //end of 8-bit servo code


#include "Servo.h"

#define usToTicks(_us)    (( clockCyclesPerMicrosecond()* _us) / 8)     // converts microseconds to tick (assumes prescale of 8)  // 12 Aug 2009
#define ticksToUs(_ticks) (( (unsigned)_ticks * 8)/ clockCyclesPerMicrosecond() ) // converts from ticks back to microseconds


#define TRIM_DURATION       2                               // compensation ticks to trim adjust for digitalWrite delays // 12 August 2009

//#define NBR_TIMERS        (MAX_SERVOS / SERVOS_PER_TIMER)

static servo_t servos[MAX_SERVOS];                          // static array of servo structures
static volatile int8_t Channel[_Nbr_16timers ];             // counter for the servo being pulsed for each timer (or -1 if refresh interval)

uint8_t ServoCount = 0;                                     // the total number of attached servos


// convenience macros
#define SERVO_INDEX_TO_TIMER(_servo_nbr) ((timer16_Sequence_t)(_servo_nbr / SERVOS_PER_TIMER)) // returns the timer controlling this servo
#define SERVO_INDEX_TO_CHANNEL(_servo_nbr) (_servo_nbr % SERVOS_PER_TIMER)       // returns the index of the servo on this timer
#define SERVO_INDEX(_timer,_channel)  ((_timer*SERVOS_PER_TIMER) + _channel)     // macro to access servo index by timer and channel
#define SERVO(_timer,_channel)  (servos[SERVO_INDEX(_timer,_channel)])            // macro to access servo class by timer and channel

#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in uS for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in uS for this servo

/************ static functions common to all instances ***********************/

static inline void handle_interrupts(timer16_Sequence_t timer, volatile uint16_t *TCNTn, volatile uint16_t* OCRnA)
{
  if( Channel[timer] < 0 )
    *TCNTn = 0; // channel set to -1 indicated that refresh interval completed so reset the timer
  else{
    if( SERVO_INDEX(timer,Channel[timer]) < ServoCount && SERVO(timer,Channel[timer]).Pin.isActive == true )
      digitalWrite( SERVO(timer,Channel[timer]).Pin.nbr,LOW); // pulse this channel low if activated
  }

  Channel[timer]++;    // increment to the next channel
  if( SERVO_INDEX(timer,Channel[timer]) < ServoCount && Channel[timer] < SERVOS_PER_TIMER) {
    *OCRnA = *TCNTn + SERVO(timer,Channel[timer]).ticks;
    if(SERVO(timer,Channel[timer]).Pin.isActive == true)     // check if activated
      digitalWrite( SERVO(timer,Channel[timer]).Pin.nbr,HIGH); // its an active channel so pulse it high
  }
  else {
    // finished all channels so wait for the refresh period to expire before starting over
    if( ((unsigned)*TCNTn) + 4 < usToTicks(REFRESH_INTERVAL) )  // allow a few ticks to ensure the next OCR1A not missed
      *OCRnA = (unsigned int)usToTicks(REFRESH_INTERVAL);
    else
      *OCRnA = *TCNTn + 4;  // at least REFRESH_INTERVAL has elapsed
    Channel[timer] = -1; // this will get incremented at the end of the refresh period to start again at the first channel
  }
}

#ifndef WIRING // Wiring pre-defines signal handlers so don't define any if compiling for the Wiring platform
// Interrupt handlers for Arduino
#if defined(_useTimer1)
SIGNAL (TIMER1_COMPA_vect)
{
  handle_interrupts(_timer1, &TCNT1, &OCR1A);
}
#endif

#if defined(_useTimer3)
SIGNAL (TIMER3_COMPA_vect)
{
  handle_interrupts(_timer3, &TCNT3, &OCR3A);
}
#endif

#if defined(_useTimer4)
SIGNAL (TIMER4_COMPA_vect)
{
  handle_interrupts(_timer4, &TCNT4, &OCR4A);
}
#endif

#if defined(_useTimer5)
SIGNAL (TIMER5_COMPA_vect)
{
  handle_interrupts(_timer5, &TCNT5, &OCR5A);
}
#endif

#elif defined WIRING
// Interrupt handlers for Wiring
#if defined(_useTimer1)
void Timer1Service()
{
  handle_interrupts(_timer1, &TCNT1, &OCR1A);
}
#endif
#if defined(_useTimer3)
void Timer3Service()
{
  handle_interrupts(_timer3, &TCNT3, &OCR3A);
}
#endif
#endif


static void initISR(timer16_Sequence_t timer)
{
#if defined (_useTimer1)
  if(timer == _timer1) {
    TCCR1A = 0;             // normal counting mode
    TCCR1B = _BV(CS11);     // set prescaler of 8
    TCNT1 = 0;              // clear the timer count
#if defined(__AVR_ATmega8__)|| defined(__AVR_ATmega128__) || defined(__AVR_ATtiny1634__)
    TIFR |= _BV(OCF1A);      // clear any pending interrupts;
    TIMSK |=  _BV(OCIE1A) ;  // enable the output compare interrupt
#else
    // here if not ATmega8 or ATmega128
    TIFR1 |= _BV(OCF1A);     // clear any pending interrupts;
    TIMSK1 |=  _BV(OCIE1A) ; // enable the output compare interrupt
#endif
#if defined(WIRING)
    timerAttach(TIMER1OUTCOMPAREA_INT, Timer1Service);
#endif
  }
#endif

#if defined (_useTimer3)
  if(timer == _timer3) {
    TCCR3A = 0;             // normal counting mode
    TCCR3B = _BV(CS31);     // set prescaler of 8
    TCNT3 = 0;              // clear the timer count
#if defined(__AVR_ATmega128__) || defined(__AVR_ATtiny1634__)
    TIFR |= _BV(OCF3A);     // clear any pending interrupts;
    ETIMSK |= _BV(OCIE3A);  // enable the output compare interrupt
#else
    TIFR3 = _BV(OCF3A);     // clear any pending interrupts;
    TIMSK3 =  _BV(OCIE3A) ; // enable the output compare interrupt
#endif
#if defined(WIRING)
    timerAttach(TIMER3OUTCOMPAREA_INT, Timer3Service);  // for Wiring platform only
#endif
  }
#endif

#if defined (_useTimer4)
  if(timer == _timer4) {
    TCCR4A = 0;             // normal counting mode
    TCCR4B = _BV(CS41);     // set prescaler of 8
    TCNT4 = 0;              // clear the timer count
    TIFR4 = _BV(OCF4A);     // clear any pending interrupts;
    TIMSK4 =  _BV(OCIE4A) ; // enable the output compare interrupt
  }
#endif

#if defined (_useTimer5)
  if(timer == _timer5) {
    TCCR5A = 0;             // normal counting mode
    TCCR5B = _BV(CS51);     // set prescaler of 8
    TCNT5 = 0;              // clear the timer count
    TIFR5 = _BV(OCF5A);     // clear any pending interrupts;
    TIMSK5 =  _BV(OCIE5A) ; // enable the output compare interrupt
  }
#endif
}

static void finISR(timer16_Sequence_t timer)
{
    //disable use of the given timer
#if defined WIRING   // Wiring
  if(timer == _timer1) {
    #if defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
    TIMSK1 &=  ~_BV(OCIE1A) ;  // disable timer 1 output compare interrupt
    #else
    TIMSK &=  ~_BV(OCIE1A) ;  // disable timer 1 output compare interrupt
    #endif
    timerDetach(TIMER1OUTCOMPAREA_INT);
  }
  else if(timer == _timer3) {
    #if defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
    TIMSK3 &= ~_BV(OCIE3A);    // disable the timer3 output compare A interrupt
    #else
    ETIMSK &= ~_BV(OCIE3A);    // disable the timer3 output compare A interrupt
    #endif
    timerDetach(TIMER3OUTCOMPAREA_INT);
  }
#else
    //For arduino - in future: call here to a currently undefined function to reset the timer
#endif
}

static boolean isTimerActive(timer16_Sequence_t timer)
{
  // returns true if any servo is active on this timer
  for(uint8_t channel=0; channel < SERVOS_PER_TIMER; channel++) {
    if(SERVO(timer,channel).Pin.isActive == true)
      return true;
  }
  return false;
}


/****************** end of static functions ******************************/

Servo::Servo()
{
  if( ServoCount < MAX_SERVOS) {
    this->servoIndex = ServoCount++;                    // assign a servo index to this instance
    servos[this->servoIndex].ticks = usToTicks(DEFAULT_PULSE_WIDTH);   // store default values  - 12 Aug 2009
  }
  else
    this->servoIndex = INVALID_SERVO ;  // too many servos
}

uint8_t Servo::attach(int pin)
{
  return this->attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int min, int max)
{
  if(this->servoIndex < MAX_SERVOS ) {
    pinMode( pin, OUTPUT) ;                                   // set servo pin to output
    servos[this->servoIndex].Pin.nbr = pin;
    // todo min/max check: abs(min - MIN_PULSE_WIDTH) /4 < 128
    this->min  = (MIN_PULSE_WIDTH - min)/4; //resolution of min/max is 4 uS
    this->max  = (MAX_PULSE_WIDTH - max)/4;
    // initialize the timer if it has not already been initialized
    timer16_Sequence_t timer = SERVO_INDEX_TO_TIMER(servoIndex);
    if(isTimerActive(timer) == false)
      initISR(timer);
    servos[this->servoIndex].Pin.isActive = true;  // this must be set after the check for isTimerActive
  }
  return this->servoIndex ;
}

void Servo::detach()
{
  servos[this->servoIndex].Pin.isActive = false;
  timer16_Sequence_t timer = SERVO_INDEX_TO_TIMER(servoIndex);
  if(isTimerActive(timer) == false) {
    finISR(timer);
  }
}

void Servo::write(int value)
{
  if(value < MIN_PULSE_WIDTH)
  {  // treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
    if(value < 0) value = 0;
    if(value > 180) value = 180;
    value = map(value, 0, 180, SERVO_MIN(),  SERVO_MAX());
  }
  this->writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value)
{
  // calculate and store the values for the given channel
  byte channel = this->servoIndex;
  if( (channel < MAX_SERVOS) )   // ensure channel is valid
  {
    if( value < SERVO_MIN() )          // ensure pulse width is valid
      value = SERVO_MIN();
    else if( value > SERVO_MAX() )
      value = SERVO_MAX();

    value = value - TRIM_DURATION;
    value = usToTicks(value);  // convert to ticks after compensating for interrupt overhead - 12 Aug 2009

    uint8_t oldSREG = SREG;
    cli();
    servos[channel].ticks = value;
    SREG = oldSREG;
  }
}

int Servo::read() // return the value as degrees
{
  return  map( this->readMicroseconds()+1, SERVO_MIN(), SERVO_MAX(), 0, 180);
}

int Servo::readMicroseconds()
{
  unsigned int pulsewidth;
  if( this->servoIndex != INVALID_SERVO )
    pulsewidth = ticksToUs(servos[this->servoIndex].ticks)  + TRIM_DURATION ;   // 12 aug 2009
  else
    pulsewidth  = 0;

  return pulsewidth;
}

bool Servo::attached()
{
  return servos[this->servoIndex].Pin.isActive ;
}
#endif // 8bit servo test
#endif // ARDUINO_ARCH_AVR
