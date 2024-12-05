#include "Regle.h"

bool RegleSt::application(bool etat, int nombreDeVoisins) const {
    if (etat && (nombreDeVoisins == 2 || nombreDeVoisins == 3)) return true;
    if (!etat && nombreDeVoisins == 3) return true;
    return false;
}

