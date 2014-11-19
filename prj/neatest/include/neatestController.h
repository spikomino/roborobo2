/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#ifndef NEATESTCONTROLLER_H
#define NEATESTCONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "neatest/include/neatestAgentObserver.h"
#include <neat/gene.h>
#include <neat/genome.h>
#include <neat/genomeAdapted.h>

#include <iomanip>

using namespace NEAT;
using namespace std;

typedef tuple<GenomeAdapted*, double, double, int> message;

class neatestController:public Controller {

private:
    int                     _iteration;
    int                     _birthdate; // iteration at which we are created 
    double                  _fitness;
    double                  _prev_fitness; // to report to World observer
    unsigned int            _items; // items collected 
    double                  _sigma;
    GenomeAdapted*          _genome;
    Network*                _neurocontroller;
    unsigned int            _nbInputs;
    unsigned int            _nbOutputs;
    
    std::map<int, message>  _glist;

   
    /* behavior */ 
    void initRobot     ();
    void stepBehaviour ();

    /* genome list */
    void broadcast       ();
    void storeMessage    (int, message);  
    void emptyGenomeList ();

    /* neuro controller */
    void createNeuroController ();

    /* vvolution */
    void stepEvolution ();
    int  selectRandom  ();
    int  selectBest    ();
    void updateFitness ();
    void emptyBasket   ();
    /* misc */
  
    void printRobot      ();
    void printGenomeList ();
    void printMessage    (message);
    void save_genome     ();
    void printAll        ();
    

public:

    neatestController  (RobotWorldModel * wm);
    ~neatestController ();
    
    int getId  (){ return _wm->getId(); }
    double getFitness  (){ return _prev_fitness; }

    void reset ();
    void step  ();
    void pickItem      ();
};


#endif
