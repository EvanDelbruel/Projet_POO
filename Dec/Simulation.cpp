#include "Simulation.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <string>
using namespace std;
namespace fs = std::filesystem;


// Constructeur
Simulation::Simulation(const std::string& filename, int iterations, int pixelSize)
    : grille(0, 0), iterations(iterations), pixelSize(pixelSize) {
    folderPath = createSimulationFolder(filename);
    grille.ficher(filename);
}
std::string getBaseName(const string& filepath) {
         size_t lastSlash = filepath.find_last_of("/\\"); // Trouve le dernier '/' ou '\' dans le chemin
         string filename = (lastSlash == string::npos) ? filepath : filepath.substr(lastSlash + 1); // Extrait le nom du fichier

         size_t lastDot = filename.find_last_of("."); // Trouve le dernier '.' dans le nom
         return (lastDot == string::npos) ? filename : filename.substr(0, lastDot); // Supprime l'extension
}

// Méthode pour créer un dossier
std::string Simulation::createSimulationFolder(const std::string& filename) {
        string baseName = getBaseName(filename); // Récupère le nom du fichier d'entrée
        int simulationNumber = 1;
        string folderName;

        // Trouve un nom de dossier unique
        while (true) {
            folderName = baseName + "_" + to_string(simulationNumber) + "_out"; // Génère un nom avec un numéro
            if (!fs::exists(folderName)) { // Vérifie si le dossier existe déjà
                fs::create_directory(folderName); // Crée le dossier
                break;
            }
            simulationNumber++; // Incrémente le numéro si le dossier existe déjà
        }

        return folderName; // Retourne le NOM du dossier
}

// Méthode pour exécuter la simulation
void Simulation::run() {
        // Crée une fenêtre graphique avec des dimensions adaptées à la grille
        sf::RenderWindow window(sf::VideoMode(grille.getNbColonnes() * pixelSize + 50, grille.getNbLignes() * pixelSize + 50), "Jeu de la Vie");

        // Charge une police pour afficher le texte
        sf::Font font;
        if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) { // Chemin vers la police
            cerr << "Erreur : Impossible de charger la police (DejaVuSans.ttf)" << endl;
            return;
        }

        // Configure le texte affiché dans la fenêtre
        sf::Text text;
        text.setFont(font); // Applique la police
        text.setCharacterSize(20); // Définit la taille du texte
        text.setFillColor(sf::Color::White); // Définit la couleur du texte (blanc)
        text.setPosition(10, grille.getNbLignes() * pixelSize); // Positionne le texte sous la grille

        int currentIteration = 1; // Initialise le compteur d'itérations
        bool spacePressed = false; // Indique si la barre espace est appuyée
        auto lastUpdate = chrono::steady_clock::now(); // Enregistre l'instant de la dernière mise à jour
         const int SPACE_PRESS_DELAY = 150;

        // Boucle principale de la simulation
        while (window.isOpen() && currentIteration <= iterations) {
            sf::Event event; // Gère les événements utilisateur
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) window.close(); // Ferme la fenêtre si l'utilisateur clique sur la croix
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) spacePressed = true; // Barre espace appuyée
                if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space) spacePressed = false; // Barre espace relâchée
            }

            auto now = chrono::steady_clock::now(); // Enregistre l'instant actuel

            // Si la barre espace est maintenue et que le délai est respecté
            if (spacePressed && chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= SPACE_PRESS_DELAY) {
                if (!grille.updateGrille()) { // Met à jour la grille et vérifie si elle est stabilisée
                    cout << "Simulation stabilisée à l'itération " << currentIteration << endl;
                    grille.saveStabilisation(currentIteration, folderPath); // Sauvegarde l'état stabilisé
                    break; // Termine la simulation si stabilisée
                }
                grille.saveIteration(currentIteration, folderPath); // Sauvegarde l'état actuel
                grille.afficherGraphique(window, text, pixelSize, currentIteration); // Met à jour l'affichage
                currentIteration++; // Passe à l'itération suivante
                lastUpdate = now; // Met à jour l'instant de la dernière mise à jour
            }
        }

        cout << "Simulation terminée !" << endl; // Affiche un message de fin
}

