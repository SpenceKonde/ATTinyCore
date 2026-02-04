/*
Sketch using <RcSeq> library, for automatically dropping a pneumatic Zodiac at sea and returning for it back to the deck of a supply vessel.
The sequence is launched after sending the 'g' (Go) character at the USB interface.

In this example, the declared sequence is:
1) The crane lifts the pneumatic Zodiac from the deck to the air and stops
2) The crane rotates (90°) to locate the pneumatic Zodiac above the sea
3) The crane drops down the pneumatic Zodiac at sea level
4) The crane stops during 6 seconds
5) The crane lifts up the pneumatic Zodiac from sea level to the air and stops
6) The crane rotates (90°) to locate the pneumatic Zodiac above the deck
7) The crane drops down the pneumatic Zodiac on the deck and stops. The sequence ends.
This sequence uses:
- 2 commands from USB interface ('g' and 't' characters from Digiterm or Digi Monitor)
- 2 servos (a "ROTATION" servo for the crane rotation and an "UP/DOWN" servo to drop and lift the pneumatic Zodiac)

IMPORTANT:
=========
For this sketch, which is using <DigiUSB> library:
1) Comment "#define RC_SEQ_WITH_SOFT_RC_PULSE_IN_SUPPORT" AND #define RC_SEQ_CONTROL_SUPPORT in "arduino-1.xx\libraries\RcSeq.h".
   This will disable the code to manage incoming RC pulses and save some flash memory.
   RC_SEQ_WITH_SHORT_ACTION_SUPPORT and RC_SEQ_WITH_SOFT_RC_PULSE_OUT_SUPPORT shall be defined
2) Replace #define RING_BUFFER_SIZE 128 with #define RING_BUFFER_SIZE 32 in "arduino-1.xx\libraries\DigisparkUSB\DigiUSB.h".
3) The sequence will be launch by sending "g" character through USB link (using Digiterm or Digi Monitor).
   To check all the sequence is performed asynchronously, you can send 't' to toggle the LED during servo motion!
If step 1) and 2) are not done, this sketch won't compile because won't fit in programm memory of the DigiSpark!

RC Navy 2013
http://p.loussouarn.free.fr
*/

static void ToggleLed(void); /* Declare Short Action: Toggle a LED */

/*************************************************/
/* STEP #1: Include the needed libraries         */
/*************************************************/
#include <DigiUSB.h> /* The Servo Sequence will be launched by sending "g" character (Go) at the USB interface */
#include <RcSeq.h>
#include <SoftRcPulseOut.h>

#define LED_PIN                     1

/*****************************************************************/
/* STEP #2: Enumeration of the servos used in the sequence       */
/*****************************************************************/
enum {ROTATION_SERVO=0, UP_DOWN_SERVO, SERVO_NB};

/*****************************************************************/
/* STEP #3: Servos Digital Pins assignment                       */
/*****************************************************************/
#define UP_DOWN_SERVO_PIN           2
/* /!\ Do not use Pin 3 (used by USB) /!\ */
/* /!\ Do not use Pin 4 (used by USB) /!\ */
#define ROTATION_SERVO_PIN          5

/**************************************************************************************/
/* STEP #4: Declaration of the angles of the servos for the different motions (in °)  */
/**************************************************************************************/
#define UP_DOWN_ON_DECK_POS         120 /* Zodiac on the deck  */
#define UP_DOWN_ON_AIR_POS          180 /* Zodiac in the air   */
#define UP_DOWN_ON_SEA_POS          0   /* Zodiac at sea level */

#define ROTATION_ABOVE_DECK_POS     90  /* crane at deck side  */
#define ROTATION_ABOVE_SEA_POS      0   /* crane at sea  side  */


