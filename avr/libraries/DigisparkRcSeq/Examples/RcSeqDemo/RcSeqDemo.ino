#include <RcSeq.h>
#include <TinyPinChange.h>
#include <SoftRcPulseIn.h>
#include <SoftRcPulseOut.h>

/*
IMPORTANT:
Pour compiler ce sketch, RC_SEQ_WITH_SOFT_RC_PULSE_IN_SUPPORT, RC_SEQ_WITH_SHORT_ACTION_SUPPORT et RC_SEQ_WITH_SOFT_RC_PULSE_OUT_SUPPORT
doivent etre definis dans ChemainDesLibraires/(Digispark)RcSeq/RcSeq.h.

RC Navy 2013
http://p.loussouarn.free.fr

Ce sketch de demo de la librairie RcSeq montre comment configurer tres facilement la commande d'actions ou de sequences de servo predefinies.
La commande peut etre:
- un manche de l'emetteur RC avec possibilité de definir jusqu'a 8 positions "actives" (le nombre de position doit etre pair: neutre au milieu)
- un clavier: un montage resistances/boutons-poussoirs remplacant le potentiometre du manche d'un emetteur RC
  (les resistances doivent etre d'egales valeurs avec une 2 resistances identiques "au centre/neutre" pour la zone inactive)
- un clavier "maison": un montage resistances/boutons-poussoirs remplacant le potentiometre du manche d'un emetteur RC avec des resistances pas forcement identiques
  (la largeur d'impulsion pour chaque bouton-poussoir est define dans une table, une tolerance est egalement prevue)
Les 3 exemples sont traites dans ce sketch de demo.
*/

enum {RC_VOIE1, RC_VOIE2, RC_VOIE3, NBR_VOIES_RC}; /* Declaration des voies */

enum {BP1, BP2, NBR_BP}; /* Declaration des Boutons-Poussoirs (On peut aller jusqu'à BP8) */

enum {POS_MINUS1, POS_PLUS1, NBR_POS}; /* Declaration des positions du Manche on peut aller de POS_MOINS2 à POS_PLUS2 (4 Positions actives Max)*/


/* Declaration d'un clavier "Maison": les impulsions des Boutons-Poussoirs n'ont pas besoin d'etre equidistantes */
enum {BP_MAISON1, BP_MAISON2, BP_MAISON3, NBR_BP_MAISON};
#define TOLERANCE  40 /* Tolerance en + ou en - (en micro-seconde) */
const KeyMap_t ClavierMaison[] PROGMEM ={  {VALEUR_CENTRALE_US(1100, TOLERANCE)}, /* BP_MAISON1: 1100 +/-40 us */
                                           {VALEUR_CENTRALE_US(1300, TOLERANCE)}, /* BP_MAISON2: 1300 +/-40 us */
                                           {VALEUR_CENTRALE_US(1700, TOLERANCE)}, /* BP_MAISON3: 1700 +/-40 us */
                                        };

enum {AZIMUT=0, ELEVATION , NBR_SERVO}; /* Delaration de tous les servos, 2 dans cet exemple (On peut déclaer jusqu'à 8 servos) */

/* Declaration des broches reliees aux sorties du recepteur RC */
#define BROCHE_SIGNAL_RECEPTEUR_VOIE1         8
#define BROCHE_SIGNAL_RECEPTEUR_VOIE2         2
#define BROCHE_SIGNAL_RECEPTEUR_VOIE3         9

/* Declaration des broches de commande des servos */
#define BROCHE_SIGNAL_SERVO_EL                3
#define BROCHE_SIGNAL_SERVO_AZ                4

/* Declaration des differents angles des servos */
#define ELEVATION_POS_PONT                    120 /* position zodiac sur pont   (Pos A) */
#define ELEVATION_POS_HAUT                    180 /* position zodiac en haut    (Pos B) */
#define ELEVATION_POS_MER                     0   /* position zodiac dans l'eau (pos C) */

#define AZIMUT_POS_PONT                       90 /* position rotation sur pont */
#define AZIMUT_POS_MER                        0  /* position rotation sur mer  */

