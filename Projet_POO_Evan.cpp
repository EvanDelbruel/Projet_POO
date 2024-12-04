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

// **Constantes**
const int SPACE_PRESS_DELAY = 150; // Temps minimum (en millisecondes) entre deux itérations lors de l'appui sur la barre espace

// **Fonction pour extraire le nom de base d'un fichier**
// Retourne le nom du fichier d'entrée sans son chemin ni son extension.
string getBaseName(const string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\"); // Trouve le dernier '/' ou '\' dans le chemin
    string filename = (lastSlash == string::npos) ? filepath : filepath.substr(lastSlash + 1); // Extrait le nom du fichier

    size_t lastDot = filename.find_last_of("."); // Trouve le dernier '.' dans le nom
    return (lastDot == string::npos) ? filename : filename.substr(0, lastDot); // Supprime l'extension
}

// **Classe Position**
// Représente une position (x, y) dans la grille du jeu de la vie
class Position {
private:
    int x, y; // Coordonnées de la position

public:
    // **Constructeurs**
    Position(int x = 0, int y = 0) : x(x), y(y) {} // Initialise la position avec des valeurs par défaut

    // **Accesseurs**
    int getX() const { return x; } // Retourne la coordonnée x
    int getY() const { return y; } // Retourne la coordonnée y

    // **Mutateurs**
    void setX(int newX) { x = newX; } // Modifie la coordonnée x
    void setY(int newY) { y = newY; } // Modifie la coordonnée y

    // **Opérateur de comparaison**
    // Permet de comparer deux positions pour les utiliser dans un `std::set`
    bool operator<(const Position& other) const {
        return tie(x, y) < tie(other.x, other.y); // Compare en fonction des coordonnées (x, y)
    }

    // **Opérateur d'égalité**
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y; // Retourne vrai si les deux positions sont identiques
    }
};

// **Classe abstraite Regle**
// Définit une interface pour appliquer des règles au jeu
class Regle {
public:
    virtual bool application(bool etat, int voisin) const = 0; // Méthode virtuelle pure
};

// **Classe RegleSt**
// Implémente les règles standard du jeu de la vie
class RegleSt : public Regle {
public:
    // Applique les règles standard :
    // - Une cellule vivante reste vivante avec 2 ou 3 voisins vivants.
    // - Une cellule morte devient vivante avec exactement 3 voisins vivants.
    bool application(bool etat, int voisin) const override {
        if (etat && (voisin == 2 || voisin == 3)) return true; // Cellule vivante survit
        if (!etat && voisin == 3) return true; // Cellule morte devient vivante
        return false; // Sinon, la cellule reste morte ou meurt
    }
};

// **Classe Grille**
// Représente la grille et gère la logique principale du jeu de la vie
class Grille : public RegleSt {
private:
    int dim1, dim2;                   // Dimensions de la grille (lignes, colonnes)
    vector<vector<bool>> grille;      // Matrice contenant l'état des cellules (true = vivante, false = morte)
    set<Position> cellulesVivantes;  // Ensemble des cellules actuellement vivantes
    set<string> configurations;      // Historique des configurations uniques pour détecter les répétitions

    // **Méthode pour notifier les voisins d'une cellule vivante**
    void notifierVoisines(const Position& pos, map<Position, int>& voisins) {
        for (int dx = -1; dx <= 1; ++dx) {       // Parcourt les 8 voisins potentiels (y compris diagonales)
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue; // Ignore la cellule elle-même

                // Calcul des coordonnées voisines avec gestion de la grille torique
                int nx = (pos.getX() + dx + dim1) % dim1; // Gestion des bords pour la coordonnée x
                int ny = (pos.getY() + dy + dim2) % dim2; // Gestion des bords pour la coordonnée y

                voisins[Position(nx, ny)]++; // Incrémente le nombre de voisins pour cette position
            }
        }
    }

    // **Génère une chaîne représentant l'état actuel de la grille**
    // Cela permet de comparer facilement deux configurations
    string getConfigurationString() const {
        stringstream ss;
        for (const auto& row : grille) {
            for (bool cell : row) {
                ss << (cell ? "1" : "0"); // Ajoute '1' si la cellule est vivante, '0' sinon
            }
        }
        return ss.str(); // Retourne la chaîne correspondant à l'état actuel
    }

