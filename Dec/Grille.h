#ifndef GRILLE_H
#define GRILLE_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include "Position.h"
#include "Regle.h"
#include <SFML/Graphics.hpp> 


class Grille : public RegleSt {
private:
    int nbLignes, nbColonnes;
    std::vector<std::vector<bool>> grille;
    std::set<Position> cellulesVivantes;
    std::set<std::string> configurations;

public:
    Grille(int nbLignes, int nbColonnes);
    void ficher(const std::string& filename);
    void notifierVoisines(const Position& pos, std::map<Position, int>& nombreDeVoisins);
    std::string getConfigurationString() const;
    bool updateGrille();
    void saveIteration(int iteration, const std::string& folderPath);
    void saveStabilisation(int iteration, const std::string& folderPath);
    void afficherGraphique(sf::RenderWindow& window, sf::Text& text, int pixelSize, int iteration);
    int getNbLignes() const;
    int getNbColonnes() const;
};

#endif // GRILLE_H

