#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <memory>
#include <chrono> // Pour limiter la fréquence d'exécution

using namespace std;
using namespace chrono;

// Structure représentant une position (x, y) dans la grille
struct Position {
    int x, y;

    // Opérateur de comparaison pour les ensembles
    bool operator<(const Position& other) const {
        return tie(x, y) < tie(other.x, other.y);
    }

    // Opérateur d'égalité pour les comparaisons directes
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Classe abstraite pour les règles
class Regle {
protected:
    bool etat;
    int voisin;
public:
    virtual bool application(bool etat, int voisin) const = 0;
};

// Classe pour les règles standard du jeu de la vie
class RegleSt : public Regle {
public:
    bool application(bool etat, int voisinV) const override {
        if (etat && (voisinV == 2 || voisinV == 3)) {
            return true;
        } else if (!etat && voisinV == 3) {
            return true;
        } else {
            return false;
        }
    }
};

// Classe principale pour la grille
class Grille : public RegleSt {
private:
    int dim1, dim2;
    vector<vector<bool>> grille;
    set<Position> cellulesVivantes;

    void notifierVoisines(Position pos, map<Position, int>& voisins) {
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue; // Ignore la cellule elle-même

                int nx = (pos.x + dx + dim1) % dim1; // Gestion torique
                int ny = (pos.y + dy + dim2) % dim2;

                Position voisine = {nx, ny};
                voisins[voisine]++;
            }
        }
    }

public:
    Grille(int dim1, int dim2) : dim1(dim1), dim2(dim2), grille(dim1, vector<bool>(dim2, false)) {}

    void ficher(const std::string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Impossible d'ouvrir le fichier !" << endl;
            exit(1);
        }

        file >> dim1 >> dim2;
        grille.resize(dim1, vector<bool>(dim2, false));

        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                int state;
                file >> state;
                grille[i][j] = (state == 1);
                if (grille[i][j]) {
                    cellulesVivantes.insert({i, j});
                }
            }
        }
        file.close();
    }

    bool updateGrille() {
        map<Position, int> voisins;

        for (const auto& pos : cellulesVivantes) {
            notifierVoisines(pos, voisins);
        }

        set<Position> nouvellesCellulesVivantes;

        for (const auto& [pos, count] : voisins) {
            bool estVivante = grille[pos.x][pos.y];
            bool nouveauEtat = application(estVivante, count);
            if (nouveauEtat) {
                nouvellesCellulesVivantes.insert(pos);
            }
        }

        // Vérifier si la grille reste identique (stabilité)
        if (cellulesVivantes == nouvellesCellulesVivantes) {
            return false; // La grille est stable
        }

        // Mettre à jour la grille et les cellules vivantes
        grille.assign(dim1, vector<bool>(dim2, false));
        for (const auto& pos : nouvellesCellulesVivantes) {
            grille[pos.x][pos.y] = true;
        }
        cellulesVivantes = nouvellesCellulesVivantes;

        return true; // La grille a changé
    }

    void afficherGraphique(sf::RenderWindow& window, sf::Text& text, int pixelSize, int iteration) {
        window.clear();

        text.setString("Iteration : " + to_string(iteration));

        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                if (grille[i][j]) {
                    sf::RectangleShape cell(sf::Vector2f(pixelSize - 1, pixelSize - 1));
                    cell.setPosition(j * pixelSize, i * pixelSize);
                    cell.setFillColor(sf::Color::Green);
                    window.draw(cell);
                }
            }
        }

        window.draw(text);
        window.display();
    }

    int getDim1() const { return dim1; }
    int getDim2() const { return dim2; }
};

int main() {
    string filename;
    cout << "Entrez le chemin du fichier d'entree : ";
    cin >> filename;

    int iterations;
    cout << "Entrez le nombre d'iterations : ";
    cin >> iterations;

    int pixelSize;
    cout << "Entrez la taille des pixels (ex : 20) : ";
    cin >> pixelSize;

    Grille g(0, 0);
    g.ficher(filename);

    int windowWidth = g.getDim2() * pixelSize;
    int windowHeight = g.getDim1() * pixelSize + 50;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Jeu de la Vie");

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        cerr << "Erreur : Impossible de charger la police (DejaVuSans.ttf)" << endl;
        return 1;
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setPosition(10, windowHeight - 40);

    int currentIteration = 0;
    bool spacePressed = false;
    auto lastUpdate = chrono::steady_clock::now();

    while (window.isOpen() && currentIteration < iterations) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                spacePressed = true;
            }

            if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space) {
                spacePressed = false;
            }
        }

        auto now = chrono::steady_clock::now();

        // Une pression unique
        if (spacePressed && chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 150) {
            if (currentIteration < iterations) {
                currentIteration++;
                if (!g.updateGrille()) {
                    cout << "Simulation terminee : Stabilite atteinte a l'iteration " << currentIteration << endl;
                    break;
                }
                g.afficherGraphique(window, text, pixelSize, currentIteration);
                lastUpdate = now; // Mise à jour du délai
            }
        }
    }

    cout << "Simulation terminee !" << endl;
    return 0;
}
