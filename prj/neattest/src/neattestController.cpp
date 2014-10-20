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

#include "neattest/include/neattestController.h"
#include "neattest/include/neattestWorldObserver.h"

using namespace NEAT;

neattestController::neattestController (RobotWorldModel * wm){

  _wm = wm;
  nn = NULL;

  load_neat_params ("prj/neattest/src/explo.ne", false);

  _currentSigma = neattestSharedData::gSigmaRef;

  initRobot ();

  _iteration = 0;
  _birthdate = 0;

  _wm->setAlive (true);
}

neattestController::~neattestController (){
    delete nn;
    nn = NULL;
}

void neattestController::reset (){
    _currentFitness = 0.0;
    _birthdate = gWorld->getIterations ();
    
    _genomesList.clear ();
    _fitnessList.clear ();
    _sigmaList.clear ();
    _birthdateList.clear ();
}

void neattestController::initRobot (){
    _nbInputs = 1;		// Bias constant input (1.0)
    
    if (gExtendedSensoryInputs)
	_nbInputs += (1) * _wm->_cameraSensorsNb;  // Switch
       
    _nbInputs += _wm->_cameraSensorsNb;	// proximity sensors
    _nbOutputs = 2;

    
    // Inputs, outputs, 0 hidden neurons, fully connected. 
    // Start with Simple Perceptron 
    _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);

    _genome->setIdTrace (_wm->getId ());
    _genome->genome_id = _wm->getId ();
    _genome->setMom (-1);
    _genome->setDad (-1);
      
    _genome->mutate_link_weights (1.0, 1.0, COLDGAUSSIAN);

    createNN ();

    if (gVerbose)
	std::cout << std::flush;
    
    setNewGenomeStatus (true);
    _genomesList.clear ();
    _fitnessList.clear ();
    
    //TOFIX NEAT-like innovation number and node id FOR THIS ROBOT
    innovNumber = (double) nn->linkcount ();
    nodeId = 1 + _nbInputs + _nbOutputs;
}


void neattestController::createNN (){
  if (nn != NULL)
    delete nn;
  nn = _genome->genesis (_wm->_id);
}

unsigned int neattestController::computeRequiredNumberOfWeights (){
  unsigned int res = nn->linkcount ();
  return res;
}

void neattestController::step (){
  _iteration++;

  stepBehaviour ();
  broadcastGenome ();

  if (dynamic_cast <
      neattestWorldObserver *
      >(gWorld->getWorldObserver ())->getLifeIterationCount () >=
      neattestSharedData::gEvaluationTime - 1){

      stepEvolution ();
      reset();
  }
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void neattestController::stepBehaviour () {

    // step the neural controller and read outputs 
    std::pair<std::vector<double>, std::vector<double>> io = act();
    
    std::vector <double> outputs = io.second;
    std::vector <double> inputs  = io.first;
    
    _wm->_desiredTranslationalValue = outputs[0];
    _wm->_desiredRotationalVelocity = 2 * (outputs[1] - 0.5); // [-1, 1]

    // normalize to motor interval values
    _wm->_desiredTranslationalValue =
	_wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity =
	_wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
    
    _currentFitness += updateFitness (inputs, outputs);
}

std::pair<std::vector<double>,std::vector<double>> neattestController::act(){
    // ---- Build inputs ----
    std::vector < double >*inputs = new std::vector < double >(_nbInputs);
    int inputToUse = 0;
    
    (*inputs)[inputToUse++] = 1.0;
    
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
			(*inputs)[inputToUse] = 1;	// match
		    }
		    else
			(*inputs)[inputToUse] = 0;
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

    // ---- compute and read out ----
    nn->load_sensors ((&(*inputs)[0]));

    if (!(nn->activate ())) {
	std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
	exit (-1);
    }

    std::vector < double >outputs;
    for (std::vector < NNode * >::iterator out_iter = nn->outputs.begin ();
	 out_iter != nn->outputs.end (); out_iter++)
	outputs.push_back ((*out_iter)->activation);
    
    return std::make_pair(*inputs,outputs);
}


float neattestController::updateFitness (std::vector < double >in,
					 std::vector < double >out){
    float deltaFit = 0.0;
    int targetIndex = _wm->getGroundSensorValue ();
    if (PhysicalObject::isInstanceOf (targetIndex))
	deltaFit += 1.0;
    return deltaFit;
}

void neattestController::broadcastGenome () {
    // only if agent is active (ie. not just revived) and deltaE>0.
    if (_wm->isAlive () == true){
	for (int i = 0; i < _wm->_cameraSensorsNb; i++)	{
	    int targetIndex = _wm->getObjectIdFromCameraSensor (i);
	    
	    // sensor ray bumped into a robot : communication is possible
	    if (targetIndex >= gRobotIndexStartOffset){
		// convert image registering index into robot id.
		targetIndex = targetIndex - gRobotIndexStartOffset;
		
		neattestController *targetRobotController =
		    dynamic_cast <
		    neattestController *
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

void neattestController::storeGenome (GenomeAdapted * genome, int senderId,
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

void neattestController::stepEvolution () {

    // save genome in file / log 
    logGenome();
    
    // store our genome 
    _genomesList[_wm->getId ()]   = _genome;
    _fitnessList[_wm->getId ()]   = _currentFitness;
    _sigmaList[_wm->getId ()]     = _currentSigma;
    _birthdateList[_wm->getId ()] = _birthdate;
    
    // select an offspring 
    int selected = selectBest (_fitnessList);
    _genome = _genomesList[selected];
    _currentFitness = _fitnessList[selected];
    
    // mutate the offspring 
    int newId = _wm->getId () + 10000 * 
	(1 + (gWorld->getIterations () /
	      neattestSharedData::gEvaluationTime));
    _genome = _genome->mutate (_currentSigma,
				 _wm->getId (), 
				 newId, 
				 nodeId, 
				 innovNumber);
    createNN ();
}


void neattestController::logGenome() {
    //GENERATION ID-ROBOT FITNESS IDGENOME IDMOM
    std::cout << gWorld->getIterations()/neattestSharedData::gEvaluationTime
	      << " " << _wm->getId () 
	      << " " <<	_currentFitness 
	      << " " << _genome->getIdTrace () 
	      << " " << _genome->getMom () 
	      << std::endl;
    
    std::string filename = "logs/genomes/"; 
    filename = neattestSharedData::gGenomeLogFolder;
    filename += std::to_string(_genome -> getIdTrace());
    _genome -> print_to_filename(const_cast<char*>(filename.c_str()));
}




int neattestController::selectBest (std::map < int, float >lFitness) {
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

int  neattestController::selectRandom(std::map < int, float >lFitness){       
    return  rand () % lFitness.size ();
}

