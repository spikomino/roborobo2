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
#include <list> 
#include <iomanip>

using namespace NEAT;
using namespace std;

typedef tuple<GenomeAdapted*, double, double, int> message;

class neatestController:public Controller {

private:
    int            _iteration;
    int            _birthdate; // iteration at which we are created 

    double         _fitness;

    double         _reported_fitness; // to report to the World observer
    int            _reported_popsize;
    int            _reported_missed;
    int            _reported_collected;
    int            _reported_forraged;
    int            _reported_basket;
    double         _reported_basket_usage;

    double         _locomotion;        // accumulates the locomotion fitness
    int            _items_collected;   // items collected 
    int            _items_forraged;    // items brought back to the nest
    int            _items_miss_droped; // item dropeed outside of nest
    double         _basket_usage;      // basket usage accumulator

    //   int            _items_in_basket;
    std::list<int> _basket;
    unsigned int   _items_max;        // basket capacity 
    
   

    double                  _sigma;
    GenomeAdapted*          _genome;
    Network*                _neurocontroller;
    
     int            _nbInputs;
     int            _nbOutputs;
    
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

    /* evolution */
    void stepEvolution ();
    int  selectRandom  ();
    int  selectBest    ();
    int  select        (double sp);

    void emptyBasket   ();
    void dropItem      (int n, bool at_nest);

    /* misc */
  
    void printRobot      ();
    void printGenomeList ();
    void printMessage    (message);
    void printPopsize    ();
    void printBasket     ();
    void save_genome     ();
    void printAll        ();
    bool matured         ();

public:

    neatestController  (RobotWorldModel * wm);
    ~neatestController ();
    void reset         ();
    void step          ();
    
    int          getId             () { return _wm->getId(); }
    double       getFitness        () { return _reported_fitness; }
    double       getPopsize        () { return _reported_popsize; }
    double       getBasketUsage    () { return _reported_basket_usage; }
    int          getMisseDroped    () { return _reported_missed; }
    int          getCollected      () { return _reported_collected; }
    int          getForraged       () { return _reported_forraged; }
    int          getBasketSize     () { return _reported_basket; }
    

    bool         stillRoomInBasket () ;
    void         pickItem          (int);
    std::list<int> getBasket(){ return _basket ; }
};


#endif
