/*
This sketch illustrates 2 new features of the <RcSeq> library (since the V2.1 version):
1) the "control" capability: it's a function passed as argument to the RcSeq_DeclareCommandAndSequence() method.
   It is used to check if a sequence can be launched or not, depending of specific condition
   It is also used to inform the sequence is finished: this can be used to memorize in EEPROM the sequence id.
   Like that, at the next start-up the position of the servos can be restored according to the last position of the sequence.
2) the "timeout" capability:
   The RcSeq_Timeout() method can be used to check if the command signal remains constant (HIGH or LOW).
   It's then possible to launch the sequence based on the static state of the command pin rather than a Rc Pulse width.
   In practice, it's possible to use both manners to launch a sequence as done in the sketch below.

THE SKETCH:
==========
In this sketch, the first declared sequence opens the 2 doors with the help of 2 servos (1 per door).
The second declared sequence closes the 2 doors with the help of 2 servos (1 per door).
The 2 doors cannot open or close simultaneously with the same speed since there is a nosing secured to the right door.
This nosing forces to open and close the doors using sequences.

  Opening <-                        -> Opening
              .                  .
                .              .
                  .          .
                   .        .
                    .      .
                     .    .
                     .    .
 __       nosing -> .------.                 __
/  \----------------'---.  '----------------/  \
\__/-------------------''-------------------\__/
     Left door                 Right door
                   TOP VIEW

The opening sequence is like hereafter:
======================================
1) The servo assigned to the right door starts
2) Once rigth door slightly opened, the servo assigned to the left door starts, whilst the servo assigned to the right door resumes its travel
3) Once the 2 servos reached 90°, the 2 doors stop; the opening sequence is finished

The closing sequence is like hereafter:
======================================
1) The 2 servos assigned to the left and right doors start together but the left servo rotates more quickly than the right servo.
2) As a consequence, the left door is closed berfore the right door
3) Once the 2 servos reached 90°, the 2 doors stop; the closing sequence is finished

The sequences of this sketch can be launched either a RC channel either a regular ON/OFF switch:
===============================================================================================
A) Command from a RC channel:
   -------------------------                   _______________
   V                  _______________         |  __           |
   |  __________     |   ARDUINO:    |________| /  \  Left    |
   | |          |    |               |        | \__/  Servo   |
   | |    RC    |CH  |     UNO       |        |_______________|
   '-+ Receiver |----|     MEGA      |         _______________
     |          |    |   Digispark   |        |  __           |
     |__________|    | Digispark pro |________| /  \  Right   |
                     |_______________|        | \__/  Servo   |
                                              |_______________|
                      
B) Command from a ON/OFF switch:
   ----------------------------                _______________
                      _______________         |  __           |
                     |   ARDUINO:    |________| /  \  Left    |
                     |               |        | \__/  Servo   |
                     |     UNO       |        |_______________|
                .----|     MEGA      |         _______________
                |    |   Digispark   |        |  __           |
  ON/OFF Switch \    | Digispark pro |________| /  \  Right   |
                |    |_______________|        | \__/  Servo   |
               -+-                            |_______________|
               GND               
*/

/*************************************************/
/* STEP #1: Include the required libraries       */
/*************************************************/
#include <RcSeq.h>
#include <TinyPinChange.h>
#include <SoftRcPulseIn.h>
#include <SoftRcPulseOut.h>
#include <EEPROM.h>

/*****************************************************************/
/* STEP #2: Enumeration of the RC Signals used in the sequence   */
/*****************************************************************/
enum {RC_SIGNAL = 0, SIGNAL_NB}; /* Here, a single RC signal is used */

/******************************************************************/
/* STEP #3: Enumeration of the different position of the RC stick */
/******************************************************************/
enum {RC_PULSE_LEVEL_MINUS_1 = 0, RC_PULSE_LEVEL_PLUS_1, RC_PULSE_NB};

/*****************************************************************/
/* STEP #4: Enumeration of the servos used in the sequences      */
/*****************************************************************/
enum {DOOR_SERVO_LEFT = 0, DOOR_SERVO_RIGHT, SERVO_NB}; /* In this sketch, 2 servos are declared */

