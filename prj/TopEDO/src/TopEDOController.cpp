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
  _isFixedTopo = false;

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
  if(!_isFixedTopo)
    _genomesList.clear ();
  else
    {
      _genomesFList.clear();
      
      _previousOut.clear();
      //Fill activation for recurrent connections
      for(unsigned int i = 0; i < _nbOutputs; i++)
	{
	  _previousOut.push_back(0.0);
	}
    }

  _fitnessList.clear ();
  _sigmaList.clear ();
  _birthdateList.clear ();

}

void
TopEDOController::initRobot ()
{
  _nbInputs = 1;		// Bias constant input (1.0)

  if ((gExtendedSensoryInputs) && (TopEDOSharedData::gFitness == 0))
    {
      _nbInputs += (1) * _wm->_cameraSensorsNb;	// Switch
    }

  _nbInputs += _wm->_cameraSensorsNb;	// proximity sensors

  _nbOutputs = 2;
  unsigned int nbWeights;
  if(!_isFixedTopo)
    {
      // Inputs, outputs, 0 hidden neurons, fully connected. Start with Simple Perceptron 
      _genome = new GenomeAdapted (_nbInputs, _nbOutputs, 0, 0);
      
      _genome->setIdTrace (_wm->getId ());
      _genome->setMom (-1);
      _genome->setDad (-1);
      _genome->genome_id = _wm->getId ();
      
      _genome->mutate_link_weights (1.0, 1.0, COLDGAUSSIAN);
      createNN ();

    }
  else
    {
       //Single layer perceptron with recurrent connections of the outputs
      nbWeights = _nbInputs * _nbOutputs + _nbOutputs * _nbOutputs;
      for(unsigned int i = 0; i < nbWeights; i++ )
	{
	  _genomeF.push_back(gaussrand());
	}
      
      _previousOut.clear();
      //Fill activation for recurrent connections
      for(unsigned int i = 0; i < _nbOutputs; i++)
	{
	  _previousOut.push_back(0.0);
	}
    }


  if (gVerbose)
    std::cout << std::flush;

  setNewGenomeStatus (true);
  if(!_isFixedTopo)
    {
      _genomesList.clear ();
      //TOFIX NEAT-like innovation number and node id FOR THIS ROBOT
      innovNumber = (double) nn->linkcount ();
      nodeId = 1 + _nbInputs + _nbOutputs;
    }
  else
    {
      _genomesFList.clear();
    }

  _fitnessList.clear ();
  
 
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
  std::pair<std::vector<double>,std::vector<double>> io ;
  if(!_isFixedTopo)
      io = act();
  else
    io = actFTopo();

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
      
      if (gExtendedSensoryInputs && (TopEDOSharedData::gFitness == 0))
	{
	  int objectId = _wm->getObjectIdFromCameraSensor (i);
	  
	  // input: physical object? which type?
	  if (PhysicalObject::isInstanceOf (objectId))
	    {
	      //Switch is type 3
	      if ((gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]
		   ->getType ()) == 3)
		{
		  inputs[inputToUse] = 	_wm->getDistanceValueFromCameraSensor (i) /
		    _wm->getCameraSensorMaximumDistanceValue (i);//Match
		}
	      else
		inputs[inputToUse] = 1.0;
	      inputToUse++;
	    }	 
	  else //Not physical object
	    {	      
	      inputs[inputToUse] = 1.0;
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

std::pair<std::vector<double>,std::vector<double>> TopEDOController::actFTopo()
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
      
      if (gExtendedSensoryInputs && (TopEDOSharedData::gFitness == 0))
	{
	  int objectId = _wm->getObjectIdFromCameraSensor (i);
	  
	  // input: physical object? which type?
	  if (PhysicalObject::isInstanceOf (objectId))
	    {
	      //Switch is type 3
	      if ((gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]
		   ->getType ()) == 3)
		{
		  inputs[inputToUse] = 	_wm->getDistanceValueFromCameraSensor (i) /
		    _wm->getCameraSensorMaximumDistanceValue (i);//Match
		}
	      else
		inputs[inputToUse] = 1.0;
	      inputToUse++;
	    }	 
	  else //Not physical object
	    {	      
	      inputs[inputToUse] = 1.0;
	      inputToUse++;
	    }

	}
    }
  
  
  std::vector < double> outputs;
  int idxGenome = 0;
  double aux;
  
  //Computing activation for fully connected sigle-layer perceptron 
  //with recurrent connections of the outputs
  for(unsigned int j = 0; j < _nbOutputs; j++)
    {
      aux = 0.0;
      for(unsigned int i = 0; i < _nbInputs; i++)
	{
	  aux += inputs[i] * _genomeF[idxGenome++];	  
	}
      for(unsigned int i = 0; i < _nbOutputs; i++)
	{
	  aux += _previousOut[i] * _genomeF[idxGenome++];
	}
	  //Using sigmoid function as in NEAT
	  //Second constant is not used, 
	  //first constant is the slope for sigmoid activation function
	  outputs.push_back(NEAT::fsigmoid(aux,4.924273,2.4621365));
    }
  _previousOut = outputs;
 
  return std::make_pair(inputs,outputs);
}


