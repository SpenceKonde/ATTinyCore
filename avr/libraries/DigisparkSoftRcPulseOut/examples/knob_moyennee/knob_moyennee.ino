// Controlling a servo position using a potentiometer (variable resistor) 
// by Michal Rinott <http://people.interaction-ivrea.it/m.rinott> 
// Adapted to SoftRcPulseOut library by RC Navy  (http://p.loussouarn.free.fr)
// This sketch can work with ATtiny and Arduino UNO, MEGA, etc...

#include <SoftRcPulseOut.h> 
 
SoftRcPulseOut myservo;  // create servo object to control a servo 
 
#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
//Here is the POT_PIN definition for ATtiny, they do NOT need a 'A' prefix for Analogic definition
#define POT_PIN           2  // --analog pin--  (not digital) used to connect the potentiometer
#else
//Here is the POT_PIN definition for Arduino UNO, MEGA, they do need a 'A' prefix for Analogic definition
#define POT_PIN           A2 // --analog pin--  (not digital) used to connect the potentiometer
#endif

#define SERVO_PIN         3  // --digital pin-- (not analog)  used to connect the servo

#define REFRESH_PERIOD_MS 20

#define NOW               1

#define MOY_SUR_1_VALEUR        0
#define MOY_SUR_2_VALEURS       1
#define MOY_SUR_4_VALEURS       2
#define MOY_SUR_8_VALEURS       3
#define MOY_SUR_16_VALEURS      4
#define MOY_SUR_32_VALEURS      5

#define TAUX_DE_MOYENNAGE       MOY_SUR_4_VALEURS  /* Choisir ici le taux de moyennage parmi les valeurs precedentes possibles listees ci-dessus */
                                                   /* Plus le taux est élevé, plus le système est stable (diminution de la gigue), mais moins il est réactif */

#define MOYENNE(Valeur_A_Moyenner,DerniereValeurRecue,TauxDeMoyEnPuissanceDeDeux)  Valeur_A_Moyenner=((((Valeur_A_Moyenner)*((1<<(TauxDeMoyEnPuissanceDeDeux))-1)+(DerniereValeurRecue))/(1<<(TauxDeMoyEnPuissanceDeDeux)))+(TauxDeMoyEnPuissanceDeDeux-1))

int val;    // variable to read the value from the analog pin 
 
void setup() 
{ 
  
  myservo.attach(SERVO_PIN);  // attaches the servo on pin defined by SERVO_PIN to the servo object 
} 
 
void loop() 
{
static int ValMoyennee;
  val = analogRead(POT_PIN);           // reads the value of the potentiometer (value between 0 and 1023) 
  val = map(val, 0, 1023, 0, 179);     // scale it to use it with the servo (value between 0 and 180) 
  MOYENNE(ValMoyennee,val,TAUX_DE_MOYENNAGE);//If there is lots of noise: average with TAUX_DE_MOYENNAGE
  myservo.write(ValMoyennee);          // sets the servo position according to the scaled value 
  delay(REFRESH_PERIOD_MS);            // waits for the servo to get there 
  SoftRcPulseOut::refresh(NOW);        // generates the servo pulse
}

