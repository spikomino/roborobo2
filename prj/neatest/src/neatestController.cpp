/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 * 
 */

#include <math.h>
#include <string>

#include "World/World.h"
#include "Utilities/Misc.h"

#include "neat/genome.h"
#include "neat/genomeAdapted.h"

#include "neatest/include/neatestController.h"
#include "neatest/include/neatestWorldObserver.h"

using namespace NEAT;


neatestController::neatestController(RobotWorldModel * wm){
  _wm              = wm;
  _iteration       = 0;
  _birthdate       = 0;
  _neurocontroller = NULL;
  _sigma           = neatestSharedData::gSigmaRef;
  _wm->setAlive(true);

  load_neat_params ("prj/neatest/src/explo.ne", false);
  initRobot ();
}

neatestController::~neatestController (){
    delete _neurocontroller;
    _neurocontroller = NULL;
}

void neatestController::initRobot (){
    // setup the number of input and outputs 
    _nbInputs = 1;		                   // bias 
    if (gExtendedSensoryInputs)
	_nbInputs += (1) * _wm->_cameraSensorsNb;  // object sensor
    _nbInputs += _wm->_cameraSensorsNb;	           // proximity sensors
    _nbOutputs = 2;                                // motor output

    // Start with Simple Perceptron Inputs, outputs, 0 hidden neurons. 
    _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);
    _genome->setIdTrace (_wm->getId());
    _genome->genome_id = _wm->getId();
    _genome->setMom(-1);
    _genome->setDad(-1);  
    _genome->mutate_link_weights (1.0, 1.0, COLDGAUSSIAN);

    // create a neuro controller from this genome
    createNeuroController();

    // empty the genome list 
    emptyGenomeList();

    if (gVerbose){
	std::cout << "[initRobot] " 
		  << "id="  << _wm->getId() << " "
		  << "in="  << _nbInputs    << " "
		  << "out=" << _nbOutputs 
		  << std::endl;
	printRobot();
    }
    
      
    //TOFIX NEAT-like innovation number and node id FOR THIS ROBOT
    //innovNumber = (double) _neurocontroller->linkcount ();
    //nodeId = 1 + _nbInputs + _nbOutputs;
}



void neatestController::printRobot(){
    std::string s;
    
    std::cout << "[Robot #"        + to_string(_wm->getId())    + "]\n"
	      << "\t iteration = " + to_string(_iteration)      + "\n"
	      << "\t birthdate = " + to_string(_birthdate)      + "\n"
	      << "\t fitness   = " + to_string(_fitness) + "\n"
	      << "\t sigma     = " + to_string(_sigma)   + "\n"
	      << "\t [Genome] \n" 
	      << "\t\t id  = " + to_string(_genome->getIdTrace()) + "\n"
	      << "\t\t mon = " + to_string(_genome->getMom()) + "\n"
	      << "\t\t dad = " + to_string(_genome->getDad()) + "\n";
	
    
    //_genome->print_to_file(std::cout);
    
    std::cout << "\t[Genome list]\n";
    std::map<int, message>::iterator it;
    for (it=_glist.begin() ; it != _glist.end(); it++){
	std::cout << "\t\t[" << it->first << "] " ;
	printMessage(it->second);
	std::cout << std::endl;
    }
}



void neatestController::createNeuroController (){
  if (_neurocontroller != NULL)
    delete _neurocontroller;
  _neurocontroller = _genome->genesis (_wm->_id);
}

bool neatestController::lifeTimeOver(){
    return dynamic_cast <neatestWorldObserver*> 
	(gWorld->getWorldObserver())->getLifeIterationCount() 
	>= neatestSharedData::gEvaluationTime - 1;
}

void neatestController::reset(){
    _fitness   = 0.0;
    _birthdate = gWorld->getIterations ();
    emptyGenomeList();
    createNeuroController();
}

