/*
IMPORTANT:
=========
La librairie "RcSeq" utilise la technique de programmation dite "asynchrone", c'est-a-dire qu'aucun appel a des fonctions bloquantes telles que la fonction delay()
ou la fonction pulseIn() n'est effectue.
Ceci se traduit par un temps de boucle principale inferieur a 70 micro-secondes bien que les servos soient rafraichis toutes les 20ms a l'aide de la methode
Rafraichit() qui doit etre appelee dans la fonction loop(). Cela laisse donc enormement de temps au micro-controleur pour faire "en meme temps" d'autres taches.
Par exemple dans ce sketch, il est possible d'envoyer la commande 'i' via la serial console pour inverser l'etat de la LED connectee a la pin digitale 13 pendant
que les servos sont en mouvement.

Ce sketch illustre l'utilisation de la librairie "RcSeq" qui permet de sequencer tres facilement des servos et des actions courtes a l'aide de la librairie "SoftwareServo".
Les actions courtes doivent durer moins de 20ms pour ne pas perturber la commande des servos.
Si ce sketch est charge dans une carte UNO, il est possible de lancer la sequence en tapant 'g' puis Entree dans la serial console de l'EDI Arduino.
En tapant 'i' puis Entree, l'action InverseLed() est executee. Comme "RcSeq" est asynchrone, il est possible de le faire pendant que les servos tournent.
La possibilite de lancer les sequence et action courte via la serial console evite de sortir et cabler l'ensemble RC pour lancer la sequence et l'action.

Dans cet exemple, la sequence declaree est la mise a l'eau d'un Zodiac avec une grue depuis un bateau de service type baliseur:
1) La grue souleve le Zodiac en position haute puis s'arrete
2) La grue fait une rotation de 90° pour positionner le Zodiac au dessus de l'eau
3) La grue descend le Zodiac au niveau de l'eau
4) La grue reste sans action pendant 6 secondes
5) La grue remonte le Zodiac en position haute puis s'arrete
6) La grue fait une rotation de 90° pour positionner le Zodiac au dessus du pont
7) La grue descend le Zodiac en position basse puis s'arrete. La sequence est terminee.
Cette sequence utilise:
- 2 commande RC sur le meme manche (Impulsion d'au moins 1/4 de seconde en position mi-course pour l'action courte et extreme pour la sequnce avec le manche de l'emetteur RC)
  ou la commande 'i' ou 'g' depuis la serial console de l'EDI Arduino
- 2 servos (un servo "Azimut" pour les rotations et un servo "Elevation" pour la montee/descente)
*/

/***************************************************/
/* ETAPE N°1: Inclure les 4 librairies necessaires */
/***************************************************/
#include <RcSeq.h>
#include <TinyPinChange.h>
#include <SoftRcPulseIn.h>
#include <SoftRcPulseOut.h>

/*****************************************************/
/* ETAPE N°2: Enumeration des signaux de commande RC */
/*****************************************************/
enum {SIGNAL_RC=0, NBR_SIGNAL}; /* Delaration de tous les signaux de commande (sortie voie du recepteur), un seul dans cet exemple */

/****************************************************************/
/* ETAPE N°3: Enumeration des differentes position du manche RC */
/****************************************************************/
enum {RC_IMPULSION_NIVEAU_MOINS_2, RC_IMPULSION_NIVEAU_MOINS_1, RC_IMPULSION_NIVEAU_PLUS_1, RC_IMPULSION_NIVEAU_PLUS_2, NBR_RC_IMPULSIONS};

/*****************************************************************/
/* ETAPE N°4: Enumeration des servos utilisés pour les sequences */
/*****************************************************************/
enum {AZIMUT=0, ELEVATION , NBR_SERVO}; /* Delaration de tous les servos, 2 dans cet exemple */

/*********************************************************************************/
/* ETAPE N°5: Affectation des broches Digitales (PIN) des signaux de commande RC */
/*********************************************************************************/
#define BROCHE_SIGNAL_RECEPTEUR         2

