#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;
class Regle{
protected:
    bool etat;
    int voisin;
public:
    virtual bool application(bool etat,int voisin) const =0;
};
class RegleSt:public Regle{
public:
    bool application(bool etat,int voisinV)const{
    if (etat==true && (voisinV ==2 ||voisinV ==3)){
        return etat = true ;
    }
    else if(etat ==false &&(voisinV==3)){
        return etat =true ;
    }
    else
        return etat =false ;
    }

};
class Cellule{
private:
    bool etat;
    int voisin;
    shared_ptr<Regle> regle;
public:
    Cellule() : etat(false), voisin(0), regle(std::make_shared<RegleSt>()){}
    bool getEtat(){
    return etat;}
    int getVoisin(){
    return voisin;}
    void setEtat(bool N_etat){
        this->etat = N_etat;}
    void setVoisin(int N_voisin){
        this->voisin =N_voisin;
    }
    void NextEtat(){
    etat=regle->application(etat,voisin);}
};

class Grille {
private:
    int dim1;
    int dim2;
    vector<std::vector<Cellule>> grille;
    int voisinVi(int x,int y){
        int Vivant=0;
        int i=0;
        int j=0;
        for(i=-1;i<=1;i++){
            for(j=-1;j<=1;j++){
                if(i==0 && j==0)continue;
                int nx=x+i;
                int ny=y+j;
                if (nx >= 0 && nx < dim1 && ny >= 0 && ny < dim2){
                    if(grille[nx][ny].getEtat()==true){
                        Vivant++;
                    }
                }

            }

        }
        return Vivant;

    }
public:
    Grille():dim1(0),dim2(0),grille(dim1,(vector<Cellule>(dim2,Cellule()))){}
    Grille(int dim1,int dim2):dim1(dim1),dim2(dim2),grille(dim1,(vector<Cellule>(dim2,Cellule()))){}
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
    bool estIdentique( vector<vector<Cellule>>& autreGrille) {
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            if (grille[i][j].getEtat() != autreGrille[i][j].getEtat()) {
                return false; 
            }
        }
    }
    return true;
}



    void afficherGrille() {
        for (int i = 0; i < dim1; i++) {
            for (int j = 0; j < dim2; j++) {
                if(grille[i][j].getEtat()==true){
                    cout<<"1"<<" ";
                }
                else
                    cout<<"0"<<" ";
            }
            cout << endl;
        }
        cout << string(20, '=') << endl;
    }
    void Graph(sf::RenderWindow& window) {
    window.clear(sf::Color::Black); 

    sf::RectangleShape cell(sf::Vector2f(20 - 1.0f, 20 - 1.0f)); 

    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            if (grille[i][j].getEtat() == true) {
                cell.setFillColor(sf::Color::White); 
            } else {
                cell.setFillColor(sf::Color::Black);
            }
            cell.setPosition(j * 20, i * 20);
            window.draw(cell);
        }
    }

    window.display(); 
}
    
   void ecrireDansFichier(const string& filename, int iteration) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Impossible d'ouvrir le fichier de sortie !" << endl;
            exit(1);
        }
        file << "Iteration: " << iteration << "\n";
        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j < dim2; ++j) {
                file << (grille[i][j].getEtat() ? "1" : "0") << " ";
            }
            file << "\n";
        }
        file.close();
    } 
    void ficher(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            cout<<"Impossible d'ouvrir le fichier !"<<endl;
            exit(1);
        }

        file >> dim1 >> dim2;
        grille.resize(dim1,(vector<Cellule>(dim2,Cellule())));

        for (int i = 0; i < dim1; ++i) {
            for (int j = 0; j <dim2; ++j) {
                int state;
                file >> state;
                grille[i][j].setEtat((state == 1));
            }
        }
        file.close();
    }
    void run(const string& nomF, int iterations) {
        ficher(nomF);
        string outputDir = nomF + "_out";
        fs::create_directory(outputDir);

        sf::RenderWindow window(sf::VideoMode(dim2 * 20, dim1 * 20), "Jeu de la Vie");
        int currentIteration = 0;
        vector<vector<Cellule>> grillePrecedente = grille; 

        while (currentIteration < iterations && window.isOpen()) {
            sf::Event event;
            bool spacePressed = false;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                    spacePressed = true;
                }
            }

            if (spacePressed) {
                string iterationFilename = outputDir + "/iteration_" + to_string(currentIteration) + ".txt";
                ecrireDansFichier(iterationFilename, currentIteration);
                Graph(window);
                updateGrille();
                if (estIdentique(grillePrecedente)) {
                    ecrireDansFichier(iterationFilename, currentIteration);
                    cout << "La simulation s'arrête : plus d'évolution détectée." << endl;
                    break;
                }
                grillePrecedente = grille;
                ++currentIteration;
            }
        }
    }
};



int main() {
    string filename;
    cout << "Entrez le chemin du fichier d'entrée : ";
    cin >> filename;
    int iteration;
    cout << "Entrez le nombre d'iteration :";
    cin >> iteration;
    Grille g;
    g.run(filename,iteration);

    return 0;
}