/***************************************************/
/* STEP #5: Digital pin assignment for Command     */
/***************************************************/
#define COMMAND_PIN                        2 /* This pin can be connected to a channel of a RC Receiver or to a regular ON/OFF switch (switch wired between pin and Ground) */

/**************************************************/
/* STEP #6: Digital Pins assignment for Servos    */
/**************************************************/
#define DOOR_SERVO_LEFT_PIN                3
#define DOOR_SERVO_RIGHT_PIN               4

/*************************************************************************************/
/* STEP #7: Declaration of the angle of the servos for the different motions (in °)  */
/*************************************************************************************/
#define DOOR_SERVO_OPENED_LEFT_POS         135 /* position of the left Servo when left door is opened */
#define DOOR_SERVO_CLOSED_LEFT_POS         45  /* position of the left Servo when left door is closed */

#define DOOR_SERVO_OPENED_RIGHT_POS        45  /* position of the right Servo when right door is opened */
#define DOOR_SERVO_CLOSED_RIGHT_POS        135 /* position of the right Servo when right door is closed */


/***************************************************************************************************************************************/
/* STEP #8: Do a temporal diagram showing the start up and the duration of each motions of each servo                                  */
/***************************************************************************************************************************************/
/*
1) OPENING MOTION OF THE DOORS
   ===========================
All the start up values (time stamp) have as reference the moment of the sequence startup order (t=0).

1.1 MOTION OF THE LEFT DOOR SERVO FOR OPENING
    =========================================
    
Order                            <---OPENING_DURATION_LEFT_MS---> 
  |-----------------------------|--------------------------------|-->Time Axis
  0                  OPENING_START_LEFT_MS
  
1.2 MOTION OF THE RIGHT DOOR SERVO FOR OPENING
    ==========================================
    
Order                  <--------OPENING_DURATION_RIGHT_MS-------> 
  |-------------------|------------------------------------------|-->Time Axis
  0        OPENING_START_RIGHT_MS


2) CLOSING MOTION OF THE DOORS
   ===========================
All the start up values (time stamp) have as reference the moment of the sequence startup order (t=0).

2.1 MOTION OF THE LEFT DOOR SERVO FOR CLOSING
    =========================================

Order                  <---CLOSING_DURATION_LEFT_MS---> 
  |-------------------|--------------------------------|------------>Time Axis
  0         CLOSING_START_LEFT_MS
  
2.2 MOTION OF THE RIGTH DOOR SERVO FOR CLOSING
    ==========================================

Order                  <--------CLOSING_DURATION_RIGHT_MS-------> 
  |-------------------|------------------------------------------|-->Time Axis
  0        CLOSING_START_RIGHT_MS    
*/

/**************************************************************************************************************************************************/
/* STEP #9: With the help of the temporal diagram, declare start up time, the motion duration of servo and optional delay                         */
/**************************************************************************************************************************************************/
/* Tune below all the motion duration. Do not forget to add a trailer 'UL' for each value to force them in Unsigned Long type */
#define OPENING_START_LEFT_MS          500UL  //This means the left servo motion will be delayed of 500ms AFTER the order
#define OPENING_DURATION_LEFT_MS       2500UL //The left door motion ends after 500+2500=3s, as the right door

#define OPENING_START_RIGHT_MS         0UL    //Immediate start
#define OPENING_DURATION_RIGHT_MS      3000UL //The right door motion ends after 3s

#define CLOSING_START_LEFT_MS          0UL    //Immediate start
#define CLOSING_DURATION_LEFT_MS       3000UL //The left door will be closed BEFORE the right door

#define CLOSING_START_RIGHT_MS         0UL    //Immediate start
#define CLOSING_DURATION_RIGHT_MS      4000UL //The right door will be closed AFTER the left door

/********************************************************************************************************************/
/* STEP #10: Declare here the percentage of motion to be performed at half speed for servo start up and stop        */
/********************************************************************************************************************/
#define START_STOP_PER_CENT                 5 /* Percentage of motion performed at half-speed for starting and stopping the servos (Soft start et Soft stop) */

