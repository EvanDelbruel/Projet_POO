#include <iostream>
#include <vector>
#include <memory>
#include<fstream>

using namespace std;

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

class Grille:public Cellule{
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
        /*void setCell(int x, int y, bool etat) {
        if (x >= 0 && x < dim1 && y >= 0 && y < dim2) {
            grille[x][y].setEtat(etat);
        }
    }*/

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
    void run(const string& nomF,int iteration){
        ficher(nomF);
        for (int i = 0; i < iteration; i++) {
        afficherGrille();
        updateGrille();
    }

    }
};


int main() {
    string filename;//"C:\\Users\\hp\\Desktop\\CESI\\Prosit\\POO\\Projet POO\\Test.txt";
    cout << "Entrez le chemin du fichier d'entrée : ";
    cin >> filename;
    int iteration;
    cout << "Entrez le nombre d'iteration ";
    cin >> iteration;
    Grille g;
    g.run(filename,iteration);

    return 0;
}

