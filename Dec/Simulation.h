#ifndef SIMULATION_H
#define SIMULATION_H

#include <SFML/Graphics.hpp>
#include "Grille.h"
#include <string>
class Simulation {
private:
    Grille grille;
    std::string folderPath;
    int iterations;
    int pixelSize;

public:
    Simulation(const std::string& filename, int iterations, int pixelSize);
    std::string createSimulationFolder(const std::string& filename);
    void run();
};

#endif // SIMULATION_H

