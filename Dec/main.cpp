#include "Simulation.h"
#include <iostream>
#include <string>
using namespace std ;
int main() {
    string filename; // NOM du fichier d'entrée
    int iterations; // Nombre d'itérations souhaitées
    int pixelSize; // Taille des pixels pour l'affichage graphique

    // Demande les paramètres à l'utilisateur
    cout << "Entrez le NOM du fichier d'entree : ";
    cin >> filename;
    cout << "Entrez le nombre d'iterations : ";
    cin >> iterations;
    cout << "Entrez la taille des pixels (ex : 20) : ";
    cin >> pixelSize;

    Simulation simulation(filename, iterations, pixelSize); // Crée une instance de Simulation
    simulation.run(); // Lance la simulation

    return 0; // Retourne 0 pour indiquer que le programme s'est terminé correctement
}
