/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 * 
 */

#include "Simple/include/SimpleController.h"
#include "Simple/include/SimpleWorldObserver.h"

#include "World/World.h"
#include "Utilities/Misc.h"
#include <math.h>
#include <string>

#include <neuralnetworks/MLP.h>
#include <neuralnetworks/Perceptron.h>
#include <neuralnetworks/Elman.h>

#include <neat/genome.h>
#include "neat/genomeAdapted.h"

#include "Agents/Agent.h"

using namespace Neural;
using namespace NEAT;

SimpleController::SimpleController (RobotWorldModel * wm)
{

  _wm = wm;
 
  initRobot ();

}

SimpleController::~SimpleController ()
{
}

void
SimpleController::reset ()
{
  _currentFitness = 0.0;
  _iteration = 0;
  _evalTime = SimpleSharedData::gEvalTime - 0.25 * ranf() * SimpleSharedData::gEvalTime;
  _phase = ACT;
  
  _genomesList.clear();    
  _previousOut.clear();
  
  //Fill activation for recurrent connections
  for(unsigned int i = 0; i < _nbOutputs; i++)
    {
      _previousOut.push_back(0.0);
    }

  _fitnessList.clear ();
}

void
SimpleController::initRobot ()
{
   _currentSigma = SimpleSharedData::gSigma;

  _nbInputs = 1;		// Bias constant input (1.0)
  _nbInputs += _wm->_cameraSensorsNb;	// proximity sensors

  _nbOutputs = 2;
  unsigned int nbWeights;

  //Single layer perceptron
  //nbWeights = _nbInputs * _nbOutputs;
  //Single layer perceptron with recurrent connections of the outputs
  nbWeights = _nbInputs * _nbOutputs + _nbOutputs * _nbOutputs;
  for(unsigned int i = 0; i < nbWeights; i++ )
    {
      _genome.push_back(0.5 * gaussrand());
    }

  /*  _genome[_nbInputs] = 0.0;
  _genome[_nbInputs + 1] = 0.0;
  _genome[2 + 2 * _nbInputs] = 0.0;
  _genome[2 + 2 * _nbInputs + 1] = 0.0;*/
	 
  _previousOut.clear();
  //Fill activation for recurrent connections
  for(unsigned int i = 0; i < _nbOutputs; i++)
    {
      _previousOut.push_back(0.0);
    }

  _genomesList.clear();
  _fitnessList.clear (); 

  _phase = ACT;
  _evalTime = SimpleSharedData::gEvalTime - 0.25 * ranf() * SimpleSharedData::gEvalTime;
  _iteration = 0;
  _currentFitness = 0.0;
  
}

void
SimpleController::step ()
{
  _iteration++;
  if(_iteration == _evalTime)
      _phase = LISTEN;
  if(_phase == ACT)
    stepBehaviour ();
  else
    {
      _wm->_desiredTranslationalValue = 0.0;
      _wm->_desiredRotationalVelocity = 0.0;
    }
      
  if (_iteration ==  (_evalTime + SimpleSharedData::gListenTime))//SimpleSharedData::gEvalTime)
     {
       stepEvolution ();
       reset();
    }
}

void
SimpleController::stepBehaviour ()
{
  std::pair<std::vector<double>,std::vector<double>> io ;
  std::vector<double> outputs;
  std::vector < double >inputs;

  io = act(); 
  outputs= io.second;
  inputs= io.first;      

  _wm->_desiredTranslationalValue = outputs[0]* gMaxTranslationalSpeed;

  _wm->_desiredRotationalVelocity = outputs[1] * gMaxRotationalSpeed;
  
  _currentFitness += updateFitness (inputs, outputs);
  
  broadcastGenome ();
  
}

std::pair<std::vector<double>,std::vector<double>> SimpleController::act()
{
  
  std::vector < double >inputs(_nbInputs);
  int inputToUse = 0;
  
  inputs[inputToUse++] = 1.0;
  
  for (int i = 0; i < _wm->_cameraSensorsNb; i++)
    {
      inputs[inputToUse] =
	_wm->getDistanceValueFromCameraSensor (i) /
	_wm->getCameraSensorMaximumDistanceValue (i);
      inputToUse++;
    }
  
  std::vector < double> outputs;
  int idxGenome = 0;
  double aux;
  
   
  for(unsigned int j = 0; j < _nbOutputs; j++)
    {
      aux = 0.0;
      
      for(unsigned int i = 0; i < _nbInputs; i++)
	{
	  aux += inputs[i] * _genome[idxGenome++];	  
	  //std::cout << inputs[i] << " - ";
	}
      //std::cout << std::endl;
      for(unsigned int i = 0; i < _nbOutputs; i++)
	{
	  aux += _previousOut[i] * _genome[idxGenome++];
	}
      //Using tanh
      outputs.push_back(tanh(aux));
      
    }
  
  _previousOut.clear();
  _previousOut.push_back(outputs[0]);
  _previousOut.push_back(outputs[1]);

  return std::make_pair(inputs,outputs);
}

