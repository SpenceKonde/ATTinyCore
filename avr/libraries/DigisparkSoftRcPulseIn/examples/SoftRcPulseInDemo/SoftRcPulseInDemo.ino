#include <SoftRcPulseIn.h>
#include <TinyPinChange.h>

#define BROCHE_VOIE1  2

SoftRcPulseIn ImpulsionVoie1;


void setup()
{
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__)
    Serial.begin(9600);
    Serial.print("SoftRcPulseIn library V");Serial.print(SoftRcPulseIn::LibTextVersionRevision());Serial.print(" demo");
#endif
  ImpulsionVoie1.attache(BROCHE_VOIE1);
}

void loop()
{
  if(ImpulsionVoie1.disponible())
  {
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__)
    Serial.print("Pulse=");Serial.println(ImpulsionVoie1.largeur_us());
#endif
  }
}


