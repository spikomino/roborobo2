#include "esMedea/include/esMedeaController.h"
#include "esMedea/include/esMedeaSharedData.h"
#include "esMedea/include/esMedeaWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <string>

#include <neuralnetworks/NeuralNetwork.h>
#include <neuralnetworks/MLP.h>
#include <neuralnetworks/Perceptron.h>
#include <neuralnetworks/Elman.h>

using namespace Neural;




esMedeaController::esMedeaController( RobotWorldModel * __wm ) {
    
    _wm  = __wm;
    _nn  = NULL;

    _iteration  = 0;
    _generation = 0;
    _wm->setAlive(true);
    _wm->setRobotLED_colorValues(255, 0, 0);
    _wm->_desiredTranslationalValue = 0.0; 
    _wm->_desiredRotationalVelocity = 0.0; 

    if ( gEnergyLevel ) _wm->setEnergyLevel(gEnergyInit);
   

    init();

}

esMedeaController::~esMedeaController(){
	// nothing to do.
}

void esMedeaController::reset(){
    updateMeasures();
    resetMeasures();
    emptyPopulation();
}

void esMedeaController::step(){
    _iteration++;
      
    stepBehaviour(); 

    if( lifeTimeOver() ) {
	if( esMedeaSharedData::gExtendedVerbose )
	    printRobot();
        
	if( !esMedeaSharedData::gControllersDirectory.empty() )
	    saveController();
	
	stepEvolution();
	reset();
    }
}


/*
 * Step the neuro controller and execute command
 *
 */
void esMedeaController::stepBehaviour(){
    _wm->_desiredTranslationalValue = 0.0; 
    _wm->_desiredRotationalVelocity = 0.0; 


    if( _wm->isAlive() ) {
	readInputs();
	writeOutput();
	updateFitness();
	broadcast();
    }

    if( esMedeaSharedData::gExtendedVerbose ){
	printGenome();
	printSensors();
	printOutputs();
	printBasket();
        printPopulation();
    }
}

void esMedeaController::stepEvolution(){

    /* if update _genome 
       
       if genomes in the list  
          set alive
          select one from the list
          mutate it.
       else 
          die

       create a controller	  
       
     */

    if (_population.size()>0){
	_wm->setAlive(true);

	/* select an offspring */
	int selected = select(esMedeaSharedData::gSelectionPressure);
	_genome.clear();
	_genome = std::get<0>(_population[selected]);
	
	/*mutate*/

	createController();

	_birthdate = _iteration;
	_generation++;
    }
    else
	_wm->setAlive(false);
    
}


int esMedeaController::select(double sp){
    /* the size of the tournament */
    int inspected = sp * (double) _population.size();
   
    /* shuffle indexes */
    std::vector<int> v;
    for(auto i: _population)
	v.push_back(i.first);
    std::random_shuffle(v.begin(), v.end());
    
    /* get the best from the inspected */
    double max_fit =  std::get<1>(_population[v[0]]);
    int    best_g  =  v[0];
    int    j=1; /* index in v */
    for (int i=1 ; i<inspected; i++){
	double f  = std::get<1>(_population[v[j]]);
	if(f > max_fit){
	    max_fit = f;
	    best_g = v[j] ;
	}
	j++;
    } 
    
    /* printGenomeList() ;
    std::cout << "\t\tShuffled :[" << v[0];
    for (int i=1 ; i<inspected ; i++)
	std::cout << ", " << v[i];
    std::cout << "] "  
    << "selected = " << best_g << std::endl; */

    return best_g;
}




void esMedeaController::updateMeasures(){
    _reported_fitness                    = _fitness;
    _reported_sigma                      = _sigma;
    _reported_locomotion                 = _locomotion;   
    _reported_items_collected            = _items_collected;  
    _reported_items_forraged             = _items_forraged;   
    _reported_items_miss_droped          = _items_miss_droped;
    _reported_items_forraged_at_landmark = _items_forraged_at_landmark;
    _reported_basket_size                =  _basket.size();
    _reported_basket_usage               = _basket_usage / 
	(double) esMedeaSharedData::gEvaluationTime;
}