void neatestController::step (){
  _iteration++;

  stepBehaviour(); // execure the neuro controller
  broadcast();  
  printRobot();

  if (lifeTimeOver()){
      std::cout << "------------------------ Gen \n";
      
      stepEvolution (); // select, mutate, replace

      printRobot();

      reset(); // reset fitness and neurocontroller
  }
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################


/*
 * Step the neuro controller and execute command
 *
 */
void neatestController::stepBehaviour(){
    double inputs[_nbInputs]; 
    int inputToUse = 0;
    
    inputs[inputToUse++] = 1.0; // bias

    // distance sensors 
    for(int i = 0; i < _wm->_cameraSensorsNb; i++)
	inputs[inputToUse++] = _wm->getDistanceValueFromCameraSensor (i) /
	    _wm->getCameraSensorMaximumDistanceValue (i);

    // object sensors 
    if(gExtendedSensoryInputs) {
	// Energy objects (type 1)
	for(int i = 0; i < _wm->_cameraSensorsNb; i++){
	    int objectId = _wm->getObjectIdFromCameraSensor (i);
	    // if physical object, and of type 1
	    if(PhysicalObject::isInstanceOf(objectId)){
		int type = gPhysicalObjects
		    [objectId-gPhysicalObjectIndexStartOffset]->getType();
		if(type == 1)
		    inputs[inputToUse] = 1.0;	// match 
		else
		    inputs[inputToUse] = 0.0;
		inputToUse++;
	    }
	    else{// Physical object but not interesting 
		inputs[inputToUse] = 0.0;
		inputToUse++;
	    }
	}
    }
    // some output 
    if(gVerbose){
	std::cout << "[Controller] "
		  << "\t[Robot #" + to_string(_wm->getId()) + "]\n"
		  << "\t\t[Inputs : " ;
	for(unsigned int i = 0; i < _nbInputs; i++){
	    std::cout << to_string(inputs[i]) + " ";
	    if ((i % _wm->_cameraSensorsNb) == 0)
		std::cout << "]" <<  std::endl << "\t\t\t[ ";
	}
	std::cout <<  std::endl;
    }
	
    // step the neuro controller
    _neurocontroller->load_sensors(inputs);
    if (!_neurocontroller->activate()){
	std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
	exit (-1);
    }
    
    // read output
    std::vector<double> outputs;
    std::vector<NNode*>::iterator out_iter;
    for (out_iter  = _neurocontroller->outputs.begin();
	 out_iter != _neurocontroller->outputs.end(); 
	 out_iter++)
	outputs.push_back ((*out_iter)->activation);

    // more output 
    if(gVerbose){
	std::cout <<  "\t\t[Outputs : " ;
	std::vector<double>::iterator itr;
	for(itr = outputs.begin (); itr != outputs.end (); itr++)
	    std::cout << to_string(*itr) + " ";
	std::cout << "]"
		  << std::endl;
    }
    
    // execute the motor commands 
    _wm->_desiredTranslationalValue = outputs[0]; 
    _wm->_desiredRotationalVelocity = 2.0 * (outputs[1] - 0.5); // [-1, 1]

    // normalize to motor interval values
    _wm->_desiredTranslationalValue =
	_wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
	_wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
    

    //_wm->_desiredTranslationalValue = 0.0;
    //_wm->_desiredRotationalVelocity = 0.0;
    
    
}// end stepNeuralController



void neatestController::broadcast() {
    // only if agent is active (ie. not just revived) and deltaE>0.
    if (_wm->isAlive()){
	message msg (_genome, _fitness, _sigma, _birthdate);
	
	for (int i = 0; i < _wm->_cameraSensorsNb; i++)	{
	    int targetIndex = _wm->getObjectIdFromCameraSensor (i);
	    
	    // sensor ray bumped into a robot : communication is possible
	    if (targetIndex >= gRobotIndexStartOffset){
		// convert image registering index into robot id.
		targetIndex = targetIndex - gRobotIndexStartOffset;
		
		neatestController *targetRobotController =
		    dynamic_cast <
		    neatestController *
		    >(gWorld->getRobot (targetIndex)->getController ());

		if (!targetRobotController){
		    std::
			cerr << "Error from robot " << _wm->getId () <<
			" : the observer of robot " << targetIndex <<
			" is not compatible" << std::endl;
		    exit (-1);
		}
		
		// other agent stores my genome.
		targetRobotController->storeGenome (_wm->getId(), msg);
	    }
	}
    }
}

void neatestController::storeGenome(int id, message msg){
    _glist[id] = msg;
}

void neatestController::printMessage(message msg){
    GenomeAdapted* g;
    double f,s;
    int b;

    std::tie (g,f,s,b) = msg;
    std::cout << "(Id="        << g->getIdTrace()
	      << " fitness="   << f
	      << " sigma="     << s
	      << " birthdate=" << b << ")";
}

void neatestController::emptyGenomeList(){
    _glist.clear();
}

// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void neatestController::stepEvolution() {
    // store our genome in the list 
    message msg (_genome, _fitness, _sigma, _birthdate);
    storeGenome (_wm->getId(), msg);
   
    
    // select an offspring 
    int selected = selectRandom();

    _genome = std::get<0>(_glist[selected]);
    _sigma  = std::get<2>(_glist[selected]);
    
    // mutate the offspring 
    int newId = _wm->getId () + 10000 * 
	(1 + (gWorld->getIterations () /
	      neatestSharedData::gEvaluationTime));

    _genome = _genome->mutate (_sigma,
			       _wm->getId(), 
			       newId, 
			       nodeId, 
			       innovNumber);
}

void neatestController::updateFitness (double df){
    _fitness += df;
}

int  neatestController::selectRandom(){
    auto it = _glist.begin();
    std::advance(it, rand() % _glist.size());
    return it->first;
    
    
}

