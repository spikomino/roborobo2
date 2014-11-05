#ifndef SIMPLECONTROLLER_H
#define SIMPLECONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "Simple/include/SimpleAgentObserver.h"
#include <neuralnetworks/NeuralNetwork.h>
#include <neat/gene.h>
#include <neat/genome.h>
#include <neat/genomeAdapted.h>

#include <iomanip>

using namespace Neural;
using namespace NEAT;

enum Phase {ACT, LISTEN};
//enum {ACTION_ACTIVATE, ACTION_GATHER, ACTION_BRAKE, ACTION_MATE, ACTION_SELECT, ACTION_SIZE};

class SimpleController:public Controller
{
private:
  int _iteration;
  int _evalTime;
  Phase _phase;
  // ANN
  unsigned int _nbInputs;
  unsigned int _nbOutputs;
  std::pair<std::vector<double>,std::vector<double>> act();

  void initRobot ();

  void stepBehaviour ();

  float updateFitness (std::vector < double >in, std::vector < double >out);
  void broadcastGenome ();

  void storeGenome (std::vector<double> genome, int senderId, float fitness);  

  void stepEvolution ();
  void logGenome();
  void loadNewGenome ();

  int selectRandom (std::map < int, float >lFitness);

  std::vector<double> mutate(std::vector<double> g, float sigma);

  std::vector<double> _genome;
  float _currentFitness;
  float _currentSigma;

  std::vector<double> _previousOut;
  
  std::map < int, std::vector<double> >_genomesList;
  std::map < int, float >_fitnessList;
  double dist(double x1, double y1, double x2, double y2);
public:

  SimpleController (RobotWorldModel * wm);
  ~SimpleController ();

  void reset ();
  void step ();

};
#endif
