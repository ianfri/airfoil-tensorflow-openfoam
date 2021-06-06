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

#include <gmsh.h>

#define NUM_FOILS 1000
#define NUM_POINTS 120
#define MRES 100 // mesh resolution

#define dbg std::cout << "<<DEBUG>> "

double y_t(double x, double t) {
    return 5 * t * (0.2969*sqrt(x) - 0.1260*x - 0.3516*x*x + 0.2843*x*x*x - 0.1015*x*x*x*x);
}

double y_c( double x, double m, double p, double c) {
    if (x >= 0 && x <= p*c) {
        return (m/(p*p)) * ( 2*p * (x/c) - (x/c)*(x/c));
    } else if (x >= p*c && x <= c) {
        return (m / ((1-p)*(1-p))) * ( (1-2*p) + 2*p*(x/c) - (x/c)*(x/c) );
    } else { // empirically from GDB, this happens when x is 0.000000000007 greater than c
        // so, just act the same as the second if statement
        //exit(3);
        return (m / ((1-p)*(1-p))) * ( (1-2*p) + 2*p*(x/c) - (x/c)*(x/c) );
    }
}

double dyc_dx(double x, double m, double p, double c) {
    if (x >= 0 && x <= p*c) {
        return ((2*m)/(p*p)) * ( p - (x/c) );
    } else if (x >= p*c && x <= c) {
        return (2*m/((1-p)*(1-p))) * ( p - (x/c) );
    } else { // empirically from GDB, this happens when x is 0.000000000007 greater than c
        // so, just act the same as the second if statement
        //exit(3);
        return (2*m/((1-p)*(1-p))) * ( p - (x/c) );
    }
}

