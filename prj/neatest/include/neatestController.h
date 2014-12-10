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
    int            _iteration;
    int            _birthdate; // iteration at which we are created 

    double         _fitness;
    double         _reported_fitness; // to report to World observer
    int            _reported_popsize;
    
    int            _items_collected; // items collected 
    int            _items_forraged;  // items brought back to the nest
    int            _items_max; // basket capacity 
    

    double                  _sigma;
    GenomeAdapted*          _genome;
    Network*                _neurocontroller;
    
    unsigned int            _nbInputs;
    unsigned int            _nbOutputs;
    
    std::map<int, message>  _glist; // genome list (local population)
   
   
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
    int  select        (double sp);

    void updateFitnessForaging   ();
    void updateFitnessNavigation ();
    void updateFitnessCollecting ();
    void emptyBasket   ();

    void dropItem(int n);

    /* misc */
  
    void printRobot      ();
    void printGenomeList ();
    void printMessage    (message);
    void printPopsize    ();
    void save_genome     ();
    void printAll        ();
    

public:

    neatestController  (RobotWorldModel * wm);
    ~neatestController ();
    void reset         ();
    void step          ();
    
    int          getId             () { return _wm->getId(); }
    double       getFitness        () { return _reported_fitness; }
    double       getPopsize        () { return _reported_popsize; }
    bool         stillRoomInBasket () ;
    int ItemsPicked       () { return _items_collected; }
    void         pickItem          ();

};


#endif