void esMedeaController::resetMeasures(){
    _locomotion                 = 0.0;
    _items_collected            = 0;
    _items_forraged             = 0;
    _items_miss_droped          = 0;
    _items_forraged_at_landmark = 0;
    _basket_usage               = 0.0;  
}


/*
 * Initialize the robot (Things that only need to be done once at the begining)
 * see also resetRobot for thing to do at each generation
 *
 * 1 - compute number of inputs and outputs of the NN depending on the problem
 * 2 - create the sensor vector 
 * 3 - create the NN controller 
 * 4 - create a random genome
 *
 */

void esMedeaController::init(){
    // 1 - setup the number of input and outputs

    // Locomotion
    _nbInputs = _wm->_cameraSensorsNb;	           // proximity sensors

    // Collection 
    if(esMedeaSharedData::gFitnessFunction > 0)   
	_nbInputs += (1) * _wm->_cameraSensorsNb;  // object sensor

    // Foraging
    if(esMedeaSharedData::gFitnessFunction > 1){   
	_nbInputs ++;                              // basket capacity sensor
	_nbInputs += 3;                            // ground sensor 3 inputs
	if(gLandmarks.size() > 0){
	    _nbInputs ++;                          // landmark direction
	    _nbInputs ++;                          // landmark distance
	}
    }

    // Outputs 
    _nbOutputs = 2;                                // motor output
    if (esMedeaSharedData::gFitnessFunction > 1)
	_nbOutputs ++ ;                            // drop items effector 

    // Hidden neurons 
    _nbHiddenLayers = esMedeaSharedData::gNbHiddenLayers;
    _nbNeuronsPerHiddenLayer = new std::vector<unsigned int>(_nbHiddenLayers);
    for(unsigned int i = 0; i < _nbHiddenLayers; i++)
	(*_nbNeuronsPerHiddenLayer)[i] = 
	    esMedeaSharedData::gNbNeuronsPerHiddenLayer;
  
    // 2 - create the sensor vector
    _sensors = new std::vector<double> (_nbInputs);

    // 3 - create neuro controller 
    createController();

    // 4 - create a random genome
    randomGenome();
}





/*
 * Read the sensor values 
 * Read  : 
 * Write : sensors

 */
void esMedeaController::readInputs(){
    // ---- Build inputs ----
    unsigned int inputToUse = 0;
         
    /* read distance sensors  [0 -> 1 ] (1 = touching object)  */
    for(int i = 0; i < _wm->_cameraSensorsNb; i++)
	(*_sensors)[inputToUse++] = 1.0 - (
	    _wm->getDistanceValueFromCameraSensor (i) /
	    _wm->getCameraSensorMaximumDistanceValue (i));              // [0,1]
       
    /* Collecting / Foraging */
    if(esMedeaSharedData::gFitnessFunction > 0)
	
	/* read object sensors */
	for(int i = 0; i < _wm->_cameraSensorsNb; i++){
	    int objectId = _wm->getObjectIdFromCameraSensor(i);
	    
	    /* if physical object, and of correct type */
	    if(PhysicalObject::isInstanceOf(objectId)){
		if(isEnergyItem(objectId))
		    (*_sensors)[inputToUse] = 1.0;                      // {0,1}
		else
		    (*_sensors)[inputToUse] = 0.0;
		inputToUse++;
	    }
	    /* got a physical object but not interesting */
	    else
		(*_sensors)[inputToUse++] = 0.0;
	}
    
    /* Forraging */
    if (esMedeaSharedData::gFitnessFunction > 1){
    
	/* read basket capacity  [0, 1] */
	double activation = (double) _basket.size()/
	    (double) esMedeaSharedData::gBasketCapacity;                // [0,1]
	(*_sensors)[inputToUse++] = activation;

	/* update the basket usage */
	_basket_usage += activation;

	/* ground sensor [0, 1] */
	double r = (double)_wm->getGroundSensor_redValue()  / 255.0;
	double g = (double)_wm->getGroundSensor_greenValue()/ 255.0;
	double b = (double)_wm->getGroundSensor_blueValue() / 255.0;
	
	/* make all the floor as nest depending on experiment */
	if (esMedeaSharedData::gPaintFloor &&  
	    gWorld->getIterations() > esMedeaSharedData::gPaintFloorIteration) {
	    r = 0.0;
	    g = 1.0;
	    b = 0.0;
	}
	(*_sensors)[inputToUse++] = r;                                  // [0,1]
	(*_sensors)[inputToUse++] = g;                                  // [0,1]
	(*_sensors)[inputToUse++] = b;                                  // [0,1]

	/* landmark sensors */
	if(gLandmarks.size() > 0){
	    _wm->updateLandmarkSensor();
	    double landmark_dir=_wm->getLandmarkDirectionAngleValue(); // [-1,1]
 	    double landmark_dis= 1.0 - _wm->getLandmarkDistanceValue(); // [0,1]
	    (*_sensors)[inputToUse++] = landmark_dir;
	    (*_sensors)[inputToUse++] = landmark_dis;
	}
    }
        
    assert (inputToUse == _nbInputs);
}