public:
    // **Constructeur**
    Grille(int dim1, int dim2) : dim1(dim1), dim2(dim2), grille(dim1, vector<bool>(dim2, false)) {}

    // **Charge une grille initiale depuis un fichier**
    void ficher(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Erreur : Impossible d'ouvrir le fichier !" << endl;
            exit(1); // Termine le programme si le fichier ne peut pas être ouvert
        }

        file >> dim1 >> dim2; // Lit les dimensions de la grille depuis le fichier
        grille.resize(dim1, vector<bool>(dim2, false)); // Redimensionne la grille

        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                int state;
                file >> state; // Lit l'état de chaque cellule (0 ou 1)
                grille[i][j] = (state == 1); // Affecte l'état à la cellule
                if (grille[i][j]) cellulesVivantes.insert(Position(i, j)); // Ajoute la cellule à l'ensemble si elle est vivante
            }
        }

        file.close(); // Ferme le fichier après lecture
        configurations.insert(getConfigurationString()); // Ajoute la configuration initiale à l'historique
    }

    // **Met à jour la grille pour passer à l'itération suivante**
    bool updateGrille() {
        map<Position, int> voisins; // Compte les voisins de chaque cellule

        // Parcourt les cellules vivantes pour notifier leurs voisines
        for (const auto& pos : cellulesVivantes) {
            notifierVoisines(pos, voisins);
        }

        set<Position> nouvellesCellulesVivantes; // Ensemble des nouvelles cellules vivantes

        // Applique les règles du jeu aux cellules ayant des voisins
        for (const auto& [pos, count] : voisins) {
            bool estVivante = grille[pos.getX()][pos.getY()];
            if (application(estVivante, count)) {
                nouvellesCellulesVivantes.insert(pos); // Ajoute les cellules qui deviennent vivantes
            }
        }

        // Met à jour la grille avec les nouvelles cellules vivantes
        grille.assign(dim1, vector<bool>(dim2, false)); // Réinitialise la grille
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

    // **Sauvegarde une itération dans un fichier**
    void saveIteration(int iteration, const string& folderPath) {
        stringstream filename;
        filename << folderPath << "/iteration_" << iteration << ".txt"; // Crée le nom du fichier pour cette itération
        ofstream file(filename.str());

        // Écrit les informations dans le fichier
        file << "Iteration: " << iteration << "\n";
        file << "Cellules vivantes: " << cellulesVivantes.size() << "\n";
        file << "Dimensions: " << dim1 << "x" << dim2 << "\n";

        // Écrit l'état de la grille ligne par ligne
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                file << (grille[i][j] ? "1 " : "0 "); // Écrit '1' pour vivante, '0' pour morte
            }
            file << "\n";
        }
    }

    // **Sauvegarde l'état stabilisé de la grille dans un fichier**
    void saveStabilisation(int iteration, const string& folderPath) {
        stringstream filename;
        filename << folderPath << "/stabilisation_iteration_" << iteration << ".txt";
        ofstream file(filename.str());

        // Écrit les informations de stabilisation
        file << "Stabilisation à l'iteration: " << iteration << "\n";
        file << "Cellules vivantes: " << cellulesVivantes.size() << "\n";
        file << "Dimensions: " << dim1 << "x" << dim2 << "\n";

        // Écrit l'état de la grille ligne par ligne
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                file << (grille[i][j] ? "1 " : "0 ");
            }
            file << "\n";
        }
    }

    // **Affiche graphiquement la grille**
    void afficherGraphique(sf::RenderWindow& window, sf::Text& text, int pixelSize, int iteration) {
        window.clear(); // Efface la fenêtre graphique
        text.setString("Iteration : " + to_string(iteration)); // Met à jour le texte avec le numéro d'itération

        // Dessine les cellules vivantes
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                if (grille[i][j]) {
                    sf::RectangleShape cell(sf::Vector2f(pixelSize - 1, pixelSize - 1)); // Crée une cellule de taille spécifiée
                    cell.setPosition(j * pixelSize, i * pixelSize); // Positionne la cellule sur la grille
                    cell.setFillColor(sf::Color::White); // Définit la couleur de la cellule (blanc pour vivante)
                    window.draw(cell); // Ajoute la cellule à la fenêtre
                }
            }
        }

        window.draw(text); // Dessine le texte sur la fenêtre
        window.display(); // Met à jour l'affichage graphique
    }

    // **Accesseurs pour les dimensions**
    int getDim1() const { return dim1; } // Retourne le nombre de lignes
    int getDim2() const { return dim2; } // Retourne le nombre de colonnes
};

// **Classe Simulation**
// Gère l'exécution complète de la simulation
class Simulation {
private:
    Grille grille; // Instance de la grille
    string folderPath; // Chemin du dossier où les fichiers seront sauvegardés
    int iterations; // Nombre maximal d'itérations
    int pixelSize; // Taille des pixels pour l'affichage graphique

public:
    // **Constructeur**
    Simulation(const string& filename, int iterations, int pixelSize)
        : grille(0, 0), iterations(iterations), pixelSize(pixelSize) {
        folderPath = createSimulationFolder(filename); // Crée un dossier unique basé sur le fichier d'entrée
        grille.ficher(filename); // Charge la grille initiale depuis le fichier
    }

    // **Crée un dossier pour sauvegarder la simulation**
    string createSimulationFolder(const string& filename) {
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

        return folderName; // Retourne le chemin du dossier
    }

    // **Exécute la simulation**
    void run() {
        // Crée une fenêtre graphique avec des dimensions adaptées à la grille
        sf::RenderWindow window(sf::VideoMode(grille.getDim2() * pixelSize, grille.getDim1() * pixelSize + 50), "Jeu de la Vie");

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
        text.setPosition(10, grille.getDim1() * pixelSize); // Positionne le texte sous la grille

        int currentIteration = 1; // Initialise le compteur d'itérations
        bool spacePressed = false; // Indique si la barre espace est appuyée
        auto lastUpdate = chrono::steady_clock::now(); // Enregistre l'instant de la dernière mise à jour

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
};

// **Main**
int main() {
    string filename; // Chemin du fichier d'entrée
    int iterations; // Nombre d'itérations souhaitées
    int pixelSize; // Taille des pixels pour l'affichage graphique

    // Demande les paramètres à l'utilisateur
    cout << "Entrez le chemin du fichier d'entree : ";
    cin >> filename;
    cout << "Entrez le nombre d'iterations : ";
    cin >> iterations;
    cout << "Entrez la taille des pixels (ex : 20) : ";
    cin >> pixelSize;

    Simulation simulation(filename, iterations, pixelSize); // Crée une instance de Simulation
    simulation.run(); // Lance la simulation

    return 0; // Retourne 0 pour indiquer que le programme s'est terminé correctement
}