float
SimpleController::updateFitness (std::vector < double >in,
				 std::vector < double >out)
{
  float deltaFit = 0.0;
  double vT,vR,minSensor;
  
  vT = out[0];
  vR =  out[1];       

  minSensor = in[0];
 
  for (unsigned int i = 1; i < _nbInputs; i++)
    {
      if(in[i] < minSensor)
	minSensor = in[i];
    }
  
    deltaFit += fabs(vT) * (1 - fabs(vR)) * minSensor;
  //deltaFit += abs(vT) * (1 - abs(vR)) * minSensor;
  
  //std::cout << deltaFit << " - " << fabs(vT) << " - " << vR << " - " << minSensor << std::endl;
 
  return deltaFit;
}

void
SimpleController::broadcastGenome ()
{
  int x,y;
  //Not used now (communication via sensors)
  if(true)
    {
      for (int i = 0; i < _wm->_cameraSensorsNb; i++)
	{
	  int targetIndex = _wm->getObjectIdFromCameraSensor (i);
	  
	  if (targetIndex >= gRobotIndexStartOffset)
	    {
	      targetIndex = targetIndex - gRobotIndexStartOffset;
	      SimpleController *targetRobotController =
		dynamic_cast <SimpleController *>
		(gWorld->getRobot (targetIndex)->getController ());
	      if((targetRobotController -> _phase) == LISTEN)
		targetRobotController->
		  storeGenome (_genome, _wm->getId (), _currentFitness);
	      
	    }
	}
    }
  else
    {   
      //      std::vector<Agent>* agents 
      int nbR= gWorld->getNbOfRobots();
      
      Robot* agent;
      for(int i = 0; i < nbR; i++)
	{
	  agent =gWorld -> getRobot(i);
	  agent ->getCoord(x,y);
	  if(dist(x,y,_wm ->getXReal(),_wm -> getYReal()) < 20)
	    {
	      SimpleController* targetRobotController = static_cast<SimpleController*>(agent->getController());
	      if((targetRobotController -> _phase) == LISTEN)
		targetRobotController->
		  storeGenome (_genome, _wm->getId (), _currentFitness);
	    }
	      
	}
    }
}

double
SimpleController::dist(double x1,double y1,double x2, double y2)
{
  return sqrt((x1 - x2)*(x1 -x2) + (y1 -y2) * (y1 - y2));
}

void
  SimpleController::storeGenome (std::vector<double> genome, int senderId, float fitness)
{
  _genomesList[senderId] = genome;
  _fitnessList[senderId] = fitness;
}

void
SimpleController::stepEvolution ()
{

    logGenome();
    
    //L = L + A
    _genomesList[_wm->getId()] = _genome;
    _fitnessList[_wm->getId()] = _currentFitness;
    
    int selected = selectRandom(_fitnessList);
    SimpleSharedData::gEvoLog << " "  << selected << std::endl;
    _genome = _genomesList[selected];
    _currentFitness = _fitnessList[selected];
   
    _genome = mutate(_genome,_currentSigma);
    
}

std::vector<double> SimpleController::mutate(std::vector<double> g, float sigma)
{
  std::vector<double> result;
  
  for(unsigned int i = 0; i < g.size(); i ++ )
    {
      result.push_back( g[i] + sigma * gaussrand());
    }
  /*  result[_nbInputs] = 0.0;
  result[_nbInputs + 1] = 0.0;
  result[2 + 2 * _nbInputs] = 0.0;
  result[2 + 2 * _nbInputs + 1] = 0.0;*/
  
  return result;
}


void SimpleController::logGenome()
{
  //GENERATION ID-ROBOT FITNESS SIZE(localPop)
  SimpleSharedData::gEvoLog << (gWorld->getIterations () /SimpleSharedData::gEvalTime) << " " << _wm->getId () << " " <<  _currentFitness  << " " << _fitnessList.size()
    ;// << std::endl;

}

int
SimpleController::selectRandom(std::map < int, float >lFitness)
{       
  int randomIndex = rand()%lFitness.size();
  std::map<int, float >::iterator it = lFitness.begin();
  while (randomIndex !=0 )
    {
      it ++;
      randomIndex --;
    }
  return it->first;
}
