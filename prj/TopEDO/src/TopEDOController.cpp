/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 * 
 */

#include "TopEDO/include/TopEDOController.h"
#include "TopEDO/include/TopEDOWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <string>

#include <neuralnetworks/MLP.h>
#include <neuralnetworks/Perceptron.h>
#include <neuralnetworks/Elman.h>

#include <neat/genome.h>
#include "neat/genomeAdapted.h"

using namespace Neural;
using namespace NEAT;

TopEDOController::TopEDOController (RobotWorldModel * wm)
{

  _wm = wm;

  nn = NULL;
  NEAT::load_neat_params ("prj/TopEDO/src/explo.ne", false);

  _currentSigma = TopEDOSharedData::gSigmaRef;

  initRobot ();

  _iteration = 0;
  _birthdate = 0;
}

TopEDOController::~TopEDOController ()
{
  delete nn;
  nn = NULL;
}

void
TopEDOController::reset ()
{
  _currentFitness = 0.0;
  _birthdate = gWorld->getIterations ();
  
  _genomesList.clear ();
  _fitnessList.clear ();
  _sigmaList.clear ();
  _birthdateList.clear ();
  
}

void
TopEDOController::initRobot ()
{
  _nbInputs = 1;		// Bias constant input (1.0)

  if (gExtendedSensoryInputs)
    {
      _nbInputs += (1) * _wm->_cameraSensorsNb;	// Switch
    }

  _nbInputs += _wm->_cameraSensorsNb;	// proximity sensors

  _nbOutputs = 2;


  // Inputs, outputs, 0 hidden neurons, fully connected. Start with Simple Perceptron 
  _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);

  _genome->setIdTrace (_wm->getId ());
  _genome->setMom (-1);
  _genome->setDad (-1);
  _genome->genome_id = _wm->getId ();

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


void
TopEDOController::createNN ()
{
  if (nn != NULL)
    delete nn;

  nn = _genome->genesis (_wm->_id);

}

unsigned int
TopEDOController::computeRequiredNumberOfWeights ()
{
  unsigned int res = nn->linkcount ();
  return res;
}

void
TopEDOController::step ()
{
  _iteration++;
  stepBehaviour ();
  
  if (dynamic_cast <
      TopEDOWorldObserver *
      >(gWorld->getWorldObserver ())->getLifeIterationCount () >=
      TopEDOSharedData::gEvaluationTime - 1){

      stepEvolution ();
      reset();
  }
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void
TopEDOController::stepBehaviour ()
{
  std::pair<std::vector<double>,std::vector<double>> io = act();

  std::vector<double> outputs = io.second;
  std::vector < double >inputs = io.first;
  
  _wm->_desiredTranslationalValue = outputs[0];
  //Rotational velocity in [-1,+1]
  _wm->_desiredRotationalVelocity = 2 * (outputs[1] - 0.5);

  // normalize to motor interval values
  _wm->_desiredTranslationalValue =
    _wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
  _wm->_desiredRotationalVelocity =
    _wm->_desiredRotationalVelocity * gMaxRotationalSpeed;

  _currentFitness += updateFitness (inputs, outputs);

  // broadcasting genome : robot broadcasts its genome 
  //to all neighbors (contact-based wrt proximity sensors)

  broadcastGenome ();
}

std::pair<std::vector<double>,std::vector<double>> TopEDOController::act()
  {
  // ---- Build inputs ----

    std::vector < double >inputs(_nbInputs);
  int inputToUse = 0;

  inputs[inputToUse++] = 1.0;

  // distance sensors
  for (int i = 0; i < _wm->_cameraSensorsNb; i++)
    {
      inputs[inputToUse] =
	_wm->getDistanceValueFromCameraSensor (i) /
	_wm->getCameraSensorMaximumDistanceValue (i);
      inputToUse++;
      
      if (gExtendedSensoryInputs)
	{
	  int objectId = _wm->getObjectIdFromCameraSensor (i);
	  
	  // input: physical object? which type?
	  if (PhysicalObject::isInstanceOf (objectId))
	    {
	      //Switch is type 3
	      if ((gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]
		   ->getType ()) == 3)
		{
		  inputs[inputToUse] = 1;	// match
		}
	      else
		inputs[inputToUse] = 0;
	      inputToUse++;
	    }	 
	  else //Not physical object
	    {	      
	      inputs[inputToUse] = 0;
	      inputToUse++;
	    }

	}
    }
 
  // ---- compute and read out ----
  nn->load_sensors (&(inputs[0]));

  if (!(nn->activate ()))
    {
      std::cerr << "[ERROR] Activation of ANN not correct" << std::endl;
      exit (-1);
    }

  std::vector < double >outputs;
  for (std::vector < NNode * >::iterator out_iter = nn->outputs.begin ();
       out_iter != nn->outputs.end (); out_iter++)
    {
      outputs.push_back ((*out_iter)->activation);
    }
  return std::make_pair(inputs,outputs);
}


