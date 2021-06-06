#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <utility>
#include <random>
#include <fstream>
#include <iomanip>
#include <sstream>

#define NUM_FOILS 10000

int main() {
    // NACA 3-parameter profile 3xNUM_FOILS, initialized to 0
    //std::vector<std::vector<double> > profiles( 3 , std::vector<double> (NUM_FOILS, 0)); 
    double profiles[NUM_FOILS][3];
    std::vector<double> airV(NUM_FOILS); // air velocities
    std::vector<double> angle(NUM_FOILS); // angle of attack

    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis1(2,8); // distribution for 1st param
    std::uniform_real_distribution<> dis2(2,8); // distribution for 2nd param
    std::uniform_real_distribution<> dis3(10,35); // distribution for 3rd param
    std::uniform_real_distribution<> disAirV(0,99); // distribution for air velocity
    std::uniform_real_distribution<> disAngle(0,99); // distribution for angle of attack


    for(int i=0; i < NUM_FOILS; i++) {
        profiles[i][0] = dis1(gen);
        profiles[i][1] = dis2(gen);
        profiles[i][2] = dis3(gen);
    }

    for (int i=0; i < NUM_FOILS; i++) {
        airV[i] = disAirV(gen);
    }

    for (int i=0; i < NUM_FOILS; i++) {
        angle[i] = disAngle(gen);
    }

    // add data to a string buffer, then write out the buffer
    std::ostringstream stringStream;
    stringStream << std::fixed;
    stringStream << std::setprecision(16);
    for (int i=0; i < NUM_FOILS; i++) {
       stringStream <<  profiles[i][0];
       stringStream <<  ",";
       stringStream <<  profiles[i][1];
       stringStream <<  ",";
       stringStream <<  profiles[i][2];
       stringStream <<  ",";
       stringStream <<  airV[i];
       stringStream <<  ",";
       stringStream <<  angle[i];
       stringStream <<  "\n";
    }
    
    std::ofstream outputFile;
    outputFile.open("foilData.csv");
    outputFile << stringStream.str();
    outputFile.close();
}
