/************************************************************************************************************/
/* STEP #11: Use a "const SequenceSt_t" structure table to declare the servo sequence                       */
/* For each table entry, arguments are:                                                                     */
/* - Servo Index                                                                                            */
/* - Initial Servo Position in °                                                                            */
/* - Final   Servo Position in °                                                                            */
/* - Motion Start Time Stamp in ms                                                                          */
/* - Motion duration in ms between initial and final position                                               */
/* - Percentage of motion performed at half speed for servo start and servo stop (Soft start and Soft stop) */
/************************************************************************************************************/
/* Table describing the motions of the 2 servos for opening the 2 doors */
const SequenceSt_t OpeningSequence[] PROGMEM = {/*     Servo Id ,  Initial Angle             , Final Angle                , Delay after order     , Motion Duration          , Percentage at half speed */
/* 1st Servo */ MOTION_WITH_SOFT_START_AND_STOP(DOOR_SERVO_LEFT,  DOOR_SERVO_CLOSED_LEFT_POS,  DOOR_SERVO_OPENED_LEFT_POS,  OPENING_START_LEFT_MS,  OPENING_DURATION_LEFT_MS,  START_STOP_PER_CENT)
/* 2nd Servo */ MOTION_WITH_SOFT_START_AND_STOP(DOOR_SERVO_RIGHT, DOOR_SERVO_CLOSED_RIGHT_POS, DOOR_SERVO_OPENED_RIGHT_POS, OPENING_START_RIGHT_MS, OPENING_DURATION_RIGHT_MS, START_STOP_PER_CENT)
                };

/* Table describing the motions of the 2 servos for closing the 2 doors */
const SequenceSt_t ClosingSequence[] PROGMEM = {/*    Servo Id  ,  Initial Angle             , Final Angle                , Delai after order     , Motion Duration          , Percentage at half speed */
/* 1st Servo */ MOTION_WITH_SOFT_START_AND_STOP(DOOR_SERVO_LEFT,  DOOR_SERVO_OPENED_LEFT_POS,  DOOR_SERVO_CLOSED_LEFT_POS,  CLOSING_START_LEFT_MS,  CLOSING_DURATION_LEFT_MS,  START_STOP_PER_CENT)
/* 2nd Servo */ MOTION_WITH_SOFT_START_AND_STOP(DOOR_SERVO_RIGHT, DOOR_SERVO_OPENED_RIGHT_POS, DOOR_SERVO_CLOSED_RIGHT_POS, CLOSING_START_RIGHT_MS, CLOSING_DURATION_RIGHT_MS, START_STOP_PER_CENT)
                };

enum {COMMAND_OPEN = 0, COMMAND_CLOSE};

/* GLOBAL VARIABLES */
uint8_t LastExecutedSeqIdx;