/***************************************************************************************************************************************/
/* STEP #5: Do a temporal diagram showing the start up and the duration of each motions of each servo                                  */
/***************************************************************************************************************************************/
/*
All the start up values (time stamp) have as reference the moment of the sequence startup order (t=0).

                            UP_DOWN_SERVO MOTION             ROTATION_SERVO MOTION                 UP_DOWN_SERVO MOTION           NO MOTION MOUVEMENT(WAITING)        UP_DOWN_SERVO MOTION                     ROTATION_SERVO MOTION              UP_DOWN_SERVO MOTION
Order                  <--DECK_TO_AIR_DURATION_MS--> <--DECK_TO_SEA_ROTATION_DURATION_MS--> <--AIR_TO_SEA_FALLING_DURATION_MS--> <--DELAY_BEFORE_RISING_UP_MS--> <--SEA_TO_AIR_RISING_DURATION_MS--> <--SEA_TO_DECK_ROTATION_DURATION_MS--> <--AIR_TO_DECK_FALLING_DURATION_MS-->
  |-------------------|-----------------------------|--------------------------------------|------------------------------------|-------------------------------|-----------------------------------|--------------------------------------|-------------------------------------|-->Time Axis
  0       START_UP_DECK_TO_AIR_MS    START_UP_DECK_TO_SEA_ROTATION_MS       START_UP_AIR_TO_SEA_FALLING_MS                                        START_UP_SEA_TO_AIR_RISING_MS     START_UP_SEA_TO_DECK_ROTATION_MS         START_UP_AIR_TO_DECK_FALLING_MS
*/

/**************************************************************************************************************************************************/
/* STEP #6: With the help of the temporal diagram, declare start up time, the motion duration of servo and optional delay                         */
/**************************************************************************************************************************************************/
/* Tune below all the motion duration. Do not forget to add a trailer 'UL' for each value to force them in Unsigned Long type */
#define START_UP_DECK_TO_AIR_MS           0UL /* 0 for immediate start up, but you can put a delay here. Ex: 2000UL, will delay the startup of the whole sequence after 2 seconds */
#define DECK_TO_AIR_DURATION_MS	           3000UL

#define START_UP_DECK_TO_SEA_ROTATION_MS  (START_UP_DECK_TO_AIR_MS + DECK_TO_AIR_DURATION_MS)
#define DECK_TO_SEA_ROTATION_DURATION_MS  3000UL

#define START_UP_AIR_TO_SEA_FALLING_MS    (START_UP_DECK_TO_SEA_ROTATION_MS + DECK_TO_SEA_ROTATION_DURATION_MS)
#define AIR_TO_SEA_FALLING_DURATION_MS    9000UL

#define DELAY_BEFORE_RISING_UP_MS         6000UL

#define START_UP_SEA_TO_AIR_RISING_MS     (START_UP_AIR_TO_SEA_FALLING_MS + AIR_TO_SEA_FALLING_DURATION_MS + DELAY_BEFORE_RISING_UP_MS)
#define SEA_TO_AIR_RISING_DURATION_MS     9000UL

#define START_UP_SEA_TO_DECK_ROTATION_MS  (START_UP_SEA_TO_AIR_RISING_MS + SEA_TO_AIR_RISING_DURATION_MS)
#define SEA_TO_DECK_ROTATION_DURATION_MS  3000UL


#define START_UP_AIR_TO_DECK_FALLING_MS   (START_UP_SEA_TO_DECK_ROTATION_MS + SEA_TO_DECK_ROTATION_DURATION_MS)
#define AIR_TO_DECK_FALLING_DURATION_MS   3000UL

/********************************************************************************************************************/
/* STEP #7: Declare here the percentage of motion to be performed at half speed for servo start up and stop         */
/********************************************************************************************************************/
#define START_STOP_PER_CENT			5L /* Percentage of motion performed at half speed for servo start and servo stop (Soft start and Soft stop) */
/* Note: due to the lack of programm memory on the DigiSpark, this feature is not used */

