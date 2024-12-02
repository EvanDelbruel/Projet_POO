#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

using namespace std;

class Regle {
protected:
    bool etat;
    int voisin;
public:
    virtual bool application(bool etat, int voisin) const = 0;
};

class RegleSt : public Regle {
public:
    bool application(bool etat, int voisinV) const override {
        if (etat && (voisinV == 2 || voisinV == 3)) {
            return true;
        }
        else if (!etat && voisinV == 3) {
            return true;
        }
        else {
            return false;
        }
    }
};

class Cellule {
private:
    bool etat;
    int voisin;
    shared_ptr<Regle> regle;
public:
    Cellule() : etat(false), voisin(0), regle(make_shared<RegleSt>()) {}
    bool getEtat() const { return etat; }
    void setEtat(bool N_etat) { this->etat = N_etat; }
    void setVoisin(int N_voisin) { this->voisin = N_voisin; }
    void NextEtat() { etat = regle->application(etat, voisin); }
};

class Grille : public Cellule {
private:
    int dim1, dim2;
    vector<vector<Cellule>> grille;

    int voisinVi(int x, int y) {
        int Vivant = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                int nx = x + i, ny = y + j;
                if (nx >= 0 && nx < dim1 && ny >= 0 && ny < dim2 && grille[nx][ny].getEtat()) {
                    Vivant++;
                }
            }
        }
        return Vivant;
    }

public:
    Grille() : dim1(0), dim2(0) {}
    Grille(int dim1, int dim2) : dim1(dim1), dim2(dim2), grille(dim1, vector<Cellule>(dim2)) {}

    void ficher(const std::string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Impossible d'ouvrir le fichier ! Vous devez en créer un en format .txt " << endl;
            exit(1);
        }

        file >> dim1 >> dim2;
        grille.resize(dim1, vector<Cellule>(dim2));

        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                int state;
                file >> state;
                grille[i][j].setEtat(state == 1);
            }
        }
        file.close();
    }

    void updateGrille() {
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                grille[i][j].setVoisin(voisinVi(i, j));
            }
        }
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                grille[i][j].NextEtat();
            }
        }
    }

    void afficherGraphique(sf::RenderWindow& window, int cellSize) {
        window.clear();
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                sf::RectangleShape cell(sf::Vector2f(cellSize - 1, cellSize - 1));
                cell.setPosition(j * cellSize, i * cellSize);
                cell.setFillColor(grille[i][j].getEtat() ? sf::Color::Green : sf::Color::Black);
                window.draw(cell);
            }
        }
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

    Grille g;
    g.ficher(filename);

    const int cellSize = 20;
    sf::RenderWindow window(sf::VideoMode(g.getDim2() * cellSize, g.getDim1() * cellSize), "Jeu de la Vie");

    for (int i = 0; i < iterations; i++) {
        g.updateGrille();
        g.afficherGraphique(window, cellSize);

        // Pause de 10 secondes entre les itérations
        sf::sleep(sf::seconds(1));
    }

    return 0;
}
