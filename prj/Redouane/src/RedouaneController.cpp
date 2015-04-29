/**
 * @author Nicolas Bredeche <nicolas.bredeche@upmc.fr>
 * NNlib: Leo Cazenille <leo.cazenille@upmc.fr>
 */

#include "Redouane/include/RedouaneController.h"
#include "Redouane/include/RedouaneWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <string>

#include <neuralnetworks/MLP.h>
#include <neuralnetworks/Perceptron.h>
#include <neuralnetworks/Elman.h>

using namespace Neural;

RedouaneController::RedouaneController( RobotWorldModel *wm )
{
  
  _wm = wm;
  
  nn = NULL;
  
  // evolutionary engine
  
  _minValue = -1.0;
  _maxValue = 1.0;
  
  _currentSigma = RedouaneSharedData::gSigmaRef;
  
  resetRobot();
  
  // behaviour
  
  _iteration = 0;
  
  _birthdate = 0;
  
  _lastSwitch = -1;
  
  if ( gEnergyLevel )
    _wm->setEnergyLevel(gEnergyInit);
  
  _wm->updateLandmarkSensor();
  
  _wm->setAlive(true);
  _wm->setRobotLED_colorValues(255, 0, 0);

  //_behavior =(* new std::vector<std::pair<std::vector<double>,std::vector<double>>>(0));
}

RedouaneController::~RedouaneController()
{
  _parameters.clear();
  delete nn;
  nn = NULL;
}

void RedouaneController::reset()
{
  _parameters.clear();
  _parameters = _genome;
  _currentFitness = 0.0;
  _behavior.clear();
}

void RedouaneController::resetRobot()
{
  _nbInputs = 1; // Bias constant input (1.0)


  if ( gExtendedSensoryInputs ) 
    {
       _nbInputs += ( 1 ) * _wm->_cameraSensorsNb; // CoupledSwitch 
      //_nbInputs = ( PhysicalObjectFactory::getNbOfTypes()+3+1 ) * _wm->_cameraSensorsNb; // nbOfTypes + ( isItAnAgent? + isItSameGroupId? + agentAngleDifference?) + isItAWall?
      }
        
  _nbInputs += _wm->_cameraSensorsNb; // proximity sensors

  //_nbInputs +=  3; // ground sensor (3 values)

    if ( gEnergyLevel )
      _nbInputs += 1; // incl. energy level

    //if ( gLandmarks.size() > 0 )
    //_nbInputs += 2; // incl. landmark (angle,dist)
    
  _nbOutputs = 2;

  if ( RedouaneSharedData::gEnergyRequestOutput )
    _nbOutputs += 1; // incl. energy request


  _nbHiddenLayers = RedouaneSharedData::gNbHiddenLayers;


  //_nbNeuronsPerHiddenLayer = new std::vector<unsigned int>(_nbHiddenLayers);

  for(unsigned int i = 0; i < _nbHiddenLayers; i++)
    _nbNeuronsPerHiddenLayer[i] = RedouaneSharedData::gNbNeuronsPerHiddenLayer;

  createNN();
  
 

  unsigned int const nbGene = computeRequiredNumberOfWeights();
  
  if ( gVerbose )
    std::cout << _wm->_cameraSensorsNb<< std::flush ;
  
  _genome.clear();
  
  /* for ( unsigned int i = 0 ; i != nbGene ; i++ )
    {
      _genome.push_back((double)(rand()%RedouaneSharedData::gNeuronWeightRange)/(RedouaneSharedData::gNeuronWeightRange/2)-1.0); // weights: random init between -1 and +1
      }*/
  _genome.push_back(2.0); // Biais -> LW
  _genome.push_back(-1.0); // S1 -> LW
  _genome.push_back(0.0); // Bidouille
  _genome.push_back(1.0); //s2 -> LW
  _genome.push_back(0.0); // Bidouille
  _genome.push_back(2.0); // Biais -> RW
  _genome.push_back(1.0); // S1 -> RW
  _genome.push_back(0.0); // Bidouille
  _genome.push_back(-1.0); // S2 -> RW
  _genome.push_back(0.0); // Bidouille
  
  _currentGenome = _genome;
  setNewGenomeStatus(true);
  _genomesList.clear();
  _fitnessList.clear();
  
  _genomesList[0] = _genome;
  _fitnessList[0] = -10000.0;
 
  std::cout << "NbIn: " << _nbInputs << " - NbOut: " << _nbOutputs << " - NbGenes: " << nbGene << std::endl ;
  
}