/************************************************************************************************************/
/* STEP #11: Use a "SequenceSt_t" structure table to declare the servo sequence                             */
/* For each table entry, arguments are:                                                                     */
/* - Servo Index                                                                                            */
/* - Initial Servo Position in °                                                                            */
/* - Final   Servo Position in °                                                                            */
/* - Motion Start Time Stamp in ms                                                                          */
/* - Motion duration in ms between initial and final position                                               */
/* - Percentage of motion performed at half speed for servo start and servo stop (Soft start and Soft stop) */
/* Note: START_STOP_PER_CENT not used (MOTION_WITHOUT_SOFT_START_AND_STOP() macro used)                     */
/************************************************************************************************************/
const SequenceSt_t ZodiacSequence[] PROGMEM = {
	SHORT_ACTION_TO_PERFORM(ToggleLed, START_UP_DECK_TO_AIR_MS) /* Switch ON the Led at the beginning of the sequence */
	SHORT_ACTION_TO_PERFORM(ToggleLed, START_UP_AIR_TO_DECK_FALLING_MS+AIR_TO_DECK_FALLING_DURATION_MS) /* Switch OFF the Led at the beginning of the sequence: You are not obliged to put this line at the end of the table */
	/* 1) The crane lifts the pneumatic Zodiac from the deck to the air and stops */
	MOTION_WITHOUT_SOFT_START_AND_STOP(UP_DOWN_SERVO, UP_DOWN_ON_DECK_POS, UP_DOWN_ON_AIR_POS, START_UP_DECK_TO_AIR_MS, DECK_TO_AIR_DURATION_MS)
	/* 2) The crane rotates (90°) to locate the pneumatic Zodiac above the sea */
	MOTION_WITHOUT_SOFT_START_AND_STOP(ROTATION_SERVO, ROTATION_ABOVE_DECK_POS, ROTATION_ABOVE_SEA_POS, START_UP_DECK_TO_SEA_ROTATION_MS, DECK_TO_SEA_ROTATION_DURATION_MS)
	/* 3) The crane drops down the pneumatic Zodiac at sea level */
	MOTION_WITHOUT_SOFT_START_AND_STOP(UP_DOWN_SERVO, UP_DOWN_ON_AIR_POS, UP_DOWN_ON_SEA_POS, START_UP_AIR_TO_SEA_FALLING_MS, AIR_TO_SEA_FALLING_DURATION_MS)
	/* 4) The crane stops during 6 seconds and 5) The crane lifts up the pneumatic Zodiac from sea level to the air and stops */
	MOTION_WITHOUT_SOFT_START_AND_STOP(UP_DOWN_SERVO, UP_DOWN_ON_SEA_POS, UP_DOWN_ON_AIR_POS, START_UP_SEA_TO_AIR_RISING_MS, SEA_TO_AIR_RISING_DURATION_MS)
	/* 6) The crane rotates (90°) to locate the pneumatic Zodiac above the deck */
	MOTION_WITHOUT_SOFT_START_AND_STOP(ROTATION_SERVO, ROTATION_ABOVE_SEA_POS, ROTATION_ABOVE_DECK_POS, START_UP_SEA_TO_DECK_ROTATION_MS, SEA_TO_DECK_ROTATION_DURATION_MS)
	/* 7) The crane drops down the pneumatic Zodiac on the deck and stops. The sequence ends. */
	MOTION_WITHOUT_SOFT_START_AND_STOP(UP_DOWN_SERVO, UP_DOWN_ON_AIR_POS, UP_DOWN_ON_DECK_POS, START_UP_AIR_TO_DECK_FALLING_MS, AIR_TO_DECK_FALLING_DURATION_MS)                                    
    };

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    
    DigiUSB.begin();

/***************************************************************************/
/* STEP #9: Init <RcSeq> library                                           */
/***************************************************************************/
    RcSeq_Init();

/****************************************************************************************/
/* STEP #10: declare the servo command signals with their digital pin number            */
/****************************************************************************************/
    RcSeq_DeclareServo(UP_DOWN_SERVO,  UP_DOWN_SERVO_PIN);
    RcSeq_DeclareServo(ROTATION_SERVO, ROTATION_SERVO_PIN);
    
/**************************************************************************************************************************/
/* STEP #11: declare the sequence command signal (0), the stick level (0), and the sequence to call                       */
/**************************************************************************************************************************/
    RcSeq_DeclareCommandAndSequence(0, 0, RC_SEQUENCE(ZodiacSequence)); /* 0, 0 since there's no RC command */
}

void loop()
{
char RxChar;
  
/***********************************************************************************************************************************/
/* STEP #12: call the refresh function inside the loop() to catch RC commands and to manage the servo positions                    */
/***********************************************************************************************************************************/
    RcSeq_Refresh();

/****************************************************************************************************************/
/* STEP  #13: the sequence can be launched directly by calling the RcSeq_LaunchSequence() function              */
/****************************************************************************************************************/
    if(DigiUSB.available())
    {
        RxChar = DigiUSB.read();
        if(RxChar == 'g') /* Go ! */
        {
            RcSeq_LaunchSequence(ZodiacSequence);
        }
        if(RxChar == 't') /* Toggle LED ! */
        {
            RcSeq_LaunchShortAction(ToggleLed); /* You can toggle LED during Servo Motion! */
        }
    }
    DigiUSB.refresh();
}

static void ToggleLed(void)
{
static boolean Status = LOW;
    Status = !Status; /* Toggle Status */
    digitalWrite(LED_PIN, Status);
}
