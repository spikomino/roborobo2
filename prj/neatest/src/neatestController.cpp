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
  _currentSigma    = neatestSharedData::gSigmaRef;
  _wm->setAlive (true);

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
	_nbInputs += (1) * _wm->_cameraSensorsNb;  // switch sensor
    _nbInputs += _wm->_cameraSensorsNb;	           // proximity sensors
    _nbOutputs = 2;                                // motor output

    // Start with Simple Perceptron Inputs, outputs, 0 hidden neurons. 
    _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);
    _genome->setIdTrace (_wm->getId ());
    _genome->genome_id = _wm->getId ();
    _genome->setMom (-1);
    _genome->setDad (-1);  
    _genome->mutate_link_weights (1.0, 1.0, COLDGAUSSIAN);

    // create a neuro controller from this genome
    createNeuroController();

    // empty the genome list 
    emptyGenomeList();

    if (gVerbose)
	std::cout << std::flush;
    
    //setNewGenomeStatus (true);
      
    //TOFIX NEAT-like innovation number and node id FOR THIS ROBOT
    //innovNumber = (double) _neurocontroller->linkcount ();
    //nodeId = 1 + _nbInputs + _nbOutputs;
}

void neatestController::emptyGenomeList(){
    _genomesList.clear();
    _fitnessList.clear();
    _sigmaList.clear();
    _birthdateList.clear();
}

void neatestController::reset(){
    _currentFitness = 0.0;
    _birthdate = gWorld->getIterations ();
    emptyGenomeList();
}

void neatestController::createNeuroController (){
  if (_neurocontroller != NULL)
    delete _neurocontroller;
  _neurocontroller = _genome->genesis (_wm->_id);
}

//unsigned int neatestController::computeRequiredNumberOfWeights (){
//  unsigned int res = _neurocontroller->linkcount ();
//  return res;
//}


bool neatestController::lifeTimeOver(){
    return dynamic_cast <neatestWorldObserver*> 
	(gWorld->getWorldObserver())->getLifeIterationCount() 
	>= neatestSharedData::gEvaluationTime - 1;
}


void neatestController::step (){
  _iteration++;

  stepBehaviour ();
  broadcastGenome ();

  if (lifeTimeOver()){
      stepEvolution ();
      reset();
  }
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void neatestController::stepBehaviour () {

    // step the neural controller and read outputs 
    std::pair<std::vector<double>, std::vector<double>> io = act();
    
    std::vector <double> outputs = io.second;
    std::vector <double> inputs  = io.first;
    
    _wm->_desiredTranslationalValue = outputs[0]; 
    _wm->_desiredRotationalVelocity = 2.0 * (outputs[1] - 0.5); // [-1, 1]

    // normalize to motor interval values
    _wm->_desiredTranslationalValue =
	_wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
	_wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
    
    _currentFitness += updateFitness (inputs, outputs);
}

std::pair<std::vector<double>, std::vector<double>> neatestController::act(){
    // ---- Build inputs ----
    std::vector<double>* inputs = new std::vector<double>(_nbInputs);
    int inputToUse = 0;
    
    (*inputs)[inputToUse++] = 1.0; // bias
    
    // distance sensors
    for (int i = 0; i < _wm->_cameraSensorsNb; i++){
	(*inputs)[inputToUse] =
	    _wm->getDistanceValueFromCameraSensor (i) /
	    _wm->getCameraSensorMaximumDistanceValue (i);
	inputToUse++;
	
	if (gExtendedSensoryInputs)   {
	    int objectId = _wm->getObjectIdFromCameraSensor (i);
	    
	    // input: physical object? which type?
	    if (PhysicalObject::isInstanceOf (objectId)){
		int nbOfTypes = 5;	//Only type 4 (Switch)
		for (int i = 4; i != nbOfTypes; i++){
		    if (i == (gPhysicalObjects
			      [objectId -
			       gPhysicalObjectIndexStartOffset]->getType ())){
			(*inputs)[inputToUse] = 1.0;	// match
		    }
		    else
			(*inputs)[inputToUse] = 0.0;
		    inputToUse++;
		}
	    }
	    // not an object.Should still fill in the inputs (with zeroes)
	    else{
		int nbOfTypes = 5;
		for (int i = 4; i != nbOfTypes; i++){
		    (*inputs)[inputToUse] = 0;
		    inputToUse++;
		}
	    }
	}
    }

    // step the neuro controller
    _neurocontroller->load_sensors ((&(*inputs)[0]));
    if (!_neurocontroller->activate()){
	std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
	exit (-1);
    }
    
    // read output
    std::vector<double> outputs;
    std::vector<NNode*>::iterator out_iter;
    for (out_iter = _neurocontroller->outputs.begin ();
	 out_iter != _neurocontroller->outputs.end (); 
	 out_iter++)
	outputs.push_back ((*out_iter)->activation);
    
    return std::make_pair(*inputs,outputs);
}


float neatestController::updateFitness (std::vector < double >in,
					 std::vector < double >out){
    float deltaFit = 0.0;
    int targetIndex = _wm->getGroundSensorValue ();
    if (PhysicalObject::isInstanceOf (targetIndex))
	deltaFit += 1.0;
    return deltaFit;
}

void neatestController::broadcastGenome () {
    // only if agent is active (ie. not just revived) and deltaE>0.
    if (_wm->isAlive () == true){
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
		targetRobotController->storeGenome (_genome, _wm->getId (),
						    _birthdate, _currentSigma,
						    _currentFitness);
	    }
	}
    }
}