float
TopEDOController::updateFitness (std::vector < double >in,
				 std::vector < double >out)
{
  float deltaFit = 0.0;
  int targetIndex;
  double vT,vR,minSensor;


  switch(TopEDOSharedData::gFitness)
    {
    case 0:
      targetIndex = _wm->getGroundSensorValue ();
      
      if (PhysicalObject::isInstanceOf (targetIndex))
	{
	  if((gPhysicalObjects[targetIndex - gPhysicalObjectIndexStartOffset]
	      ->getType ()) == 3)
	    deltaFit += 1.0;
	}
      break;
    case 1:
      //[Floreano2000] locomotion fitness function
      //Translational speed * (1 - abs(Rotational Speed)) * minimal(distance to obstacle)
      vT = out[0];
      vR =  2 * (out[1] - 0.5);

      minSensor = in[0];
      
      for (int i = 0; i < _wm->_cameraSensorsNb; i++)
	{
	  if(in[i + 1] < minSensor)
	    minSensor = in[i + 1];
	}
      deltaFit += vT * (1 - fabs(vR)) * minSensor;
      
      break;

    default:
      std::cerr << "[ERROR] Unknown fitness function selected. Check gFitness parameter in properties file." << std::endl;
      exit(-1);
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
	  if(!_isFixedTopo)
	    {
	      targetRobotController->
		storeGenome (_genome, _wm->getId (), _birthdate,
			     _currentFitness / (gWorld -> getIterations() + 1 - _birthdate),
					      _currentFitness);
	    }
	  else
	    {
	        targetRobotController->
		storeGenomeF (_genomeF, _wm->getId (), _birthdate,
			     _currentFitness / (gWorld -> getIterations() + 1 - _birthdate),
					      _currentFitness);
	    }
	    
	  
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

void
  TopEDOController::storeGenomeF (std::vector<double> genome, int senderId,
			       int senderBirthdate, float sigma,
			       float fitness)
{
  //08/10/14 (storeGenome Adaptedto NEAT, I think)
  _genomesFList[senderId] = genome;
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
    if(!_isFixedTopo)
      	_genomesList[_wm->getId ()] = _genome;
    else
      _genomesFList[_wm->getId()] = _genomeF;
      
    _fitnessList[_wm->getId()] = _currentFitness / (gWorld -> getIterations() + 1 - _birthdate);
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
    
    if((!_isFixedTopo) && (_genomesList[selected] == NULL))
      {
	std::cerr << "[ERROR] Selected genome not existing: " << selected << " in robot " << _wm->getId() << std::endl;
	exit(-1);
      }
    if((_isFixedTopo) && (_genomesFList.find(selected) == _genomesFList.end()))
      {
	std::cerr << "[ERROR] Selected fixed-topology genome not existing: " << selected << " in robot " << _wm->getId() << std::endl;
	exit(-1);
      }
      
    if(!_isFixedTopo)
      _genome = _genomesList[selected];
    else
      _genomeF = _genomesFList[selected];
    
    _currentFitness = _fitnessList[selected];
   
    
    int newId =
	_wm->getId () + 10000 * (1 +
				 (gWorld->getIterations () /
				  TopEDOSharedData::gEvaluationTime));
    if(!_isFixedTopo)
      {
	_genome = _genome -> mutate (_currentSigma,
				 _wm->getId (), newId, nodeId, innovNumber);
	createNN ();
      }
    else
      _genomeF = mutateF(_genomeF,_currentSigma);
    
}

 std::vector<double> TopEDOController::mutateF(std::vector<double> g, float sigma)
  {
    std::vector<double> result;
    
    for(unsigned int i = 0; i < g.size(); i ++ )
      {
	result.push_back( g[i] + sigma * gaussrand());
      }
    return result;
  }


void TopEDOController::logGenome()
{
   if(!_isFixedTopo)
      {
	//GENERATION ID-ROBOT FITNESS IDGENOME IDMOM
	TopEDOSharedData::gEvoLog << (gWorld->getIterations () /
				      TopEDOSharedData::
				      gEvaluationTime) << " " << _wm->getId () << " " <<
	  _currentFitness / (gWorld -> getIterations() + 1 - _birthdate) << " " << _genome->getIdTrace () << " " << _genome->
	  getMom () << std::endl;
      }
   else
     {
       //GENERATION ID-ROBOT FITNESS 
	TopEDOSharedData::gEvoLog << (gWorld->getIterations () /
				      TopEDOSharedData::
				      gEvaluationTime) << " " << _wm->getId () << " " <<
	  _currentFitness / (gWorld -> getIterations() + 1 - _birthdate) 
	  << std::endl;
     }
  
  
  std::string filename; 
  if(!_isFixedTopo)
    {
      filename = TopEDOSharedData::gGenomeLogFolder;
      filename += std::to_string(_genome -> getIdTrace());
      
      _genome -> print_to_filename(const_cast<char*>(filename.c_str()));
    }
  
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
  std::vector<std::pair<int,float>> rankedFitness;
  int result = -1;
  int idx = 1, totalIndex = 0;
    
  //Compute the total sum of indexes
  for (it->first; it != lFitness.end (); it++)
    {
      totalIndex += idx;
      idx++;
      rankedFitness.push_back(std::make_pair(it->first,it->second));
    }

  int random = rand() * totalIndex;

  //Fitness ascending order
  std::sort(rankedFitness.begin(), rankedFitness.end(), compareFitness);

  std::vector<std::pair<int,float>>::iterator iter = rankedFitness.begin();

  idx = 1;
  iter = rankedFitness.begin();
  while((random > 0) && (iter != rankedFitness.end()))
    {
      iter++;
      random -= (iter ->first);
    }

  if(iter != rankedFitness.end())
    result = iter->first;
  else
    {
      iter--;
      result = iter->first;
    }
  return result;
}
int
TopEDOController::selectBinaryTournament(std::map < int, float >lFitness)
{
  int result = -1;

  std::vector<int> v;

  //Vector for storing the keys (robot ID)
  for(std::map<int,float>::iterator it = lFitness.begin(); it != lFitness.end(); ++it) {
    v.push_back(it->first);
  }
  
  if(lFitness.size() > 1)
    {
      int ind1 =  rand () % lFitness.size ();
      ind1 = v[ind1];
      int ind2 =  rand () % lFitness.size ();
      ind2 = v[ind2];
      
      while(ind1 == ind2)
	{
	  ind2 =  rand () % lFitness.size ();
	  ind2 = v[ind2];	  
	}
      
      if(lFitness[ind1] >= lFitness[ind2])
	result = ind1;
      else
	result = ind2;
    }
  else 
    result = lFitness.begin()->first;

  if(result == -1)
    {
      std::cerr << "[ERROR] No individual selected by binary tournament." << std::endl << std::flush;
      exit(-1);
    }

  if(lFitness.find(result) == lFitness.end())
    {
      std::cerr << "[ERROR] Unexisting individual selected by binary tournament." << std::endl << std::flush;
      exit(-1);
    }
  
  return result;
}
int
TopEDOController::selectRandom(std::map < int, float >lFitness)
{      
  std::vector<int> v;
  //Vector for storing the keys (robot ID)
  for(std::map<int,float>::iterator it = lFitness.begin(); it != lFitness.end(); ++it) {
    v.push_back(it->first);
  } 

  return  v[rand () % v.size ()];
  
}

bool TopEDOController::compareFitness(std::pair<int,float> i,std::pair<int,float> j) 
{
  return (i.second < j.second); 
}