int main() {
    double profiles[10000][3];

    // read in the foil data
    std::ifstream data;
    data.open("foilData.csv");
    std::string line, word;
    std::vector<std::string> row = {""};

    int i=0;
    //while (data >> unusedString) {
    while ( std::getline(data, line) ) {
        row.clear();
        std::istringstream s(line);
        while (std::getline(s, word, ',')) { 
            row.push_back(word); 
        }
        
        profiles[i][0] = std::stod( row.at(0) );
        profiles[i][1] = std::stod( row.at(1) );
        profiles[i][2] = std::stod( row.at(2) );
        i++;
    }

    /* Generate the points (x,y) for the foils */
    // try doing just one foil first, add loop after

    // populate vector of equally spaced x values ranging from 0 to 1
    std::vector<double> xCoords(NUM_POINTS);
    double temp = 0.0;
    double addThisFront = (1.0/16) / (NUM_POINTS/3); // use the first 1/3 of the points in the first 1/16 of the geom
    double addThisBack = (15.0/16) / ((NUM_POINTS/3)*2);
    for (int i = 0; i < xCoords.size(); i++) {
        if (i < xCoords.size() / 3) { // for the front 1/16 of geometry, defined above
            xCoords.at(i) = temp + addThisFront;
            temp += addThisFront;
        } else { // back 7/8
            xCoords.at(i) = temp + addThisBack;
            temp += addThisBack;
        }
    }

    std::cout << "made xCoords" << std::endl;

    /* START OF OPERATIONS FOR EACH GEOMETRY */

    for(int foilIndex=652; foilIndex < NUM_FOILS; foilIndex++) {
        dbg << "foilIndex: " << foilIndex << std::endl;
        // find the values of m, p, c
        double m = profiles[foilIndex][0] / 100.0;
        double p = profiles[foilIndex][1] / 10.0;
        double t = profiles[foilIndex][2] / 100.0;
        double c = 1.0;

        // corrected values of x and y
        std::vector<double> x_u( xCoords.size() );
        std::vector<double> x_l( xCoords.size() );
        std::vector<double> y_u( xCoords.size() );
        std::vector<double> y_l( xCoords.size() );

        // find corrected values for x_u
        dbg << "CALCULATE X_U" << std::endl;
        for (int i=0; i < x_u.size(); i++) {
            dbg << "Index: " << i << std::endl;
            x_u[i] = xCoords[i] - y_t(xCoords[i], t) * sin( atan( dyc_dx(xCoords[i], m, p, c) ) );
            dbg << x_u[i] << std::endl;
        }

        // find corrected values for y_u
        dbg << "CALCULATE Y_U" << std::endl;
        for (int i=0; i < x_u.size(); i++) {
            dbg << "Index: " << i << std::endl;
            y_u[i] = y_c(xCoords[i], m, p, c) + y_t(xCoords[i], t) * cos( atan( dyc_dx(xCoords[i], m, p, c) ) );
            dbg << y_u[i] << std::endl;
        }

        // find corrected values for x_l
        dbg << "CALCULATE X_L" << std::endl;
        for (int i=0; i < x_u.size(); i++) {
            dbg << "Index: " << i << std::endl;
            x_l[i] = xCoords[i] + y_t(xCoords[i], t) * sin( atan( dyc_dx(xCoords[i], m, p, c) ) );
            dbg << x_l[i] << std::endl;
        }

        // find corrected values for y_l
        dbg << "CALCULATE X_U" << std::endl;
        for (int i=0; i < x_u.size(); i++) {
            dbg << "Index: " << i << std::endl;
            y_l[i] = y_c(xCoords[i], m, p, c) - y_t(xCoords[i], t) * cos( atan( dyc_dx(xCoords[i], m, p, c) ) );
            dbg << y_l[i] << std::endl;
        }


        /************** Begin gmsh ****************/
        /*
        * Handle all of the tag assignments automatically... store in vectors of tags
        * We are assuming that the vectors hold consecutive points, lines, etc
        */
        std::vector<int> p_tags, l_tags, cl_tags; // point, line, curveLoop
        gmsh::initialize();
        gmsh::model::add("foil");

        // add upper points
        for (int i=0; i < x_u.size(); i++) {
            int tmp_p = gmsh::model::geo::addPoint(x_u[i], y_u[i], 0, MRES);
            p_tags.push_back(tmp_p);
        }

        // add lower points
        // NB: add points in reverse because drawing model clockwise
        for (int i = x_l.size() - 1; i > -1; i--) {
            int tmp_p = gmsh::model::geo::addPoint(x_l[i], y_l[i], 0, MRES);
            p_tags.push_back(tmp_p);
        }

        // connect the dots with lines
        for (int i=0; i < p_tags.size() - 1; i++) {
            int tmp_l = gmsh::model::geo::addLine(p_tags[i], p_tags[i+1]);
            l_tags.push_back(tmp_l);
        }
        // connect the final dot with the initial one
        int tmp_l = gmsh::model::geo::addLine(p_tags[p_tags.size() - 1], p_tags[0]);
        l_tags.push_back(tmp_l);

        // add the curve loop, passing in the line tag vector
        int tmp_cl = gmsh::model::geo::addCurveLoop(l_tags);
        cl_tags.push_back(tmp_cl);

        // add the plane surface
        int tmp_ps = gmsh::model::geo::addPlaneSurface(cl_tags);

        std::vector<std::pair<int, int> > modelDimTags;
        gmsh::model::getEntities(modelDimTags);

        std::vector<std::vector<std::size_t> > elementTags;
        std::vector<std::vector<std::size_t> > nodeTags;

        // need to pick out only the plane surface
        std::vector<std::pair<int, int> > outDimTags;
        
        gmsh::model::geo::extrude({{2,tmp_ps}},
                                    0,0,1,
                                    outDimTags
                                    );
        std::cout << "Done extruding" << std::endl;
    
        gmsh::model::geo::synchronize();
        gmsh::model::mesh::generate(3);
        std::cout << "Done generating 3D" << std::endl;
        gmsh::model::mesh::optimize("Netgen");
        std::cout << "Done optimizing with netgen" << std::endl;
        std::string fileName = "/mnt/c/Users/444in/Documents/xpsProgramming/r_nn_phy/foilGeoms/" + std::to_string(foilIndex) + ".stl";
        gmsh::write(fileName);
        std::cout << "<<WRITE>> " << fileName << std::endl;
    }
}