void RedouaneController::step()
{
  _iteration++;
  stepEvolution();
 
  stepBehaviour();
 
}



// ################ ######################## ################
// ################ BEHAVIOUR METHOD(S)      ################
// ################ ######################## ################

void RedouaneController::stepBehaviour()
{
  //int objId = _wm->getGroundSensorValue();
  //objId = objId - gPhysicalObjectIndexStartOffset;
   //std::cout << "Obj Id " << objId << std::endl << std::flush;

  // pre-compute closest landmark information (if needed)
    if ( gLandmarks.size() > 0 )
    {
      _wm->updateLandmarkSensor();
    }
    
  // ---- Build inputs ----
  
  std::vector<double>* inputs = new std::vector<double>(_nbInputs);
  int inputToUse = 0;
 
  (*inputs)[inputToUse++] = 1.0;

  // distance sensors
  for(int i  = 0; i < _wm->_cameraSensorsNb; i++)
    {
      (*inputs)[inputToUse] = _wm->getDistanceValueFromCameraSensor(i) / _wm->getCameraSensorMaximumDistanceValue(i);
      inputToUse++;
      
      if ( gExtendedSensoryInputs ) 
        {
	  int objectId = _wm->getObjectIdFromCameraSensor(i);
          
	  // input: physical object? which type?
            if ( PhysicalObject::isInstanceOf(objectId) )
	      {
		int nbOfTypes = 5;//PhysicalObjectFactory::getNbOfTypes();
		 for ( int i = 4 ; i != nbOfTypes ; i++ )
		  {
		    // std::cout <<"Object ID  : " << ( objectId - gPhysicalObjectIndexStartOffset ) <<  std::endl;
		    // CoupledSwitch* phObj = (CoupledSwitch*) gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset];
		    //std::cout << "Type: " << gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]->getType() << std::endl << std::flush;
                    if (i  == ( gPhysicalObjects[objectId - gPhysicalObjectIndexStartOffset]->getType()))
		      {
			(*inputs)[inputToUse] = 1; // match
		      }
                    else
		      (*inputs)[inputToUse] = 0;
		      inputToUse++;
		  }
	      }
            else
	      {
                // not a physical object.Should still fill in the inputs (with zeroes)
		int nbOfTypes = 1;
		//int nbOfTypes = PhysicalObjectFactory::getNbOfTypes();
                for ( int i = 0 ; i != nbOfTypes ; i++ )
		  {
                    (*inputs)[inputToUse] = 0;
		      inputToUse++;
		  }
	      }
	    
            // input: another agent? If yes: same group?
            /*if ( Agent::isInstanceOf(objectId) )
	      {
                // this is an agent
                (*inputs)[inputToUse] = 1;
                inputToUse++;
		
                // same group?
                if ( gWorld->getRobot(objectId-gRobotIndexStartOffset)->getWorldModel()->getGroupId() == _wm->getGroupId() )
		  {
                    (*inputs)[inputToUse] = 1; // match: same group
		  }
                else
		  {
                    (*inputs)[inputToUse] = 0; // not the same group
		  }
                inputToUse++;
                
                // relative orientation? (ie. angle difference wrt. current agent)
                double srcOrientation = _wm->_agentAbsoluteOrientation;
                double tgtOrientation = gWorld->getRobot(objectId-gRobotIndexStartOffset)->getWorldModel()->_agentAbsoluteOrientation;
                double delta_orientation = - ( srcOrientation - tgtOrientation );
                if ( delta_orientation >= 180.0 )
                    delta_orientation = - ( 360.0 - delta_orientation );
                else
		  if ( delta_orientation <= -180.0 )
		    delta_orientation = - ( - 360.0 - delta_orientation );
                (*inputs)[inputToUse] = delta_orientation/180.0;
                inputToUse++;
                
            }
            else
            {*/
	      //(*inputs)[inputToUse] = 0; // not an agent...
	      //inputToUse++;
	      //(*inputs)[inputToUse] = 0; // ...therefore no match wrt. group.
	      //inputToUse++;
	      ///*
	      //(*inputs)[inputToUse] = 0; // ...and no orientation.
	      //inputToUse++;
	      //*/
            //}
            
            // input: wall or empty?
	    // if ( objectId >= 0 && objectId < gPhysicalObjectIndexStartOffset ) // not empty, but cannot be identified: this is a wall.
		 //  (*inputs)[inputToUse] = 1;
            //else
	    //  (*inputs)[inputToUse] = 0; // nothing. (objectId=-1)
            //inputToUse++;
        }
    }
    
    // floor sensor
    //(*inputs)[inputToUse++] = (double)_wm->getGroundSensor_redValue()/255.0;
    //(*inputs)[inputToUse++] = (double)_wm->getGroundSensor_greenValue()/255.0;
    //(*inputs)[inputToUse++] = (double)_wm->getGroundSensor_blueValue()/255.0;
    
    // closest landmark (if exists)
    if ( gLandmarks.size() > 0 )
      {
        (*inputs)[inputToUse++] = _wm->getLandmarkDirectionAngleValue();
        (*inputs)[inputToUse++] = _wm->getLandmarkDistanceValue();
      }
    
    // energy level
    if ( gEnergyLevel )
      {
        (*inputs)[inputToUse++] = _wm->getEnergyLevel() / gEnergyMax;
      }
    
    // ---- compute and read out ----
    
    nn->setWeigths(_parameters); // create NN
    
    nn->setInputs(*inputs);
        
    nn->step();
    
    std::vector<double> outputs = nn->readOut();
    
    // _wm->_desiredTranslationalValue = outputs[0];
    //_wm->_desiredRotationalVelocity = outputs[1];
    
    if ( RedouaneSharedData::gEnergyRequestOutput )
      {
        _wm->setEnergyRequestValue(outputs[2]);
      }
    
      _wm->_desiredTranslationalValue = 2 * (outputs[0] - 0.5);
    //Rotational velocity in [-1,+1]. Neat uses sigmoid [0,+1]
    _wm->_desiredRotationalVelocity = 2 * (outputs[1] - 0.5);
    // normalize to motor interval values
    _wm->_desiredTranslationalValue = _wm->_desiredTranslationalValue * gMaxTranslationalSpeed;
    _wm->_desiredRotationalVelocity = _wm->_desiredRotationalVelocity * gMaxRotationalSpeed;
    
    _currentFitness += updateFitness(*inputs,outputs);
    storeBehavior(*inputs,outputs);
    delete (inputs);
}
void RedouaneController::storeBehavior(std::vector<double> in,std::vector<double> out)
{
  std::pair<std::vector<double>,std::vector<double> > currentBehav;
  currentBehav = std::make_pair(in,out);
  _behavior.push_back(currentBehav);
  
}

