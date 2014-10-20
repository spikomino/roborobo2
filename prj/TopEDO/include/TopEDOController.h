/**
 * @author Inaki Fernandez Perez <inaki.fernandez@loria.fr>
 *
 */

#ifndef TOPEDOCONTROLLER_H
#define TOPEDOCONTROLLER_H

#include "RoboroboMain/common.h"
#include "RoboroboMain/roborobo.h"
#include "Utilities/Graphics.h"
#include "Controllers/Controller.h"
#include "WorldModels/RobotWorldModel.h"
#include "TopEDO/include/TopEDOAgentObserver.h"
#include <neuralnetworks/NeuralNetwork.h>
#include <neat/gene.h>
#include <neat/genome.h>
#include <neat/genomeAdapted.h>

#include <iomanip>

using namespace Neural;
using namespace NEAT;

class TopEDOController:public Controller
{
private:
  int _iteration;
  int _birthdate;		// evaluation when this controller was initialized.

  Network *nn;

  void createNN ();
  unsigned int computeRequiredNumberOfWeights ();
  void initRobot ();

  bool _isNewGenome;

  void stepBehaviour ();
  std::pair<std::vector<double>,std::vector<double>> act();
  float updateFitness (std::vector < double >in, std::vector < double >out);
  void broadcastGenome ();
  void storeGenome (GenomeAdapted * genome, int senderId, int senderBirthdate,
		    float sigma, float fitness);  




  void stepEvolution ();
  void logGenome();
  void loadNewGenome ();

  int selectBest (std::map < int, float >lFitness);
  int selectRandom (std::map < int, float >lFitness);
  void mutate (float sigma);

  bool getNewGenomeStatus ()
  {
    return _isNewGenome;
  }
  void setNewGenomeStatus (bool __status)
  {
    _isNewGenome = __status;
  }

  // evolutionary engine

  GenomeAdapted *_genome;

  std::map < int, GenomeAdapted * >_genomesList;
  std::map < int, float >_sigmaList;
  std::map < int, float >_fitnessList;
  std::map < int, int >_birthdateList;	// store the birthdate of the received controllers (useful for monitoring).


  float _currentFitness;
  float _currentSigma;

  //NOTE: NEAT-like innovation number and number of nodes FOR THIS ROBOT
  double innovNumber;
  int nodeId;

  // ANN
  unsigned int _nbInputs;
  unsigned int _nbOutputs;

  

public:

  TopEDOController (RobotWorldModel * wm);
  ~TopEDOController ();

  void reset ();
  void step ();

  int getBirthdate ()
  {
    return _birthdate;
  }


};


#endif
