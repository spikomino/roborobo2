#ifndef ESMEDEACONTROLLER_H
#define ESMEDEACONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"

#include "Controllers/Controller.h"

#include "WorldModels/RobotWorldModel.h"

#include "neuralnetworks/NeuralNetwork.h"

#include <list> 

typedef std::vector<double> genome;
typedef std::tuple<genome, double> message;

class esMedeaController : public Controller {
    
private:
    
    
    
    int    _iteration;
    int    _generation;

    /* genome */
    int    _birthdate; // evaluation when this controller was initialized.
    double _fitness;
    double _sigma; 
    
    double _reported_fitness;
    double _reported_sigma; 
    
    std::vector<double>     _genome;
    std::map<int, message>  _population;
    void emptyPopulation();


    /* neurocontroller */
    std::string      _nnType;
    std::vector<int> _nbHiddenNeuronsPerLayer;
    std::vector<int> _nbBiaisNeuronsPerLayer;
    Neural::NeuralNetwork* _nn;
    
    double _minValue;
    double _maxValue;
    unsigned int _nbInputs;
    unsigned int _nbOutputs;
    unsigned int _nbHiddenLayers;
    std::vector<unsigned int>* _nbNeuronsPerHiddenLayer;
    
    std::vector<double> *_sensors;
    std::vector<double>  _outputs;

    

    /* effectors and sensors */
    std::list<int> _basket;
    unsigned int   _items_max;        // basket capacity 
    


    /* various counters for diferent fitness measures */
    
    double _locomotion;        // accumulates the locomotion fitness
    int    _items_collected;   // items collected 
    int    _items_forraged;    // items brought back to the nest
    int    _items_miss_droped; // item dropeed outside of nest
    int    _items_forraged_at_landmark ;
    double _basket_usage;      // basket usage accumulator
  
    double _reported_locomotion;       
    int    _reported_items_collected;  
    int    _reported_items_forraged;   
    int    _reported_items_miss_droped;
    int    _reported_items_forraged_at_landmark ;
    double _reported_basket_usage;
    
    

    
    void init  (); 

    void createController();
    void saveController();

    void stepEvolution();
    int  select(double);


    void stepBehaviour();
    void readInputs();
    void writeOutput();

    bool matured();
    void broadcast();
    void storeGenome(int, message);
    void emptyGenomeList();


    void   updateFitness();
    double updateFitnessLocomotion();
    double updateFitnessCollection();
    double updateFitnessForraging();

    void updateMeasures();
    void resetMeasures();
    
    void randomGenome();

    void pickItem(int);
    void dropItem(int, bool);
    void emptyBasket();
    bool stillRoomInBasket();

    bool isEnergyItem(int);
    int  getLifetime();
    bool lifeTimeOver();
   
    void reset(); /* called every generation */
    
    void printRobot();

public:
    
    esMedeaController   ( RobotWorldModel *__wm );
    ~esMedeaController  ();
		
   
    
    void step  (); /* called every iteration */
    

    int          getId             () { return _wm->getId(); }




};


#endif