/* Declaration des moments de demarrage ainsi que la duree des mouvement de servo */
#define DEMARRAGE_MONTEE_PONT_HAUT_MS           0L /* 0 pour demarrage immediat, mais on peut mettre une tempo ici. Ex 2000L, va differer la sequence complete de 2 secondes */
#define DUREE_MONTEE_PONT_HAUT_MS            3000L

#define DEMARRAGE_ROTATION_PONT_MER_MS       (DEMARRAGE_MONTEE_PONT_HAUT_MS+DUREE_MONTEE_PONT_HAUT_MS)
#define DUREE_ROTATION_PONT_MER_MS           3000L

#define DEMARRAGE_DESCENTE_HAUT_MER_MS       (DEMARRAGE_ROTATION_PONT_MER_MS+DUREE_ROTATION_PONT_MER_MS)
#define DUREE_DESCENTE_HAUT_MER_MS           9000L

#define ATTENTE_AVANT_REMONTEE_MS            6000L /* Exemple d'utilisation d'une temporisation */

#define DEMARRAGE_MONTEE_MER_HAUT_MS         (DEMARRAGE_DESCENTE_HAUT_MER_MS+DUREE_DESCENTE_HAUT_MER_MS+ATTENTE_AVANT_REMONTEE_MS)
#define DUREE_MONTEE_MER_HAUT_MS             9000L

#define DEMARRAGE_ROTATION_MER_PONT_MS       (DEMARRAGE_MONTEE_MER_HAUT_MS+DUREE_MONTEE_MER_HAUT_MS)
#define DUREE_ROTATION_MER_PONT_MS           3000L


#define DEMARRAGE_DESCENTE_HAUT_PONT_MS      (DEMARRAGE_ROTATION_MER_PONT_MS+DUREE_ROTATION_MER_PONT_MS)
#define DUREE_DESCENTE_HAUT_PONT_MS          3000L

#define DEM_ARRET_POUR_CENT                  5 /* Pourcentage du mouvement devant etre effectue a mi-vitesse pour demarrage servo et arret servo (Soft start et Soft stop) */

/* Declaration de la table de sequence des mouvements des servo et des actions courtes */
const SequenceSt_t SequenceServoEtActionCourte[] PROGMEM = {
    ACTION_COURTE_A_EFFECTUER(InverseLed, DEMARRAGE_MONTEE_PONT_HAUT_MS)
    /* Montee du Zodiac du pont vers la position haute */
    MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(ELEVATION, ELEVATION_POS_PONT, ELEVATION_POS_HAUT, DEMARRAGE_MONTEE_PONT_HAUT_MS, DUREE_MONTEE_PONT_HAUT_MS, DEM_ARRET_POUR_CENT)
    /* Rotation Grue du pont vers la mer */
    MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(AZIMUT, AZIMUT_POS_PONT, AZIMUT_POS_MER, DEMARRAGE_ROTATION_PONT_MER_MS, DUREE_ROTATION_PONT_MER_MS, DEM_ARRET_POUR_CENT)
    /* Descente du Zodiac depuis la position haute vers la la mer */
    MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(ELEVATION, ELEVATION_POS_HAUT, ELEVATION_POS_MER, DEMARRAGE_DESCENTE_HAUT_MER_MS, DUREE_DESCENTE_HAUT_MER_MS, DEM_ARRET_POUR_CENT)
    ACTION_COURTE_A_EFFECTUER(InverseLed, DEMARRAGE_DESCENTE_HAUT_MER_MS + DUREE_DESCENTE_HAUT_MER_MS)
    ACTION_COURTE_A_EFFECTUER(InverseLed, DEMARRAGE_MONTEE_MER_HAUT_MS)
    /* Montee du Zodiac de la mer vers la position haute */
    MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(ELEVATION, ELEVATION_POS_MER, ELEVATION_POS_HAUT, DEMARRAGE_MONTEE_MER_HAUT_MS, DUREE_MONTEE_MER_HAUT_MS, DEM_ARRET_POUR_CENT)
    /* Rotation Grue de la mer vers le pont */
    MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(AZIMUT, AZIMUT_POS_MER, AZIMUT_POS_PONT, DEMARRAGE_ROTATION_MER_PONT_MS, DUREE_ROTATION_MER_PONT_MS, DEM_ARRET_POUR_CENT)
    /* Descente du Zodiac de la position haute vers le pont */
    MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(ELEVATION, ELEVATION_POS_HAUT, ELEVATION_POS_PONT, DEMARRAGE_DESCENTE_HAUT_PONT_MS, DUREE_DESCENTE_HAUT_PONT_MS, DEM_ARRET_POUR_CENT)                                   
    ACTION_COURTE_A_EFFECTUER(InverseLed, DEMARRAGE_DESCENTE_HAUT_PONT_MS + DUREE_DESCENTE_HAUT_PONT_MS)
    };