float RedouaneController::updateFitness(std::vector<double> in,std::vector<double> out)
{
  float deltaFit = 0.0;
  int targetIndex = _wm->getGroundSensorValue();
 
  if ( PhysicalObject::isInstanceOf(targetIndex) ) 
    {
      deltaFit += 1.0;
      targetIndex = targetIndex - gPhysicalObjectIndexStartOffset;
      _lastSwitch = targetIndex;
    }
    
  /* double red = in[13];
  double green = in[14];
  double blue = in[15];
  if((green >= 1.0) && (red < 1.0) && (blue < 1.0) )
    deltaFit = 100;
  else
  deltaFit = green - (red + blue)/2;*/
  return deltaFit;
  /*
  int targetIndex = _wm->getGroundSensorValue();
  if(_wm -> getXReal() > 800)
    {
      deltaFit += 100 * out[0]*out[1];
      return deltaFit;
    }
  else
  if ( PhysicalObject::isInstanceOf(targetIndex) ) 
    {
      deltaFit += 10;
      return deltaFit;
    }
  else 
    {
      deltaFit += 1 - (dist(_wm -> getXReal() / ((float) gScreenWidth), _wm -> getYReal() / ((float) gScreenHeight), 800.0 / ((float) gScreenWidth), 750.0 / ((float) gScreenHeight)))/sqrt(2);
      //std::cout << 1 - deltaFit << std::endl;
      return deltaFit/100;
      }


  if(_wm -> getXReal() > 800)
    deltaFit += 2;
  else
    deltaFit += 1 
                - (dist(
			_wm -> getXReal() / ((float) gScreenWidth), _wm -> getYReal() / ((float) gScreenHeight), 
			800.0 / ((float) gScreenWidth), 750.0 / ((float) gScreenHeight)))/sqrt(2);
  
  // std::cout << "Width: " << gScreenWidth << " - Height: " << gScreenHeight << std::endl;
  // std::cout << "Position: (" << _wm -> getXReal() << " , " << _wm -> getYReal() << ")" << std::endl << "DistToDoor: " << dist(_wm -> getXReal() / ((float) gScreenWidth), _wm -> getYReal() / ((float) gScreenHeight), 800.0 / ((float) gScreenWidth), 750.0 / ((float) gScreenHeight)) << std::endl;
  // std::cout << 1 - deltaFit << std::endl;
  return deltaFit;*/
}