/*
 * execute the neuro controller 
 * Read  : genome, sensors
 * Write : outputs
 *
 */

void esMedeaController::writeOutput(){
    const  int L = 0; /* left / right motor*/
    const  int R = 1;
    const  int D = 2; /* drop effector */

    /* foraging */
    bool at_nest = false;
    int  droped  = 0;

    /* set up the NN and step  */
    _nn->setWeigths(_genome);
    _nn->setInputs(*_sensors);
    _nn->step();
    _outputs = _nn->readOut();
    
    
    // (4) execute the motor commands 
    
    /* outputs => rot & trans (not used) */ 
    //_wm->_desiredTranslationalValue = outputs[0]; 
    //_wm->_desiredRotationalVelocity = 2.0 * (outputs[1] - 0.5); /* [-1, 1] */

    /* output = L & R velocity (differential drive) */ 
    _outputs[L] = 2.0 * (_outputs[L]-0.5); /* rescale to [-1, 1] */
    _outputs[R] = 2.0 * (_outputs[R]-0.5);
  
    _wm->_desiredTranslationalValue =  (_outputs[R] + _outputs[L]) / 2.0 ;
    _wm->_desiredRotationalVelocity =  _outputs[R] - _outputs[L] ;

    /* normalize to motor interval values */
    _wm->_desiredTranslationalValue =
    	_wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
    	_wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
   
    /* Foraging : drop items */ 
    if (esMedeaSharedData::gFitnessFunction > 1 ){
	droped = (int) (_outputs[D] * _basket.size());
	const int nest_color = 255*256; 
	at_nest = _wm->getGroundSensorValue() == nest_color; 
	
	// make all the floor as nest 
	if (esMedeaSharedData::gPaintFloor && 
	    gWorld->getIterations() > esMedeaSharedData::gPaintFloorIteration) 
	    at_nest=true;

	dropItem(droped, at_nest);
	
	/* std::cout << "Dropped " << droped << "/" << _basket.size() ;   */
	/* if (! at_nest)   */
	/*     std::cout<< " not " ; */
	/* std::cout  << " at nest."  */
	/* << std::endl; */
    }
}





void esMedeaController::broadcast(){

    if( matured() ) {

	/* Make a list of all neighbors within reach  */
	std::vector<esMedeaController *> neighbors;
	for (int i = 0; i < _wm->_cameraSensorsNb; i++) {
	    int targetIndex = _wm->getObjectIdFromCameraSensor (i);
	
	    /* sensor ray bumped into a robot : communication is possible */
	    if (targetIndex >= gRobotIndexStartOffset){
		
		/* convert image registering index into robot id. */
		targetIndex = targetIndex - gRobotIndexStartOffset;
		esMedeaController *targetRobotController =
		    dynamic_cast <
		    esMedeaController *
		    >(gWorld->getRobot (targetIndex)->getController ());
	    
		/* an error occured */ 
		if(!targetRobotController){
		    std:: cerr << "Error from robot "         << _wm->getId () 
			       << " : the observer of robot " << targetIndex 
			       << " is not compatible"        << std::endl;
		    exit (-1);
		}
	    
		/* add to the list  */
		neighbors.push_back(targetRobotController);
	    }
	}
    
	/* if found neighbors, broadcast my genome */
	if(neighbors.size() > 0) {
	    message msg (_genome, _fitness);

	    /* remove duplicates */
	    std::sort(neighbors.begin(), neighbors.end()); 
	    auto last = std::unique(neighbors.begin(), neighbors.end());
	    neighbors.erase(last, neighbors.end());
	
	    /* send */
	    for (const auto& c : neighbors)
		c->storeGenome (_wm->getId() , msg);
	    
	    /* some screen output */
	    /*if (gVerbose){
	      std::cout << "@"  << _iteration << " R" << getId() << " -> " ;
	      for (const auto& c : neighbors)
	      std::cout << c->getId() << " ";
	      std::cout << std::endl;
	      }*/
	    
	    /* delete neighbors list */
	    neighbors.clear();
	}
    }
}

