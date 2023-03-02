#include <Arduino.h>
#include "lum.h"

/*Mesure de la luminosité avec une photorésistance NSL19
paramètre d'entré:
Tesnion d'aliemntation du pont diviseur
Resolution du CAN
GPIO de mesure*/
int lux(float fTension,int iResolution,int iPin){
    int iResLum;
    float fTensLum;

    float ftirage = 10; //valeur de la resistance de tirage du pont diviseur(en kOhm)

    int iLux;
  
    iResLum = analogRead(iPin);//valeur numérique en sortie du capteur

    fTensLum = iResLum * fTension / pow(2,iResolution);//calcul de la tension du capteur
    iLux = (955 * (pow(2,iResolution) - iResLum) / (ftirage * iResLum)); /*Formule de la luminosité/valeur numérique*/

    return iLux;
}