float RedouaneController::dist(float x1, float y1, float x2, float y2)
{
  float result = 0.0;
  result = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  return result;
}

void RedouaneController::createNN()
{
  if ( nn != NULL ) // useless: delete will anyway check if nn is NULL or not.
    delete nn;
  
  switch ( RedouaneSharedData::gControllerType )
    {
    case 0:
      {
	// MLP
	nn = new MLP(_parameters, _nbInputs, _nbOutputs, _nbNeuronsPerHiddenLayer);
	break;
      }
    case 1:
      {
	// PERCEPTRON
	nn = new Perceptron(_parameters, _nbInputs, _nbOutputs);
	break;
      }
    case 2:
      {
	// ELMAN
	nn = new Elman(_parameters, _nbInputs, _nbOutputs, _nbNeuronsPerHiddenLayer);
	break;
      }
    default: // default: no controller
      std::cerr << "[ERROR] gController type unknown (value: " << RedouaneSharedData::gControllerType << ").\n";
      exit(-1);
    };
}


unsigned int RedouaneController::computeRequiredNumberOfWeights()
{
    unsigned int res = nn->getRequiredNumberOfWeights();
    return res;
}

// ################ ######################## ################
// ################ ######################## ################
// ################ EVOLUTION ENGINE METHODS ################
// ################ ######################## ################
// ################ ######################## ################

void RedouaneController::stepEvolution()
{
  // * broadcasting genome : robot broadcasts its genome to all neighbors (contact-based wrt proximity sensors)
  /*if  ( gRadioNetwork )
    {
    broadcastGenome();
    }*/
  
  // * lifetime ended: replace genome (if possible)
  if( dynamic_cast<RedouaneWorldObserver*>(gWorld->getWorldObserver())->getLifeIterationCount() >= RedouaneSharedData::gEvaluationTime-1 )
    {
      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
     
      if(r < RedouaneSharedData::gProbReEval)
	{
           //Reevaluate champion
           //std::cout << "Reevaluate!!! (Fit: " << _currentFitness << ")" << std::endl;
	  if(_lastSwitch == 0)
	    _currentFitness += 1 - dist(_wm->getXReal(),_wm->getYReal(),100,505)/941;
	  else
	    if(_lastSwitch == 1)
	      _currentFitness += 1 - dist(_wm->getXReal(),_wm->getYReal(),624,153)/941;
	    else
	      _currentFitness += 1 - std::min(dist(_wm->getXReal(),_wm->getYReal(),624,153),dist(_wm->getXReal(),_wm->getYReal(),100,505))/941 ; //Normalized distance to closest switch (change location here if changed in properties file) 

	  std::cout << _currentFitness << std::endl;
           _fitnessList[0] = _currentFitness;
	   _currentFitness = 0.0;	  
        }
      else
	{
	  //feedBehaviorBase(_behavior);
	  // std::cout << "Fitness: " <<  _currentFitness << std::endl;
	  //_currentFitness += 1 - std::min(dist(_wm->getXReal(),_wm->getYReal(),624,153),dist(_wm->getXReal(),_wm->getYReal(),100,505))/941 ; //Normalized distance to closest switch (change location here if changed in properties file) 
	  if(_lastSwitch == 0)
	      _currentFitness += 1 - dist(_wm->getXReal(),_wm->getYReal(),100,505)/941;
	  else
	    if(_lastSwitch == 1)
	    _currentFitness += 1 - dist(_wm->getXReal(),_wm->getYReal(),624,153)/941;
	    else
	      _currentFitness += 1 - std::min(dist(_wm->getXReal(),_wm->getYReal(),624,153),dist(_wm->getXReal(),_wm->getYReal(),100,505))/941 ; //Normalized distance to closest switch (change location here if changed in properties file) 


	  std::cout << _currentFitness << std::endl;
	   
	  loadNewGenome();
	}

    }
  
  if ( getNewGenomeStatus() ) // check for new NN parameters
    {
      reset();
      setNewGenomeStatus(false);
    }
}

