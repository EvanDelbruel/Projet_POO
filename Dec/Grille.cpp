#include "Grille.h"
#include <SFML/Graphics.hpp> // Bibliothèque utilisée pour l'affichage graphique
#include <iostream>          // Pour l'entrée et la sortie standard
#include <vector>            // Pour gérer la grille en tant que matrice 2D
#include <set>               // Pour stocker les configurations uniques de la grille
#include <fstream>           // Pour lire et écrire des fichiers
#include <filesystem>        // Pour la gestion des fichiers et des dossiers
#include <sstream>           // Pour manipuler des chaînes de caractères
#include <chrono>            // Pour gérer le temps et les délais

using namespace std;
using namespace chrono;       // Simplifie l'utilisation des types liés au temps
namespace fs = std::filesystem; // Simplifie l'accès aux fonctionnalités de filesystem

// Constructeur
Grille::Grille(int nbLignes, int nbColonnes)
    : nbLignes(nbLignes), nbColonnes(nbColonnes),
      grille(nbLignes, std::vector<bool>(nbColonnes, false)) {}
std::string Grille::getConfigurationString() const{
        stringstream ss;
        for (const auto& row : grille) {
            for (bool cell : row) {
                ss << (cell ? "1" : "0"); // Ajoute '1' si la cellule est vivante, '0' sinon
            }
        }
        return ss.str(); // Retourne la chaîne correspondant à l'état actuel
    }

// Méthode pour charger une grille depuis un fichier
void Grille::ficher(const std::string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Erreur : Impossible d'ouvrir le fichier !" << endl;
            exit(1); // Termine le programme si le fichier ne peut pas être ouvert
        }
        if (!(file >> nbLignes >> nbColonnes) || nbLignes <= 0 || nbColonnes <= 0) {
         cout<<"Fichier de configuration invalide."<<endl;
         exit(1);}


        file >> nbLignes >> nbColonnes; // Lit les dimensions de la grille depuis le fichier
        grille.resize(nbLignes, vector<bool>(nbColonnes, false)); // Redimensionne la grille

        for (int i = 0; i < nbLignes; ++i) {
            for (int j = 0; j < nbColonnes; ++j) {
                int state;
                file >> state; // Lit l'état de chaque cellule (0 ou 1)
                grille[i][j] = (state == 1); // Affecte l'état à la cellule
                if (grille[i][j]) cellulesVivantes.insert(Position(i, j)); // Ajoute la cellule à l'ensemble si elle est vivante
            }
        }

        file.close(); // Ferme le fichier après lecture
        configurations.insert(getConfigurationString()); 
}

// Mise à jour de la grille
void Grille::notifierVoisines(const Position& pos, std::map<Position, int>& nombreDeVoisins){
        for (int dx = -1; dx <= 1; ++dx) {       // Parcourt les 8 voisins potentiels (y compris diagonales)
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue; // Ignore la cellule elle-même

                // Calcul des coordonnées voisines avec gestion de la grille torique
                int nx = (pos.getX() + dx + nbLignes) % nbLignes; // Gestion des bords pour la coordonnée x
                int ny = (pos.getY() + dy + nbColonnes) % nbColonnes; // Gestion des bords pour la coordonnée y

                nombreDeVoisins[Position(nx, ny)]++; // Incrémente le nombre de voisins pour cette position
            }
        }
    }