float
TopEDOController::updateFitness (std::vector < double >in,
				 std::vector < double >out)
{
  float deltaFit = 0.0;
  int targetIndex = _wm->getGroundSensorValue ();

  if (PhysicalObject::isInstanceOf (targetIndex))
    {
      if((gPhysicalObjects[targetIndex - gPhysicalObjectIndexStartOffset]
		   ->getType ()) == 3)
	deltaFit += 1.0;
    }

  return deltaFit;
}

void
TopEDOController::broadcastGenome ()
{
  // only if agent is active (ie. not just revived) and deltaE>0.
  for (int i = 0; i < _wm->_cameraSensorsNb; i++)
    {
      int targetIndex = _wm->getObjectIdFromCameraSensor (i);

      // sensor ray bumped into a robot : communication is possible
      if (targetIndex >= gRobotIndexStartOffset)
	{
	  // convert image registering index into robot id.
	  targetIndex = targetIndex - gRobotIndexStartOffset;
	  
	  TopEDOController *targetRobotController =
	    dynamic_cast <
	    TopEDOController *
	    >(gWorld->getRobot (targetIndex)->getController ());
	  
	  if (!targetRobotController)
	    {
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
void
TopEDOController::storeGenome (GenomeAdapted * genome, int senderId,
			       int senderBirthdate, float sigma,
			       float fitness)
{
  //08/10/14 (storeGenome Adaptedto NEAT, I think)
  _genomesList[senderId] = genome;
  _sigmaList[senderId] = sigma;
  _birthdateList[senderId] = senderBirthdate;
  _fitnessList[senderId] = fitness;

}

// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void
TopEDOController::stepEvolution ()
{

    logGenome();
    
    //L = L + A
    _genomesList[_wm->getId ()] = _genome;
    _fitnessList[_wm->getId ()] = _currentFitness;
    _sigmaList[_wm->getId ()] = _currentSigma;
    _birthdateList[_wm->getId ()] = _birthdate;
    
    int selected =-1;
    switch(TopEDOSharedData::gSelectionMethod)
      {
      case 1:
	selected = selectBest (_fitnessList);
	break;
      case 2:
	selected = selectRankBased (_fitnessList);
	break;
      case 3: 
	selected = selectBinaryTournament (_fitnessList);
	break;
      case 4:
	selected = selectRandom(_fitnessList);
	break;
      default: 
	   std::cerr << "[ERROR] Selection method unknown (value: " << TopEDOSharedData::gSelectionMethod << ").\n";
            exit(-1);
      }
    
    _genome = _genomesList[selected];
    _currentFitness = _fitnessList[selected];
   
    
    int newId =
	_wm->getId () + 10000 * (1 +
				 (gWorld->getIterations () /
				  TopEDOSharedData::gEvaluationTime));
    
    _genome = _genome -> mutate (_currentSigma,
				 _wm->getId (), newId, nodeId, innovNumber);
    
    createNN ();
    
}


void TopEDOController::logGenome()
{
  
  //GENERATION ID-ROBOT FITNESS IDGENOME IDMOM
  TopEDOSharedData::gEvoLog << (gWorld->getIterations () /
    TopEDOSharedData::
    gEvaluationTime) << " " << _wm->getId () << " " <<
    _currentFitness << " " << _genome->getIdTrace () << " " << _genome->
    getMom () << std::endl;
  
  
  std::string filename; 
  filename = TopEDOSharedData::gGenomeLogFolder;
  filename += std::to_string(_genome -> getIdTrace());
  
  _genome -> print_to_filename(const_cast<char*>(filename.c_str()));
  
}




int
TopEDOController::selectBest (std::map < int, float >lFitness)
{
  std::map < int, float >::iterator it = lFitness.begin();

  float bestFit = it->second;
  int idx = it->first;

  for (; it != lFitness.end (); it++)
    {
      if (it->second > bestFit)
	{
	  bestFit = it->second;
	  idx = it->first;
	}

    }
  return idx;
}
int
TopEDOController::selectRankBased(std::map < int, float >lFitness)
{       
  std::map < int, float >::iterator it = lFitness.begin();
  float totalFitness = 0.0;
  int result = -1;

  for (it->first; it != lFitness.end (); it++)
    {
      totalFitness += it->second; 

    }
  float random = randfloat() * totalFitness;
  it = lFitness.begin();

  while((random > 0.0) && (it != lFitness.end()))
    {
      it++;
    }
  if(it != lFitness.end())
    result = it->first;
  else
    {
      it--;
      result = it->first;
    }
  return result;
}
int
TopEDOController::selectBinaryTournament(std::map < int, float >lFitness)
{
  int result = -1;
  if(lFitness.size() > 1)
    {
      int ind1 =  rand () % lFitness.size ();
      int ind2 =  rand () % lFitness.size ();
      while(ind1 == ind2)
	{
	  ind2 =  rand () % lFitness.size ();
	}
      if(lFitness[ind1] >= lFitness[ind2])
	result = ind1;
      else
	result = ind2;
    }
  else 
    result = lFitness.begin()->first;

  return result;
}
int
TopEDOController::selectRandom(std::map < int, float >lFitness)
{       
  return  rand () % lFitness.size ();
  
}