void RedouaneController::feedBehaviorBase(std::vector<std::pair< std::vector<double>, std::vector<double> > > b)
{
  double minDist = 1000000.0;
  int idx = -1;
  unsigned int i = 0;
  for(i = 0; i < _behBase.size() ; i++)
    {
      if(dist(_behBase[i],b) < minDist)
	{
	  minDist = dist(_behBase[i],b);
	  idx = i;
	}
    }
   if(minDist < RedouaneSharedData::gBehThresh)
    {
      _behBaseCounter[idx] +=1;
    }
  else
    {
      _behBase[i] = b;
      _behBaseCounter[i] = 1;
    }
   //_currentFitness = minDist;
   //std::cout << "Novelty: " << _currentFitness << std::endl;

   // std::cout << "Size of the behavior base: " << _behBase.size() << std::endl;
}

double RedouaneController::dist(std::vector<std::pair< std::vector<double>, std::vector<double> > > b1, std::vector<std::pair< std::vector<double>, std::vector<double> > > b2)
{
  if(b1.size() != b2.size())
    {
      //std::cout << "[ERROR] Behaviors of different sizes: b1 (" << b1.size() << "), b2 (" << b2.size() << ")"<< std::endl;
      //exit(-1);
    }
  double res = 0.0;
  double aux = 0.0;
  
  for(unsigned int i = 0; i < std::min(b1.size(),b2.size()); i++)
    {
      aux = 0.0;
      
      for(unsigned int j = 0; j < b1[0].first.size(); j++)
	{
	  aux += (b1[i].first[j] - b2[i].first[j]) * (b1[i].first[j] - b2[i].first[j]);
	}
     for(unsigned int j = 0; j < b1[0].second.size(); j++)
	{
	  aux += (b1[i].second[j] - b2[i].second[j]) * (b1[i].second[j] - b2[i].second[j]);
	}
     res += sqrt(aux);
    }

  // std::cout << "Distance between behaviors: " << res << std::endl;

  return res;
}

void RedouaneController::selectRandomGenome()
{
  if(_genomesList.size() != 0)
    {
      int randomIndex = rand()%_genomesList.size();
      std::map<int, std::vector<double> >::iterator it = _genomesList.begin();
      while (randomIndex !=0 )
        {
	  it ++;
	  randomIndex --;
        }
      
      _currentGenome = (*it).second;
      
      mutate(_sigmaList[(*it).first]);
      
      setNewGenomeStatus(true);
      
      _birthdate = gWorld->getIterations();
      
      // Logging
        std::string s = std::string("");
        s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] descends from [" + std::to_string((*it).first) + "::" + std::to_string(_birthdateList[(*it).first]) + "]\n";
        gLogManager->write(s);
        gLogManager->flush();
        
        _genomesList.clear();
    }
}


void RedouaneController::storeGenome(std::vector<double> genome, int senderId, int senderBirthdate, float sigma)
{
  /*_genomesList[senderId] = genome;
  _sigmaList[senderId] = sigma;
  _birthdateList[senderId] = senderBirthdate;*/
}