void esMedeaController::storeGenome(int id, message msg){
    _population[id] = msg;
}

void esMedeaController::emptyGenomeList(){
    _population.clear();
}







double esMedeaController::updateFitnessLocomotion(){
    double lv = _outputs[0]-0.5;
    double rv = _outputs[1]-0.5;
    double md = (*_sensors)[0];

    for(int i = 1; i < _wm->_cameraSensorsNb; i++)
	if(md < (*_sensors)[i])
	    md = (*_sensors)[i];
    
    _locomotion += (fabs(lv) + fabs(rv)) * (1.0 - sqrt(fabs(lv - rv))) * 
	(1.0 - md) ;
    
    return _locomotion  / (double) getLifetime();
}

double esMedeaController::updateFitnessCollection(){
    return (double) _items_collected / (double) getLifetime();
}

double esMedeaController::updateFitnessForraging(){
    return (double) _items_forraged / (double) getLifetime();
}

void esMedeaController::updateFitness(){
    
    switch(esMedeaSharedData::gFitnessFunction) {

    case 0: /* locomotion */

	_fitness = updateFitnessLocomotion();
	break;

    case 1: /* Collection */
	_fitness = updateFitnessCollection();
	break;
	
    case 2: /* Forraging */ 
	_fitness = updateFitnessForraging();
	break;
	
    default:
	std::cerr << "Error unknown fitness function" << std::endl;
	exit (-1);
    }
}





/*
 *  Create a neurocontroller
 * 
 */
void esMedeaController::createController(){
    if( _nn != NULL )
	delete _nn;
    
    switch ( esMedeaSharedData::gControllerType ){
    case 0: // MLP
	_nn = new MLP(_genome, 
		      _nbInputs, 
		      _nbOutputs, 
		      *(_nbNeuronsPerHiddenLayer), 
		      esMedeaSharedData::gActiveBiais, 
		      esMedeaSharedData::gOnlyUseBiaisForFirstHiddenLayer);
	break;
	
    case 1: // PERCEPTRON
	_nn = new Perceptron(_genome, _nbInputs, _nbOutputs);
	break;
	
    case 2: // ELMAN
	_nn = new Elman(_genome, 
			_nbInputs, 
			_nbOutputs, 
			*(_nbNeuronsPerHiddenLayer), 
			esMedeaSharedData::gActiveBiais, 
			esMedeaSharedData::gOnlyUseBiaisForFirstHiddenLayer);
	break;
        
    default: // default: no controller
	std::cerr << "[ERROR] gController type unknown (value: " 
		  << esMedeaSharedData::gControllerType 
		  << ").\n";
	exit(-1);
    }
}


/*
 *  Save a neurocontroller
 * 
 */
void esMedeaController::saveController(){
    char fname[512];
    
    if ( !esMedeaSharedData::gControllersDirectory.empty()) 
	snprintf(fname, 511, "%s/%s/%04d-%010d.gen", 
		 gLogDirectoryname.c_str(),
		 esMedeaSharedData::gControllersDirectory.c_str(),  
		 _wm->getId(), 
		 _generation);
    else
	snprintf(fname, 511, "%s/%04d-%010d.gen", 
		 gLogDirectoryname.c_str(),
		 _wm->getId(), 
		 _generation);

    std::string sfname(fname);
    _nn->saveFile(_wm->getId(), sfname);
}

/*
 * Misc functions / methods 
 *
 */

void esMedeaController::pickItem(int item_id){
    _basket.push_back(item_id);
    _items_collected++;
}

void esMedeaController::emptyBasket(){
    _basket.clear();
}