/*****************************************************************************************/
/* ETAPE N°6: Affectation des broches Digitales (PIN) des signaux de commande des servos */
/*****************************************************************************************/
#define BROCHE_SIGNAL_SERVO_EL          3
#define BROCHE_SIGNAL_SERVO_AZ          4

/**************************************************************************************/
/* ETAPE N°7: Declaration des angles des servos pour les differents mouvements (en °) */
/**************************************************************************************/
#define ELEVATION_POS_PONT              120 /* position zodiac sur pont   (Pos A) */
#define ELEVATION_POS_HAUT              180 /* position zodiac en haut    (Pos B) */
#define ELEVATION_POS_MER               0   /* position zodiac dans l'eau (pos C) */

#define AZIMUT_POS_PONT                 90 /* position rotation sur pont */
#define AZIMUT_POS_MER                  0  /* position rotation sur mer  */


/***************************************************************************************************************************************/
/* ETAPE N°8: Faire un croquis temporel faisant apparaitre les moments de demarrages et les duree des mouvements des differents servos */
/***************************************************************************************************************************************/
/*
Toutes les valeurs de demarrage ont comme reference le moment de l'ordre de demarrage de sequence (t=0).

                           MOUVEMENT SERVO ELEVATION         MOUVEMENT SERVO AZIMUT            MOUVEMENT SERVO ELEVATION      AUCUN MOUVEMENT(ATTENTE)        MOUVEMENT SERVO ELEVATION          MOUVEMENT SERVO AZIMUT            MOUVEMENT SERVO ELEVATION
Ordre                  <---DUREE_MONTEE_PONT_HAUT_MS--> <--DUREE_ROTATION_PONT_MER_MS----> <--DUREE_DESCENTE_HAUT_MER_MS--><--ATTENTE_AVANT_REMONTEE_MS--><---DUREE_MONTEE_MER_HAUT_MS---><----DUREE_ROTATION_MER_PONT_MS-----><--DUREE_DESCENTE_HAUT_PONT_MS-->
  |-------------------|--------------------------------|----------------------------------|--------------------------------|------------------------------|-------------------------------|------------------------------------|--------------------------------|-->Axe du Temps
  0     DEMARRAGE_MONTEE_PONT_HAUT_MS   DEMARRAGE_ROTATION_PONT_MER_MS     DEMARRAGE_DESCENTE_HAUT_MER_MS                                  DEMARRAGE_MONTEE_MER_HAUT_MS    DEMARRAGE_ROTATION_MER_PONT_MS        DEMARRAGE_DESCENTE_HAUT_PONT_MS
*/

/**************************************************************************************************************************************************/
/* ETAPE N°9: A l'aide du croquis temporel, declarer les moments de demarrage, les durees des movement de servo et les eventuelles temporisations */
/**************************************************************************************************************************************************/
/* Regler ci-dessous les temps de mouvement en ms. Ne pas oulier de d'ajouter un 'L' a la fin de la valeur pour forcer les valeurs en type Long */
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

/********************************************************************************************************************/
/* ETAPE N°10: Declarer le pourcentage de mouvement devant etre a mi-vitesse pour les demarrage et arret des servos */
/********************************************************************************************************************/
#define DEM_ARRET_POUR_CENT                  5 /* Pourcentage du mouvement devant etre effectue a mi-vitesse pour demarrage servo et arret servo (Soft start et Soft stop) */