bool Grille::updateGrille() {
        map<Position, int> nombreDeVoisins; // Compte le nombre de voisins de chaque cellule

        // Parcourt les cellules vivantes pour notifier le nombre de voisines à leurs voisines
        for (const auto& pos : cellulesVivantes) {
            notifierVoisines(pos, nombreDeVoisins);
        }

        set<Position> nouvellesCellulesVivantes; // Ensemble des nouvelles cellules vivantes

        // Applique les règles du jeu aux cellules ayant des voisins
        for (const auto& [pos, count] : nombreDeVoisins) {    // découpe nombreDeVoisins pour prendre d'un coté la position (x,y) de la cellule et de l'autre coté le nombre de voisin qu'elle possède (count)
            bool estVivante = grille[pos.getX()][pos.getY()]; 
            if (regle.application(estVivante, count)) {
                nouvellesCellulesVivantes.insert(pos); // Ajoute les cellules qui deviennent vivantes
            } 
        }
        

        // Met à jour la grille avec les nouvelles cellules vivantes
        grille.assign(nbLignes, vector<bool>(nbColonnes, false)); // Réinitialise la grille
        for (const auto& pos : nouvellesCellulesVivantes) {
            grille[pos.getX()][pos.getY()] = true; // Active les cellules vivantes
        }
        cellulesVivantes = nouvellesCellulesVivantes; // Met à jour l'ensemble des cellules vivantes

        // Vérifie si la configuration est répétée
        string currentConfiguration = getConfigurationString();
        if (configurations.count(currentConfiguration)) return false; // Si la configuration est identique, la grille est stabilisée

        configurations.insert(currentConfiguration); // Ajoute la nouvelle configuration à l'historique
        return true; // La grille a évolué
        }

// Sauvegarde une itération
void Grille::saveIteration(int iteration, const std::string& folderPath) {
            stringstream filename;
        filename << folderPath << "/iteration_" << iteration << ".txt"; // Crée le nom du fichier pour cette itération
        ofstream file(filename.str());

        // Écrit les informations dans le fichier
        file << "Iteration: " << iteration << "\n";
        file << "Cellules vivantes: " << cellulesVivantes.size() << "\n";
        file << "Dimensions: " << nbLignes << "x" << nbColonnes << "\n";

        // Écrit l'état de la grille ligne par ligne
        for (int i = 0; i < nbLignes; ++i) {
            for (int j = 0; j < nbColonnes; ++j) {
                file << (grille[i][j] ? "1 " : "0 "); // Écrit '1' pour vivante, '0' pour morte
            }
            file << "\n";
        }
}

// Sauvegarde l'état stabilisé
void Grille::saveStabilisation(int iteration, const std::string& folderPath) {
        stringstream filename;
        filename << folderPath << "/stabilisation_iteration_" << iteration << ".txt";
        ofstream file(filename.str());

        // Écrit les informations de stabilisation
        file << "Stabilisation à l'iteration: " << iteration << "\n";
        file << "Cellules vivantes: " << cellulesVivantes.size() << "\n";
        file << "Dimensions: " << nbLignes << "x" << nbColonnes << "\n";

        // Écrit l'état de la grille ligne par ligne
        for (int i = 0; i < nbLignes; ++i) {
            for (int j = 0; j < nbColonnes; ++j) {
                file << (grille[i][j] ? "1 " : "0 ");
            }
            file << "\n";
        }
}

// Affichage graphique
void Grille::afficherGraphique(sf::RenderWindow& window, sf::Text& text, int pixelSize, int iteration) {
        window.clear(sf::Color::Black); 
        text.setString("Iteration : " + to_string(iteration)); 
        sf::RectangleShape cell(sf::Vector2f(pixelSize - 1.0f, pixelSize - 1.0f)); 

        // Dessine les cellules vivantes
        for (int i = 0; i < nbLignes; ++i) {
            for (int j = 0; j < nbColonnes; ++j) {
                if (grille[i][j]) {
                    cell.setFillColor(sf::Color::White);}
                else{
                    cell.setFillColor(sf::Color::Black);}
                    cell.setPosition(j * pixelSize, i * pixelSize);
                    window.draw(cell); // Ajoute la cellule à la fenêtre
                }
            }
            window.draw(text); // Dessine le texte sur la fenêtre
            window.display(); // Met à jour l'affichage graphique
        }

        

int Grille::getNbLignes() const { return nbLignes; }
int Grille::getNbColonnes() const { return nbColonnes; }