void RedouaneController::mutate( float sigma) // mutate within bounds.
{
  _genome.clear();
  
  _currentSigma = sigma;
  
  for (unsigned int i = 0 ; i != _currentGenome.size() ; i++ )
    {
      double value = _currentGenome[i] + getGaussianRand(0,_currentSigma);
      // bouncing upper/lower bounds
      if ( value < _minValue )
	{
	  double range = _maxValue - _minValue;
	  double overflow = - ( (double)value - _minValue );
	  overflow = overflow - 2*range * (int)( overflow / (2*range) );
	  if ( overflow < range )
	    value = _minValue + overflow;
	  else // overflow btw range and range*2
	    value = _minValue + range - (overflow-range);
	}
      else if ( value > _maxValue )
	{
	  double range = _maxValue - _minValue;
	  double overflow = (double)value - _maxValue;
	  overflow = overflow - 2*range * (int)( overflow / (2*range) );
	  if ( overflow < range )
	    value = _maxValue - overflow;
	  else // overflow btw range and range*2
	    value = _maxValue - range + (overflow-range);
	}
      
      _genome.push_back(value);
    }
  
  _currentGenome = _genome;
  
  // Logging
  std::string s = std::string("");
  s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] [sigma=" + std::to_string(_currentSigma) + "]\n";
  gLogManager->write(s);
  gLogManager->flush();
}



void RedouaneController::broadcastGenome()
{
}

void RedouaneController::loadNewGenome()
{
  // Logging
  std::string s = "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] [energy:" +  std::to_string(_wm->getEnergyLevel()) + "] [genomeList:" + std::to_string(_genomesList.size()) + "]\n";
  gLogManager->write(s);
  gLogManager->flush();
  
  //_genomesList.clear();
  _genomesList[1] = _currentGenome;
  _fitnessList[1] = _currentFitness;
 

  int selected;
      
    if (_genomesList.size() > 0)
      {
	
	//	std::cout << "Champion: " << 0 << " - Fitness: " << _fitnessList[0]  << std::endl;
	//std::cout << "Challenger: " << 1 << " - Fitness: " << _fitnessList[1]  << std::endl;
	
	
	
	selected = selectBest(_fitnessList);
	_currentGenome = _genomesList[selected];
	_currentFitness = _fitnessList[selected];

	/*s = std::string("");
      s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] new_genome: ";
      for(unsigned int i=0; i<_genome.size(); i++)
	{
	  s += std::to_string(_genome[i]) + " ";	
	}
      s += "\n";
      std::cout << s;*/

      _genomesList.clear();
      _fitnessList.clear();
      
      _genomesList[0] = _currentGenome;
      _fitnessList[0] = _currentFitness;
 
      if(selected == 0)
	{
	  _currentSigma = std::min((double)_currentSigma * 2, RedouaneSharedData::gSigmaMax);
	}
      else 
	if(selected == 1)
	{
	  _currentSigma = RedouaneSharedData::gSigmaMin;
	}
	else //DEBUG
	  {
	    std::cout << "[ERROR] (1 + 1)- Online - ES. (Only a parent and an offspring). Selected number " << selected << std::endl;
	    exit(-1);
	  }

      // std::cout << _currentSigma << std::endl;
      // std::cout << _currentFitness << std::endl;
     
      mutate(_currentSigma);
      _currentFitness = 0.0;
      setNewGenomeStatus(true);
      _birthdate = gWorld->getIterations();

      /* s = std::string("");
      s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] new_genome: ";
      for(unsigned int i=0; i<_genome.size(); i++)
	{
	  s += std::to_string(_genome[i]) + " ";	
	}
      s += "\n";

      std::cout << s;*/

        _wm->setRobotLED_colorValues(rand()%255, 0, 0);
	  
    }
  
  // log the genome
  
  s = std::string("");
  s += "{" + std::to_string(gWorld->getIterations()) + "} [" + std::to_string(_wm->getId()) + "::" + std::to_string(_birthdate) + "] new_genome: ";
  for(unsigned int i=0; i<_genome.size(); i++)
    {
	  s += std::to_string(_genome[i]) + " ";	
    }
  s += "\n";
  gLogManager->write(s);
  gLogManager->flush();
  
}

int RedouaneController::selectBest(std::map<int,float> lFitness)
{
  int idx = -1;
  float bestFit = -1000000;
  std::map<int, float>::iterator it;
  for(it = lFitness.begin(); it != lFitness.end(); it++ )
    {
      if(it->second > bestFit)
	{
	  bestFit = it->second;
	  idx = it->first;
	}
      
    }
  return idx;
}