void neatestController::storeGenome(GenomeAdapted * genome, int senderId,
				    int senderBirthdate, float sigma,
				    float fitness){
    //08/10/14 (storeGenome Adaptedto NEAT, I think)
    _genomesList[senderId]   = genome;
    _sigmaList[senderId]     = sigma;
    _birthdateList[senderId] = senderBirthdate;
    _fitnessList[senderId]   = fitness;
}

// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void neatestController::stepEvolution () {

    // save genome in file / log 
    logGenome();
    
    // store our genome in the list 
    storeGenome(_genome, _wm->getId(), _birthdate, 
		_currentSigma, _currentFitness);
    
    // select an offspring 
    int selected = selectBest (_fitnessList);
    _genome = _genomesList[selected];
    _currentFitness = _fitnessList[selected];
    
    // mutate the offspring 
    int newId = _wm->getId () + 10000 * 
	(1 + (gWorld->getIterations () /
	      neatestSharedData::gEvaluationTime));
    _genome = _genome->mutate (_currentSigma,
				 _wm->getId(), 
				 newId, 
				 nodeId, 
				 innovNumber);
    createNeuroController();
}


void neatestController::logGenome() {
    //GENERATION ID-ROBOT FITNESS IDGENOME IDMOM
    std::cout << gWorld->getIterations()/neatestSharedData::gEvaluationTime
	      << " " << _wm->getId () 
	      << " " <<	_currentFitness 
	      << " " << _genome->getIdTrace () 
	      << " " << _genome->getMom () 
	      << std::endl;
    
    std::string filename = "logs/genomes/"; 
    filename = neatestSharedData::gGenomeLogFolder;
    filename += std::to_string(_genome -> getIdTrace());
    _genome -> print_to_filename(const_cast<char*>(filename.c_str()));
}




int neatestController::selectBest (std::map < int, float >lFitness) {
    std::map < int, float >::iterator it = lFitness.begin();
    float bestFit = it->second;
    int idx = it->first;
    for (; it != lFitness.end (); it++)
	if (it->second > bestFit)  {
	    bestFit = it->second;
	    idx = it->first;
	}
    return idx;
}

int  neatestController::selectRandom(std::map < int, float >lFitness){       
    return  rand () % lFitness.size ();
}

