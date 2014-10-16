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

  bool _isNewGenome;

  int selectBest (std::map < int, float >lFitness);
  void selectRandomGenome ();
  void mutate (float sigma);

  void stepBehaviour ();
  void stepEvolution ();

  void broadcastGenome ();
  void loadNewGenome ();

  float dist (float x1, float y1, float x2, float y2);
  float updateFitness (std::vector < double >in, std::vector < double >out);


  unsigned int computeRequiredNumberOfWeights ();

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

  //TOFIX NOTE: NEAT-like innovation number and number of nodes FOR THIS ROBOT
  double innovNum;
  int nodeId;

  // ANN
  unsigned int _nbInputs;
  unsigned int _nbOutputs;

  void storeGenome (GenomeAdapted * genome, int senderId, int senderBirthdate,
		    float sigma, float fitness);
  void resetRobot ();

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