void esMedeaController::dropItem(int n, bool at_nest){
    for (int i=0; i<n; i++)
	if(!_basket.empty())
	    _basket.pop_front();
    if(at_nest){
	_items_forraged += n;
	// are we on a landmark nest 
	if(gLandmarks.size() > 0 && _wm->getLandmarkDistanceValue() < 0.3)
	    _items_forraged_at_landmark += n;
    }
    else  
	_items_miss_droped += n;
}
 
bool esMedeaController::stillRoomInBasket() { 

    /* forraging => fixed capacity basket */
    if (esMedeaSharedData::gFitnessFunction > 1)
	return esMedeaSharedData::gBasketCapacity > (int) _basket.size();
    /* collection no limit */
    return true;
}

bool esMedeaController::matured(){
    /* TODO : rewrite if desynchronize */
    return _iteration > (getLifetime() * esMedeaSharedData::gMaturationTime);
}

bool esMedeaController::isEnergyItem(int id){
    return(gPhysicalObjects[id-gPhysicalObjectIndexStartOffset]->getType()==1);
} 

int esMedeaController::getLifetime(){
    return dynamic_cast <esMedeaWorldObserver*> 
	(gWorld->getWorldObserver())->getLifeIterationCount();  
}

bool esMedeaController::lifeTimeOver(){
    return dynamic_cast <esMedeaWorldObserver*> 
	(gWorld->getWorldObserver())->getLifeIterationCount() 
	>= esMedeaSharedData::gEvaluationTime - 1;
}

void esMedeaController::emptyPopulation(){
    _population.clear();
}

void esMedeaController::randomGenome(){
    unsigned int nbGene = _nn->getRequiredNumberOfWeights();
    _genome.clear();
    
    // weights: random init between -1 and +1
    for ( unsigned int i = 0 ; i != nbGene ; i++ )
        _genome.push_back(
	    (double)(rand() % esMedeaSharedData::gNeuronWeightRange) /
	    (esMedeaSharedData::gNeuronWeightRange/2)-1.0);

    _birthdate = _iteration;
}

void esMedeaController::printRobot(){
    std::cout << "[Robot: id="    <<  _wm->getId() 
	      << " iter="         <<  _iteration
	      << " birt="         <<  _birthdate

	      << " fit="          <<  _fitness
	      << " sig="          <<  _sigma 
	      << " r_fit"         <<  _reported_fitness
	      << " r_sig="        <<  _sigma 
	
	      << " nin="          <<  _nbInputs
	      << " nout="         <<  _nbOutputs
	      << " nh="           <<  _nbHiddenLayers

	      << " loc=" << _locomotion
	      << " col=" << _items_collected
	      << " for=" << _items_forraged
	      << " drp=" << _items_miss_droped
	      << " ldk=" << _items_forraged_at_landmark 
	      << " bsk=" << _basket.size()
	      << " usg=" << _basket_usage
	      << " ]\n";
}


void esMedeaController::printGenome(){
    std::cout << "[genome: id " <<  _wm->getId() 
	      << ": " ;
   
    for (const auto& g : _genome)
	std::cout << g << ", " ; 

    std::cout << "]" << std::endl;
}

void esMedeaController::printSensors(){
    std::cout << "[sensors: id " <<  _wm->getId() 
	      << ": " ;
   
    for (const auto& g : *_sensors)
	std::cout << g << ", " ; 

    std::cout << "]" << std::endl;
}

void esMedeaController::printOutputs(){
    std::cout << "[outputs: id " <<  _wm->getId() 
	      << ": " ;
   
    for (const auto& g : _outputs)
	std::cout << g << ", " ; 

    std::cout << "]" << std::endl;
}

void esMedeaController::printBasket(){
    std::cout << "[Basket: id " <<  _wm->getId() 
	      << ": " ;
   
    for (const auto& g : _basket)
	std::cout << g << ", " ; 

    std::cout << "]" << std::endl;
}

void esMedeaController::printPopulation(){
    std::cout << "[population id " <<  _wm->getId() 
	      << ": " ;

    std::map<int, message>::iterator it;
    for (it=_population.begin() ; it != _population.end(); it++)
	std::cout << it->first  << ", " ; 
	
	std::cout << "]" << std::endl;
}