#define LED		13

void setup()
{
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__)
    Serial.begin(9600);
    Serial.print("RcSeq library V");Serial.print(RcSeq_LibTextVersionRevision());Serial.print(" demo: RcSeqDemo");
#endif
    RcSeq_Init();
    
    /* Declaration des Servos */
    RcSeq_DeclareServo(ELEVATION, BROCHE_SIGNAL_SERVO_EL);
    RcSeq_DeclareServo(AZIMUT,    BROCHE_SIGNAL_SERVO_AZ);

    /* Commande d'une action courte et d'une sequence de servos avec 2 BP du clavier de la VOIE1 */
    RcSeq_DeclareSignal(RC_VOIE1, BROCHE_SIGNAL_RECEPTEUR_VOIE1);
    RcSeq_DeclareClavier(RC_VOIE1, 1000, 2000, NBR_BP);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE1, BP1, InverseLed);
    RcSeq_DeclareCommandeEtSequence(RC_VOIE1, BP2, RC_SEQUENCE(SequenceServoEtActionCourte), NULL);

    /* Commande d'une action courte et d'une sequence de servos avec le manche de la VOIE2 */
    RcSeq_DeclareSignal(RC_VOIE2, BROCHE_SIGNAL_RECEPTEUR_VOIE2);
    RcSeq_DeclareManche(RC_VOIE2, 1000, 2000, NBR_POS);
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE2, POS_MINUS1, InverseLed);
    RcSeq_DeclareCommandeEtSequence(RC_VOIE2, POS_PLUS1, RC_SEQUENCE(SequenceServoEtActionCourte), NULL);

    /* Commande d'une action courte et d'une sequence de servos avec le clavier "maison" de la VOIE3 */
    RcSeq_DeclareSignal(RC_VOIE3, BROCHE_SIGNAL_RECEPTEUR_VOIE3);
    RcSeq_DeclareClavierMaison(RC_VOIE3, RC_CLAVIER_MAISON(ClavierMaison));
    RcSeq_DeclareCommandeEtActionCourte(RC_VOIE3, BP_MAISON1, InverseLed);
    RcSeq_DeclareCommandeEtSequence(RC_VOIE3, BP_MAISON3, RC_SEQUENCE(SequenceServoEtActionCourte), NULL);

    pinMode(LED, OUTPUT);
}

void loop()
{
    RcSeq_Rafraichit();
}

/* Action associee au BP1 de la VOIE1 ou au manche position basse de la VOIE2 ou au BP_MAISON1 de la VOIE3 */
void InverseLed(void)
{
static uint32_t DebutMs = millis(); /* static, pour conserver l'etat entre 2 appels de la fonction */
static boolean Etat = HIGH;         /* static, pour conserver l'etat entre 2 appels de la fonction */

  if(millis() - DebutMs >= 500UL) /* Depuis RcSeq V2.0, la tempo inter-commande doit etre geree dans le sketch utilisateur */
  {
    DebutMs = millis();
    digitalWrite(LED, Etat);
    Etat = !Etat; /* Au prochain appel de InverseLed(), l'etat de la LED sera inverse */
  }
}
