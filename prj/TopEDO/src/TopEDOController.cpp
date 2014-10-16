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
  NEAT::load_neat_params ("src/contrib/neat/p2nv.ne", false);

  _currentSigma = TopEDOSharedData::gSigmaRef;

  resetRobot ();

  _iteration = 0;
  _birthdate = 0;

  _wm->updateLandmarkSensor ();

  _wm->setAlive (true);
  _wm->setRobotLED_colorValues (255, 0, 0);

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
}

void
TopEDOController::resetRobot ()
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
  innovNum = (double) nn->linkcount ();
  nodeId = 1 + _nbInputs + _nbOutputs;


}



void
TopEDOController::step ()
{
  _iteration++;

  stepEvolution ();

  stepBehaviour ();
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void
TopEDOController::stepBehaviour ()
{

  // ---- Build inputs ----

  std::vector < double >*inputs = new std::vector < double >(_nbInputs);
  int inputToUse = 0;

  (*inputs)[inputToUse++] = 1.0;

  // distance sensors
  for (int i = 0; i < _wm->_cameraSensorsNb; i++)
    {
      (*inputs)[inputToUse] =
	_wm->getDistanceValueFromCameraSensor (i) /
	_wm->getCameraSensorMaximumDistanceValue (i);
      inputToUse++;

      if (gExtendedSensoryInputs)
	{
	  int objectId = _wm->getObjectIdFromCameraSensor (i);

	  // input: physical object? which type?
	  if (PhysicalObject::isInstanceOf (objectId))
	    {
	      int nbOfTypes = 5;	//Only type 4 (Switch)
	      for (int i = 4; i != nbOfTypes; i++)
		{
		  if (i ==
		      (gPhysicalObjects
		       [objectId -
			gPhysicalObjectIndexStartOffset]->getType ()))
		    {
		      (*inputs)[inputToUse] = 1;	// match
		    }
		  else
		    (*inputs)[inputToUse] = 0;
		  inputToUse++;
		}
	    }
	  else
	    {
	      // not a physical object.Should still fill in the inputs (with zeroes)
	      int nbOfTypes = 5;

	      for (int i = 4; i != nbOfTypes; i++)
		{
		  (*inputs)[inputToUse] = 0;
		  inputToUse++;
		}
	    }

	}
    }



  // ---- compute and read out ----


  nn->load_sensors ((&(*inputs)[0]));

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


  _wm->_desiredTranslationalValue = outputs[0];
  //Rotational velocity in [-1,+1]
  _wm->_desiredRotationalVelocity = 2 * (outputs[1] - 0.5);

  // normalize to motor interval values
  _wm->_desiredTranslationalValue =
    _wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
  _wm->_desiredRotationalVelocity =
    _wm->_desiredRotationalVelocity * gMaxRotationalSpeed;

  _currentFitness += updateFitness (*inputs, outputs);

  delete (inputs);
}

float
TopEDOController::updateFitness (std::vector < double >in,
				 std::vector < double >out)
{
  float deltaFit = 0.0;
  int targetIndex = _wm->getGroundSensorValue ();

  if (PhysicalObject::isInstanceOf (targetIndex))
    {
      deltaFit += 1.0;
    }

  return deltaFit;
}

float
TopEDOController::dist (float x1, float y1, float x2, float y2)
{
  float result = 0.0;
  result = sqrt ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  return result;
}

void
TopEDOController::createNN ()
{
  if (nn != NULL)
    delete nn;

  switch (TopEDOSharedData::gControllerType)
    {
    case 0:
      {
	nn = _genome->genesis (_wm->_id);
	break;
      }
    default:			// default: no controller
      std::cerr << "[ERROR] gController type unknown (value: " <<
	TopEDOSharedData::gControllerType << ").\n";
      exit (-1);
    };
}


unsigned int
TopEDOController::computeRequiredNumberOfWeights ()
{
  unsigned int res = nn->linkcount ();
  return res;
}

// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################

void
TopEDOController::stepEvolution ()
{
  // broadcasting genome : robot broadcasts its genome 
  //to all neighbors (contact-based wrt proximity sensors)
  if (gRadioNetwork)
    {
      broadcastGenome ();
    }

  // lifetime ended: replace genome (if possible)
  if (dynamic_cast <
      TopEDOWorldObserver *
      >(gWorld->getWorldObserver ())->getLifeIterationCount () >=
      TopEDOSharedData::gEvaluationTime - 1)
    {

      //GENERATION ID-ROBOT FITNESS IDGENOME IDMOM
      std::cout << (gWorld->getIterations () /
		    TopEDOSharedData::
		    gEvaluationTime) << " " << _wm->getId () << " " <<
	_currentFitness << " " << _genome->getIdTrace () << " " << _genome->
	getMom () << std::endl;

      loadNewGenome ();
    }
  // check for new NN parameters
  if (getNewGenomeStatus ())
    {
      reset ();
      setNewGenomeStatus (false);
    }
}

void
TopEDOController::selectRandomGenome ()
{
  if (_genomesList.size () != 0)
    {
      int randomIndex = rand () % _genomesList.size ();
      std::map < int, GenomeAdapted * >::iterator it = _genomesList.begin ();
      while (randomIndex != 0)
	{
	  it++;
	  randomIndex--;
	}

      _genome = (*it).second;

      mutate (_sigmaList[(*it).first]);

      setNewGenomeStatus (true);

      _birthdate = gWorld->getIterations ();

      // Logging
      std::string s = std::string ("");
      s +=
	"{" + std::to_string (gWorld->getIterations ()) + "} [" +
	std::to_string (_wm->getId ()) + "::" + std::to_string (_birthdate) +
	"] descends from [" + std::to_string ((*it).first) + "::" +
	std::to_string (_birthdateList[(*it).first]) + "]\n";
      gLogManager->write (s);
      gLogManager->flush ();

      _genomesList.clear ();
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


void
TopEDOController::mutate (float sigma)
{
  std::map < int, GenomeAdapted * >::iterator curorg;

  int poolsize;			//The number of Organisms in the old generation

  GenomeAdapted *mom;		//Parent genome
  GenomeAdapted *new_genome;	//For holding baby's genes

  Network *net_analogue;	//For adding link to test for recurrency


  NEAT::weight_mut_power = 1.0;
  NEAT::pop_size = 1;

  //The weight mutation power is species specific depending on its age
  double mut_power = NEAT::weight_mut_power;


  //NOTE: added by Inaki: no loop (expected_offspring = 1)
  //since each robot can have one offspring (at most, i.e. if it does not die)

  poolsize = _genomesList.size () - 1;


  int newId =
    _wm->getId () + 10000 * (1 +
			     (gWorld->getIterations () /
			      TopEDOSharedData::gEvaluationTime));


  /************************************************/
  //NOTE: default NEAT method (randomly) replaced  by previous selection (best)
  mom = _genome;

  new_genome = (mom)->duplicate (_wm->getId (), newId);

  /************************************************/

  //First, decide whether to mate or mutate
  //If there is only one organism in the pool, then always mutate
  if ((randfloat () < NEAT::mutate_only_prob) || poolsize == 0)
    {
      //Do the mutation depending on probabilities of various mutations  
      if (randfloat () < NEAT::mutate_add_node_prob)
	{

	  //08/10/14  Bogus variables for innovations deactivated 
	  //perRobot innovNum and nodeId used instead

	  std::vector < Innovation * >innovations;

	  if (new_genome->mutate_add_node (innovations, nodeId, innovNum))
	    {
	      //std::cout << "Mutate add node " << nodeId - 1 << std::endl;
	    }

	}
      else if (randfloat () < NEAT::mutate_add_link_prob)
	{
	  //NOTE: Inaki Hack, generation is only used as a network_id
	  int generation = 0;
	  //No further repercusion of this parameter
	  net_analogue = new_genome->genesis (generation);

	  std::vector < Innovation * >innovations;
	  if (new_genome->mutate_add_link (innovations, innovNum,
					   NEAT::newlink_tries))
	    {
	      //std::cout << "Mutate add link" << std::endl;
	    }
	  delete net_analogue;

	}
      //NOTE:links CANNOT be added directly after a node  because the phenotype
      // will not be appropriately altered to reflect the change
      else
	{
	  //If we didn't do a structural mutation, we do the other kinds

	  if (randfloat () < NEAT::mutate_random_trait_prob)
	    {
	      //std::cout << "Mutate random trait" << std::endl;
	      new_genome->mutate_random_trait ();
	    }
	  if (randfloat () < NEAT::mutate_link_trait_prob)
	    {
	      //std::cout << "Mutate_link_trait" << std::endl;
	      new_genome->mutate_link_trait (1);
	    }
	  if (randfloat () < NEAT::mutate_node_trait_prob)
	    {
	      //std::cout << "Mutate_node_trait" << std::endl;
	      new_genome->mutate_node_trait (1);
	    }
	  if (randfloat () < NEAT::mutate_link_weights_prob)
	    {
	      //std::cout << "Mutate_link_weights" << std::endl;
	      new_genome->mutate_link_weights (mut_power, 1.0, GAUSSIAN);
	    }
	  if (randfloat () < NEAT::mutate_toggle_enable_prob)
	    {
	      //std::cout << "Mutate toggle enable" << std::endl;
	      new_genome->mutate_toggle_enable (1);
	    }
	  if (randfloat () < NEAT::mutate_gene_reenable_prob)
	    {
	      //std::cout << "Mutate gene reenable" << std::endl;
	      new_genome->mutate_gene_reenable ();
	    }
	}

      _genome = new_genome;
    }


  createNN ();

  // Logging
  std::string s = std::string ("");
  s +=
    "{" + std::to_string (gWorld->getIterations ()) + "} [" +
    std::to_string (_wm->getId ()) + "::" + std::to_string (_birthdate) +
    "] [sigma=" + std::to_string (_currentSigma) + "]\n";
  gLogManager->write (s);
  gLogManager->flush ();
}



void
TopEDOController::broadcastGenome ()
{
  // only if agent is active (ie. not just revived) and deltaE>0.
  if (_wm->isAlive () == true)
    {
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
	      float sigmaSendValue = _currentSigma;

	      // other agent stores my genome.

	      targetRobotController->storeGenome (_genome, _wm->getId (),
						  _birthdate, sigmaSendValue,
						  _currentFitness);

	    }
	}
    }
}

void
TopEDOController::loadNewGenome ()
{
  // Logging
  std::string s =
    "{" + std::to_string (gWorld->getIterations ()) + "} [" +
    std::to_string (_wm->getId ()) + "::" + std::to_string (_birthdate) +
    "] [energy:" + std::to_string (_wm->getEnergyLevel ()) +
    "] [genomeList:" + std::to_string (_genomesList.size ()) + "]\n";
  gLogManager->write (s);
  gLogManager->flush ();

  _genomesList[_wm->getId ()] = _genome;
  _fitnessList[_wm->getId ()] = _currentFitness;
  _sigmaList[_wm->getId ()] = _currentSigma;
  _birthdateList[_wm->getId ()] = _birthdate;

  int selected;

  if (_genomesList.size () > 0)
    {

      selected = selectBest (_fitnessList);
      _genome = _genomesList[selected];
      _currentFitness = _fitnessList[selected];

      mutate (_currentSigma);

      _currentFitness = 0.0;
      setNewGenomeStatus (true);
      _birthdate = gWorld->getIterations ();

      _genomesList.clear ();
      _fitnessList.clear ();
      _sigmaList.clear ();
      _birthdateList.clear ();

      //  _wm->setRobotLED_colorValues(rand()%255, 0, 0);
    }

  // log the genome

  s = std::string ("");
  s +=
    "{" + std::to_string (gWorld->getIterations ()) + "} [" +
    std::to_string (_wm->getId ()) + "::" + std::to_string (_birthdate) +
    "] new_genome: ";

  //TODO: adapt printing to NEAT genomes
  /* for(unsigned int i=0; i<_genome.size(); i++)
     {
     s += std::to_string(_genome[i]) + " ";     
     } */
  s += "\n";
  gLogManager->write (s);
  gLogManager->flush ();

}

int
TopEDOController::selectBest (std::map < int, float >lFitness)
{
  int idx = -1;
  float bestFit = -1000000;
  std::map < int, float >::iterator it;
  for (it = lFitness.begin (); it != lFitness.end (); it++)
    {
      if (it->second > bestFit)
	{
	  bestFit = it->second;
	  idx = it->first;
	}

    }
  return idx;
}