void setup()
{
  
  #if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny167__)
  Serial.begin(9600);
  Serial.print(F("RcSeq library V"));Serial.print(RcSeq_LibTextVersionRevision());Serial.println(F(" demo: advanced doors sequences"));
  #endif

  /***************************************************************************/
  /* STEP #12: Init <RcSeq> library                                          */
  /***************************************************************************/
  RcSeq_Init();
   
  /****************************************************************************************/
  /* STEP #13: declare the servo command signals with their digital pin number            */
  /****************************************************************************************/
  RcSeq_DeclareSignal(RC_SIGNAL, COMMAND_PIN);

  /******************************************************************************************/
  /* STEP #14: declare a stick assigned to the RC signal having RC_PULSE_NB positions       */
  /******************************************************************************************/
  RcSeq_DeclareStick(RC_SIGNAL, 1000, 2000, RC_PULSE_NB);
    
  /****************************************************************************************/
  /* STEP #15: declare the servo command signals with their digital pin number            */
  /****************************************************************************************/
  RcSeq_DeclareServo(DOOR_SERVO_LEFT,  DOOR_SERVO_LEFT_PIN);
  RcSeq_DeclareServo(DOOR_SERVO_RIGHT, DOOR_SERVO_RIGHT_PIN);
    
  /*******************************************************************************************************/
  /* STEP #16: declare the sequence assigned to specific position of the stick assigned to the RC signal */
  /*******************************************************************************************************/
  RcSeq_DeclareCommandAndSequence(RC_SIGNAL, RC_PULSE_LEVEL_MINUS_1, RC_SEQUENCE(OpeningSequence), Control); // Declare a sequence triggered by a RC pulse Level Minus 1 (stick at extreme position during at least 250 ms)
  RcSeq_DeclareCommandAndSequence(RC_SIGNAL, RC_PULSE_LEVEL_PLUS_1,  RC_SEQUENCE(ClosingSequence), Control); // Declare a sequence triggered by a RC pulse Level  Plus 1 (stick at extreme position during at least 250 ms)

  /*******************************************************************************************/
  /* STEP #17: Initialize the position of the servos according to the last finished sequence */
  /*******************************************************************************************/
  LastExecutedSeqIdx = EEPROM.read(0);
  if (LastExecutedSeqIdx == COMMAND_OPEN)
  {
    RcSeq_ServoWrite(DOOR_SERVO_LEFT,  DOOR_SERVO_OPENED_LEFT_POS);
    RcSeq_ServoWrite(DOOR_SERVO_RIGHT, DOOR_SERVO_OPENED_RIGHT_POS);
  }
  else
  {
    RcSeq_ServoWrite(DOOR_SERVO_LEFT,  DOOR_SERVO_CLOSED_LEFT_POS);
    RcSeq_ServoWrite(DOOR_SERVO_RIGHT, DOOR_SERVO_CLOSED_RIGHT_POS);
  }
}

void loop()
{
  uint8_t RcSignalPinState;  
  /****************************************************************************************************************/
  /* STEP #18: call the refresh function inside the loop() to catch RC commands and to manage the servo positions */
  /****************************************************************************************************************/
  RcSeq_Refresh();

  /*********************************************************************************************************/
  /* STEP #19: optionally, allow launching the Sequences ou Actions on Timeout (cmd with a regular switch) */
  /*********************************************************************************************************/
  if (RcSeq_SignalTimeout(RC_SIGNAL, 250, &RcSignalPinState))
  {
    /* Launch the"OpeningSequence" sequence  if a LOW level is present during at least 250ms: this allows testing the sequence of servo without using a RC set, just using a regular switch */
    if((LastExecutedSeqIdx == COMMAND_CLOSE) && (RcSignalPinState == LOW))
    {
        RcSeq_LaunchSequence(OpeningSequence);
    }
    /* Launch the"ClosingSequence" sequence  if a HIGH level is present during at least 250ms: this allows testing the sequence of servo without using a RC set, just using a regular switch */
    if((LastExecutedSeqIdx == COMMAND_OPEN) && (RcSignalPinState == HIGH))
    {
        RcSeq_LaunchSequence(ClosingSequence);
    }
  }
}

/* The Control() fonction is automatically called by the RcSeq library */
uint8_t Control(uint8_t Action, uint8_t SeqIdx)
{
  uint8_t Ret = 0;
  #if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny167__)
  Serial.print("Action=");Serial.print(Action);Serial.print(" SeqIdx=");Serial.println(SeqIdx);
  #endif
  switch(Action)
  {
    case RC_SEQ_START_CONDITION: /* RcSeq asks if the conditions are met to launch the sequence SeqIdx  */
    /* Put here a condition to allow RcSeq launching the sequence SeqIdx (Put Ret=1 if no specific condition) */
    Ret = (SeqIdx != LastExecutedSeqIdx); /* Allows RcSeq launching the sequence if the sequence to launch is different from the last one */
    break;
    
    case RC_SEQ_END_OF_SEQ: /* RcSeq informs the sequence SeqIdx is finished */
    /* We memorize the last finished sequence id in EEPROM memory. Like that, at next power-up, we will know how to position the servos ( done in the Setup() ) */  
    EEPROM.write(0, SeqIdx);
    LastExecutedSeqIdx = SeqIdx;
    break;
  }
  return(Ret);
}

