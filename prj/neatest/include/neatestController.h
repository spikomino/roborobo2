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
    GenomeAdapted*          _genome;
    double                  _fitness;
    double                  _sigma;
    Network*                _neurocontroller;
    unsigned int            _nbInputs;
    unsigned int            _nbOutputs;
    std::map<int, message>  _glist;
  

    //NOTE: NEAT-like innovation number and number of nodes FOR THIS ROBOT
    double innovNumber;
    int nodeId;
   
    /* behavior */ 
    void initRobot     ();
    void stepBehaviour ();

    /* genome list */
    void broadcast       ();
    void storeGenome     (int, message);  
    void emptyGenomeList ();

    /* neuro controller */
    void createNeuroController ();

    /* vvolution */
    void updateFitness (double); 
    void stepEvolution ();
    int  selectRandom  ();
    int  selectBest    ();
    void mutate        (float sigma);

    /* misc */
    bool lifeTimeOver    ();
    void printRobot      ();
    void printGenomeList ();
    void printMessage    (message);
    void save_genome     ();
    void printAll        ();

public:

    neatestController  (RobotWorldModel * wm);
    ~neatestController ();
    
    int getId  (){ return _wm->getId(); }
    void reset ();
    void step  ();
    double getFitness  (){ return _fitness; }
};


#endif