/***************************************************************************************************************************************************************/
/* ETAPE N°11: Dans une structure de type "SequenceSt_t", a l'aide de la macro MVT_AVEC_DEBUT_ET_FIN_MVT_LENTS(), declarer le N° de servo, l'angle initial,    */
/* l'angle final, le moment de demarrage, la duree du mouvement et le pourcentage de mouvement devant etre a mi-vitesse pour les demarrage et arret des servos */
/* Il est possible d'inclure des actions courtes. Il suffit d'utiliser la macro ACTION_COURTE_A_EFFECTUER() en donnant le nom de la fonction a appeler et le   */
/* moment ou l'action doit avoir lieu. Dans cet exemple, la LED s'allume pendant que les servos tournent et s'eteint pendant la pause de 6 secondes.           */
/***************************************************************************************************************************************************************/
const SequenceSt_t SequencePlus2[] PROGMEM = {
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
    Serial.print("RcSeq library V");Serial.print(RcSeq_LibTextVersionRevision());Serial.print(" demo: RcSeqZodiac");
#endif

/***************************************************************************/
/* ETAPE N°12: Appeler la fonction d'initialisation de la libraire "RcSeq" */
/***************************************************************************/
    RcSeq_Init();

/**************************************************************************************/
/* ETAPE N°13: declarer le(s) signal(aux) de commande RC avec leur N° de pin digitale */
/**************************************************************************************/
    RcSeq_DeclareSignal(SIGNAL_RC, BROCHE_SIGNAL_RECEPTEUR);

/******************************************************************************************/
/* ETAPE N°14: que le signal RC est associe a un manche qui a NBR_RC_IMPULSIONS positions */
/*****************************************************************************************/
    RcSeq_DeclareManche(SIGNAL_RC, 1000, 2000, NBR_RC_IMPULSIONS);
    
/********************************************************************************************/
/* ETAPE N°15: declarer le(s) signal(aux) ce commande de servo avec leur N° de pin digitale */
/********************************************************************************************/
    RcSeq_DeclareServo(ELEVATION, BROCHE_SIGNAL_SERVO_EL);
    RcSeq_DeclareServo(AZIMUT,    BROCHE_SIGNAL_SERVO_AZ);
    
/**************************************************************************************************************************/
/* ETAPE N°16: declarer le signal de commande de sequence, le niveau du manche, et la sequence ou action courte a appeler */
/**************************************************************************************************************************/
    RcSeq_DeclareCommandeEtSequence(SIGNAL_RC, RC_IMPULSION_NIVEAU_PLUS_2, RC_SEQUENCE(SequencePlus2), NULL); // Voici comment declarer une sequence actionnee par une impulsion Niveau Plus 2 (manche en position extreme pendant au moins 250 ms)

    pinMode(LED, OUTPUT);
    RcSeq_DeclareCommandeEtActionCourte(SIGNAL_RC, RC_IMPULSION_NIVEAU_MOINS_1, InverseLed); // Voici comment declarer une action actionnee par une impulsion Niveau Moins 1 (manche en position mi-course pendant au moins 250 ms)
}

void loop()
{
  
/***********************************************************************************************************************************/
/* ETAPE N°17: appeler la fonction Rafraichit dans la fonction loop() pour capter les commandes RC et gerer la position des servos */
/***********************************************************************************************************************************/
    RcSeq_Rafraichit();

/******************************************************************************************************/
/* ETAPE N°18: optionnellement, autoriser le lancement des Sequences ou Actions via la serial console */
/******************************************************************************************************/
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__)
int RxChar;
    /* Lance la sequence en envoyant le caractere 'g' dans la serial console: cela permet de tester la sequence de servo avec une carte UNO sans utiliser d'ensemble RC */
    if(Serial.available() > 0)
    {
        RxChar=Serial.read();
        if(tolower(RxChar)=='g') /* Go ! */
        {
            RcSeq_LanceSequence(SequencePlus2);
        }
        if(tolower(RxChar)=='i') /* inverse led ! */
        {
            RcSeq_LanceActionCourte(InverseLed);
        }
    }
#endif
}

/* Action associee au manche a mi-course */
void InverseLed(void)
{
static boolean Etat=HIGH; /* static, pour conserver l'etat entre 2 appels de la fonction */
	digitalWrite(LED, Etat);
	Etat=!Etat; /* AU prochain appel de InverseLed(), l'etat de la LED sera inverse */
}
